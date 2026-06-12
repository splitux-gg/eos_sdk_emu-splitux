// Copyright Epic Games, Inc. All Rights Reserved.

#include "eos/eos_p2p.h"
#include "eos/eos_p2p_types.h"
#include "internal/p2p_internal.h"
#include "internal/platform_internal.h"
#include "internal/connect_internal.h"
#include "internal/callbacks.h"
#include "internal/logging.h"
#include "lan_p2p.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Magic number for state validation
#define P2P_MAGIC 0x50325032  // "P2P2"

// Default queue sizes (in bytes)
#define DEFAULT_INCOMING_QUEUE_MAX (1024 * 1024)  // 1MB
#define DEFAULT_OUTGOING_QUEUE_MAX (1024 * 1024)  // 1MB

// LAN P2P wire message types (raw byte carried in the packet header).
// These intentionally match lan_p2p.h's P2P_MSG_* values.
#define MSG_DATA    1
#define MSG_CONNECT 2
#define MSG_ACCEPT  3
#define MSG_CLOSE   4

// Re-send an unanswered CONNECT at most this often (ms).
#define P2P_CONNECT_RESEND_MS 250

// Helper: Get current time in milliseconds
static uint64_t get_time_ms(void) {
#ifdef _WIN32
    return GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#endif
}

// Helper: Copy ProductUserId to string.
// Emit the REAL 32-char hex (the same value that travels on the wire), not the
// "%p" pointer form: the lobby/sessions layer hands us one ProductUserId object
// for the host while Palworld hands SendPacket a *different* object for the same
// logical user, so any key/compare must be value-based to line them up.
static void product_user_id_to_string(EOS_ProductUserId user_id, char* out, size_t out_size) {
    if (!user_id || !out || out_size < 33) return;
    int32_t len = (int32_t)out_size;
    if (EOS_ProductUserId_ToString(user_id, out, &len) != EOS_Success) {
        // Fall back to the pointer form only if the id won't stringify.
        snprintf(out, out_size, "%p", (void*)user_id);
    }
}

// Helper: Compare ProductUserIds by value (hex), not pointer identity.
// EOS_ProductUserId_FromString / the lobby parse can mint distinct objects for
// the same PUID, so pointer equality misses real matches (e.g. the host address
// registered under the lobby owner id vs the RemoteUserId Palworld sends to).
static bool product_user_id_equal(EOS_ProductUserId a, EOS_ProductUserId b) {
    if (a == b) return true;
    if (!a || !b) return false;
    char ha[33], hb[33];
    int32_t la = (int32_t)sizeof(ha), lb = (int32_t)sizeof(hb);
    if (EOS_ProductUserId_ToString(a, ha, &la) != EOS_Success) return false;
    if (EOS_ProductUserId_ToString(b, hb, &lb) != EOS_Success) return false;
    return strcmp(ha, hb) == 0;
}

// Helper: Copy socket ID
static void copy_socket_id(EOS_P2P_SocketId* dest, const EOS_P2P_SocketId* src) {
    if (!dest || !src) return;
    dest->ApiVersion = src->ApiVersion;
    strncpy(dest->SocketName, src->SocketName, EOS_P2P_SOCKETID_SOCKETNAME_SIZE - 1);
    dest->SocketName[EOS_P2P_SOCKETID_SOCKETNAME_SIZE - 1] = '\0';
}

// Helper: Compare socket IDs
static bool socket_id_equal(const EOS_P2P_SocketId* a, const EOS_P2P_SocketId* b) {
    if (!a || !b) return false;
    return strcmp(a->SocketName, b->SocketName) == 0;
}

// Helper: Find connection
static PeerConnection* find_connection(P2PState* state, EOS_ProductUserId peer, const EOS_P2P_SocketId* socket_id) {
    if (!state || !peer || !socket_id) return NULL;

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        PeerConnection* conn = &state->connections[i];
        if (!conn->valid) continue;
        if (!product_user_id_equal(conn->peer_id, peer)) continue;
        if (!socket_id_equal(&conn->socket_id, socket_id)) continue;
        return conn;
    }
    return NULL;
}

// Helper: Create new connection
static PeerConnection* create_connection(P2PState* state, EOS_ProductUserId peer, const EOS_P2P_SocketId* socket_id) {
    if (!state || !peer || !socket_id) return NULL;

    // Find free slot
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        PeerConnection* conn = &state->connections[i];
        if (conn->valid) continue;

        // Initialize connection
        memset(conn, 0, sizeof(PeerConnection));
        conn->valid = true;
        conn->peer_id = peer;
        product_user_id_to_string(peer, conn->peer_id_string, sizeof(conn->peer_id_string));
        copy_socket_id(&conn->socket_id, socket_id);
        conn->state = CONN_STATE_NONE;
        conn->last_activity = get_time_ms();

        state->connection_count++;
        return conn;
    }

    return NULL;  // No free slots
}

// Helper: Check if socket is auto-accepted
static bool is_socket_auto_accepted(P2PState* state, const EOS_P2P_SocketId* socket_id) {
    if (!state || !socket_id) return false;
    if (state->auto_accept_all) return true;

    for (int i = 0; i < MAX_ACCEPTED_SOCKETS; i++) {
        AcceptedSocket* acc = &state->accepted_sockets[i];
        if (!acc->valid) continue;
        if (socket_id_equal(&acc->socket_id, socket_id)) return true;
    }

    return false;
}

// Helper: Queue received packet
static bool queue_received_packet(P2PState* state, const ReceivedPacket* packet) {
    if (!state || !packet) return false;

    // Check if queue is full
    if (state->recv_count >= MAX_RECV_QUEUE) {
        EOS_LOG_WARN("P2P: Received packet queue full, dropping packet");

        // TODO: Fire queue full notification

        return false;
    }

    // Check queue size limit
    if (state->incoming_queue_max_bytes > 0) {
        if (state->incoming_queue_current_bytes + packet->size > state->incoming_queue_max_bytes) {
            EOS_LOG_WARN("P2P: Incoming queue size limit exceeded");
            return false;
        }
    }

    // Add to queue
    int idx = state->recv_tail;
    memcpy(&state->recv_queue[idx], packet, sizeof(ReceivedPacket));
    state->recv_tail = (state->recv_tail + 1) % MAX_RECV_QUEUE;
    state->recv_count++;
    state->incoming_queue_current_bytes += packet->size;

    return true;
}

// Helper: Queue pending send packet
static bool queue_pending_packet(P2PState* state, const PendingPacket* packet) {
    if (!state || !packet) return false;

    // Check if queue is full
    if (state->send_count >= MAX_SEND_QUEUE) {
        EOS_LOG_WARN("P2P: Send packet queue full");
        return false;
    }

    // Check queue size limit
    if (state->outgoing_queue_max_bytes > 0) {
        if (state->outgoing_queue_current_bytes + packet->size > state->outgoing_queue_max_bytes) {
            EOS_LOG_WARN("P2P: Outgoing queue size limit exceeded");
            return false;
        }
    }

    // Add to queue
    int idx = state->send_tail;
    memcpy(&state->send_queue[idx], packet, sizeof(PendingPacket));
    state->send_tail = (state->send_tail + 1) % MAX_SEND_QUEUE;
    state->send_count++;
    state->outgoing_queue_current_bytes += packet->size;

    return true;
}

// ----------------------------------------------------------------------------
// LAN transport helpers (wire the lan_p2p UDP socket into the P2P state)
// ----------------------------------------------------------------------------

// Resolve the local logged-in user's ProductUserId (stable pointer owned by
// ConnectState). P2PState.local_user may not be populated, so derive it from
// the connect subsystem at call time.
static EOS_ProductUserId p2p_local_puid(P2PState* state) {
    if (!state || !state->platform) return NULL;
    if (state->local_user) return state->local_user;
    ConnectState* cs = state->platform->connect;
    if (!cs) return NULL;
    // Return the MOST-RECENT logged-in user, not the first. Palworld does an
    // anonymous Connect_Login first (slot 0) and then a Steam-authenticated one
    // (slot 1); it runs the lobby and all P2P networking under that *second*
    // identity. The lobby owner/member PUIDs - and therefore the RemoteUserId
    // the peer addresses us by - are the Steam PUID. If we stamped our wire
    // sender_id with slot 0 instead, the peer's CONNECT/ACCEPT/DATA could never
    // be matched to the connection it is actually trying to establish (it would
    // ESTABLISH a throwaway conn under the wrong id while the real one spun on
    // REQUESTING forever). Iterate backwards so we hand back the Steam login.
    for (int i = MAX_LOCAL_USERS - 1; i >= 0; i--) {
        if (cs->users[i].in_use && cs->users[i].status == EOS_LS_LoggedIn) {
            return (EOS_ProductUserId)&cs->users[i].user_id;
        }
    }
    return NULL;
}

// The 32-char hex string for the local user (the on-the-wire sender_id).
// Returns a pointer into ConnectState's stable storage (do NOT free).
static const char* p2p_local_hex(P2PState* state) {
    EOS_ProductUserId p = p2p_local_puid(state);
    if (!p) return NULL;
    return ((EOS_ProductUserIdDetails*)p)->id_string;
}

// Find a connection by the peer's 32-char hex id (used on the receive path,
// where EOS_ProductUserId_FromString hands back a fresh pointer each call so
// pointer identity cannot be relied upon).
static PeerConnection* find_connection_by_hex(P2PState* state, const char* hex,
                                              const EOS_P2P_SocketId* socket_id) {
    if (!state || !hex || !socket_id) return NULL;
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        PeerConnection* conn = &state->connections[i];
        if (!conn->valid) continue;
        if (!socket_id_equal(&conn->socket_id, socket_id)) continue;
        if (strcmp(conn->peer_id_string, hex) != 0) continue;
        return conn;
    }
    return NULL;
}

// Send one wire message to a peer over the LAN UDP socket.
static void p2p_send_msg(P2PState* state, PeerConnection* conn, uint8_t msg_type,
                         uint8_t channel, const uint8_t* data, uint32_t data_len) {
    if (!state || !state->sock || !conn) return;
    if (conn->peer_address[0] == '\0') {
        EOS_LOG_DEBUG("P2P: cannot send msg %u to %s - no peer address yet",
                      (unsigned)msg_type, conn->peer_id_string);
        return;
    }

    const char* local = p2p_local_hex(state);

    P2PSendPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.target_addr = conn->peer_address;
    pkt.sender_id = local ? local : "";
    pkt.socket_name = conn->socket_id.SocketName;
    pkt.channel = channel;
    pkt.message_type = msg_type;
    pkt.data = data;
    pkt.data_len = data_len;
    pkt.sequence = 0;
    pkt.reliable = false;

    lan_p2p_send(state->sock, &pkt);
}

// Fire the stored "incoming connection request" notifications for a connection.
// Notifications are queued via the platform callback queue (info copied by
// value, pointers reference stable storage) to match the lobby/sessions style
// and avoid re-entrancy while we are still inside the tick.
static void p2p_fire_conn_request(P2PState* state, PeerConnection* conn) {
    EOS_ProductUserId local = p2p_local_puid(state);
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        P2PNotification* n = &state->conn_request_notifs[i];
        if (!n->active || !n->callback) continue;
        if (n->has_socket_filter && !socket_id_equal(&n->socket_filter, &conn->socket_id)) continue;

        EOS_P2P_OnIncomingConnectionRequestInfo info;
        memset(&info, 0, sizeof(info));
        info.ClientData = n->client_data;
        info.LocalUserId = local;
        info.RemoteUserId = conn->peer_id;
        info.SocketId = &conn->socket_id;

        if (state->platform && state->platform->callbacks) {
            callback_queue_push(state->platform->callbacks, n->callback, &info, sizeof(info));
        } else {
            ((EOS_P2P_OnIncomingConnectionRequestCallback)n->callback)(&info);
        }
    }
}

// Fire the stored "connection established" notifications for a connection.
static void p2p_fire_conn_established(P2PState* state, PeerConnection* conn) {
    EOS_ProductUserId local = p2p_local_puid(state);
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        P2PNotification* n = &state->conn_established_notifs[i];
        if (!n->active || !n->callback) continue;
        if (n->has_socket_filter && !socket_id_equal(&n->socket_filter, &conn->socket_id)) continue;

        EOS_P2P_OnPeerConnectionEstablishedInfo info;
        memset(&info, 0, sizeof(info));
        info.ClientData = n->client_data;
        info.LocalUserId = local;
        info.RemoteUserId = conn->peer_id;
        info.SocketId = &conn->socket_id;
        info.ConnectionType = EOS_CET_NewConnection;
        info.NetworkType = EOS_NCT_DirectConnection;

        if (state->platform && state->platform->callbacks) {
            callback_queue_push(state->platform->callbacks, n->callback, &info, sizeof(info));
        } else {
            ((EOS_P2P_OnPeerConnectionEstablishedCallback)n->callback)(&info);
        }
    }
}

// Fire the stored "remote connection closed" notifications for a connection.
static void p2p_fire_conn_closed(P2PState* state, PeerConnection* conn,
                                 EOS_EConnectionClosedReason reason) {
    EOS_ProductUserId local = p2p_local_puid(state);
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        P2PNotification* n = &state->conn_closed_notifs[i];
        if (!n->active || !n->callback) continue;
        if (n->has_socket_filter && !socket_id_equal(&n->socket_filter, &conn->socket_id)) continue;

        EOS_P2P_OnRemoteConnectionClosedInfo info;
        memset(&info, 0, sizeof(info));
        info.ClientData = n->client_data;
        info.LocalUserId = local;
        info.RemoteUserId = conn->peer_id;
        info.SocketId = &conn->socket_id;
        info.Reason = reason;

        if (state->platform && state->platform->callbacks) {
            callback_queue_push(state->platform->callbacks, n->callback, &info, sizeof(info));
        } else {
            ((EOS_P2P_OnRemoteConnectionClosedCallback)n->callback)(&info);
        }
    }
}

// Flush any queued (delayed-delivery) packets whose target connection has
// reached ESTABLISHED. Called every tick; this is also what drains a peer's
// backlog right after we receive its ACCEPT.
static void p2p_flush_send_queue(P2PState* state) {
    if (state->send_count <= 0) return;

    int flushed = 0;
    for (int i = 0; i < MAX_SEND_QUEUE; i++) {
        PendingPacket* pkt = &state->send_queue[i];
        if (!pkt->valid) continue;

        PeerConnection* conn = find_connection(state, pkt->target, &pkt->socket_id);
        if (!conn || conn->state != CONN_STATE_ESTABLISHED) continue;

        p2p_send_msg(state, conn, MSG_DATA, pkt->channel, pkt->data, pkt->size);

        pkt->valid = false;
        if (state->send_count > 0) state->send_count--;
        if (state->outgoing_queue_current_bytes >= pkt->size) {
            state->outgoing_queue_current_bytes -= pkt->size;
        } else {
            state->outgoing_queue_current_bytes = 0;
        }
        flushed++;
    }

    if (flushed > 0) {
        EOS_LOG_DEBUG("P2P: flushed %d queued DATA packet(s) on established connections", flushed);
    }
}

// Create P2P state
P2PState* p2p_create(PlatformState* platform) {
    if (!platform) {
        EOS_LOG_ERROR("p2p_create: Invalid platform");
        return NULL;
    }

    P2PState* state = calloc(1, sizeof(P2PState));
    if (!state) {
        EOS_LOG_ERROR("Failed to allocate P2PState");
        return NULL;
    }

    state->magic = P2P_MAGIC;
    state->platform = platform;
    state->auto_accept_all = true;  // Default: auto-accept all connections
    state->next_notif_id = 1;
    state->nat_type = EOS_NAT_Open;  // Always Open for LAN
    state->nat_queried = true;
    state->relay_control = EOS_RC_AllowRelays;
    // Base P2P port. Default 7777, but overridable via EOSLAN_P2P_BASE_PORT so
    // the emu can vacate 7777 for games whose OWN native IP/direct-connect
    // netdriver wants it (e.g. Satisfactory "IP" session type binds UDP 7777 in
    // this same process; our SO_EXCLUSIVEADDRUSE bind would otherwise refuse it).
    uint16_t base_port = 7777;
    {
        const char* env = getenv("EOSLAN_P2P_BASE_PORT");
        if (env && *env) {
            int v = atoi(env);
            if (v > 0 && v < 65536) base_port = (uint16_t)v;
        }
    }
    state->port_range_start = base_port;
    state->port_range_count = 99;
    state->incoming_queue_max_bytes = DEFAULT_INCOMING_QUEUE_MAX;
    state->outgoing_queue_max_bytes = DEFAULT_OUTGOING_QUEUE_MAX;

    // Bring up the LAN UDP transport. lan_p2p falls back to the next free port
    // when base_port is taken, so the host binds base_port and a second local
    // instance binds base_port+1. A failure here is non-fatal: the rest of the
    // P2P API still operates (degraded) so we don't crash the game.
    state->sock = lan_p2p_create(base_port);
    if (!state->sock) {
        EOS_LOG_ERROR("P2P: lan_p2p_create(%u) failed - P2P transport DEGRADED (no socket)", (unsigned)base_port);
    } else {
        EOS_LOG_INFO("P2P: LAN transport bound on %s:%u",
                     lan_p2p_get_local_ip(state->sock),
                     (unsigned)lan_p2p_get_port(state->sock));
    }
    state->last_connect_send = 0;

    EOS_LOG_INFO("P2P: Created P2P state");
    return state;
}

// Destroy P2P state
void p2p_destroy(P2PState* state) {
    if (!state || state->magic != P2P_MAGIC) return;

    EOS_LOG_INFO("P2P: Destroying P2P state");
    if (state->sock) {
        lan_p2p_destroy(state->sock);
        state->sock = NULL;
    }
    state->magic = 0;
    free(state);
}

// Local P2P listen port/ip (for advertising host_address in the lobby).
uint16_t p2p_get_listen_port(P2PState* state) {
    if (!state || state->magic != P2P_MAGIC || !state->sock) return 0;
    return lan_p2p_get_port(state->sock);
}

const char* p2p_get_listen_ip(P2PState* state) {
    if (!state || state->magic != P2P_MAGIC || !state->sock) return NULL;
    return lan_p2p_get_local_ip(state->sock);
}

// Register peer address (called by sessions module)
void p2p_register_peer_address(P2PState* state, EOS_ProductUserId peer, const char* address) {
    if (!state || state->magic != P2P_MAGIC || !peer || !address) return;

    // Find existing connection or create placeholder
    bool found = false;
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        PeerConnection* conn = &state->connections[i];
        if (!conn->valid) continue;
        if (!product_user_id_equal(conn->peer_id, peer)) continue;

        // Update address
        strncpy(conn->peer_address, address, sizeof(conn->peer_address) - 1);
        conn->peer_address[sizeof(conn->peer_address) - 1] = '\0';
        found = true;
        EOS_LOG_DEBUG("P2P: Updated peer address for %s: %s", conn->peer_id_string, address);
    }

    if (!found) {
        // Create placeholder connection (no socket yet)
        for (int i = 0; i < MAX_CONNECTIONS; i++) {
            PeerConnection* conn = &state->connections[i];
            if (conn->valid) continue;

            conn->valid = true;
            conn->peer_id = peer;
            product_user_id_to_string(peer, conn->peer_id_string, sizeof(conn->peer_id_string));
            strncpy(conn->peer_address, address, sizeof(conn->peer_address) - 1);
            conn->peer_address[sizeof(conn->peer_address) - 1] = '\0';
            conn->state = CONN_STATE_NONE;
            state->connection_count++;
            EOS_LOG_DEBUG("P2P: Registered new peer address for %s: %s", conn->peer_id_string, address);
            break;
        }
    }
}

// Get peer address
const char* p2p_get_peer_address(P2PState* state, EOS_ProductUserId peer) {
    if (!state || state->magic != P2P_MAGIC || !peer) return NULL;

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        PeerConnection* conn = &state->connections[i];
        if (!conn->valid) continue;
        if (!product_user_id_equal(conn->peer_id, peer)) continue;
        if (conn->peer_address[0] != '\0') {
            return conn->peer_address;
        }
    }

    return NULL;
}

// Tick function (process network, timeouts, etc.)
void p2p_tick(P2PState* state) {
    if (!state || state->magic != P2P_MAGIC) return;
    if (!state->sock) return;  // transport degraded - nothing to drive

    uint64_t now = get_time_ms();

    // ------------------------------------------------------------------
    // (a) RECEIVE: drain everything the socket has this tick.
    // ------------------------------------------------------------------
    P2PReceivedPacket rp;
    while (lan_p2p_recv(state->sock, &rp)) {
        // Build the socket id from the wire socket name.
        EOS_P2P_SocketId sock_id;
        memset(&sock_id, 0, sizeof(sock_id));
        sock_id.ApiVersion = EOS_P2P_SOCKETID_API_LATEST;
        strncpy(sock_id.SocketName, rp.socket_name, EOS_P2P_SOCKETID_SOCKETNAME_SIZE - 1);
        sock_id.SocketName[EOS_P2P_SOCKETID_SOCKETNAME_SIZE - 1] = '\0';

        // Find the connection by the peer's hex id (pointer identity is not
        // stable across FromString calls). Create one on first contact.
        PeerConnection* conn = find_connection_by_hex(state, rp.sender_id, &sock_id);
        if (!conn) {
            EOS_ProductUserId peer = EOS_ProductUserId_FromString(rp.sender_id);
            if (!peer) {
                EOS_LOG_WARN("P2P: dropping packet with invalid sender id '%s'", rp.sender_id);
                continue;
            }
            conn = create_connection(state, peer, &sock_id);
            if (!conn) {
                EOS_LOG_ERROR("P2P: connection table full, dropping packet from %s", rp.sender_id);
                continue;
            }
            // Key the connection on the wire hex id (not the "%p" pointer form).
            strncpy(conn->peer_id_string, rp.sender_id, sizeof(conn->peer_id_string) - 1);
            conn->peer_id_string[sizeof(conn->peer_id_string) - 1] = '\0';
        }

        // Learn / refresh the peer's source address and liveness.
        strncpy(conn->peer_address, rp.sender_addr, sizeof(conn->peer_address) - 1);
        conn->peer_address[sizeof(conn->peer_address) - 1] = '\0';
        conn->last_activity = now;

        switch (rp.message_type) {
            case MSG_CONNECT: {
                if (conn->state == CONN_STATE_ESTABLISHED) {
                    // Retransmitted CONNECT - just re-ACK, don't re-fire.
                    p2p_send_msg(state, conn, MSG_ACCEPT, 0, NULL, 0);
                    break;
                }
                if (is_socket_auto_accepted(state, &sock_id)) {
                    conn->state = CONN_STATE_ESTABLISHED;
                    conn->established_at = now;
                    p2p_send_msg(state, conn, MSG_ACCEPT, 0, NULL, 0);
                    EOS_LOG_INFO("P2P: CONNECT from %s on '%s' auto-accepted -> sent ACCEPT, ESTABLISHED",
                                 rp.sender_id, sock_id.SocketName);
                    p2p_fire_conn_request(state, conn);
                    p2p_fire_conn_established(state, conn);
                } else {
                    conn->state = CONN_STATE_PENDING;
                    EOS_LOG_INFO("P2P: CONNECT from %s on '%s' pending app accept",
                                 rp.sender_id, sock_id.SocketName);
                    p2p_fire_conn_request(state, conn);
                }
                break;
            }

            case MSG_ACCEPT: {
                if (conn->state != CONN_STATE_ESTABLISHED) {
                    conn->state = CONN_STATE_ESTABLISHED;
                    conn->established_at = now;
                    EOS_LOG_INFO("P2P: ACCEPT from %s on '%s' -> ESTABLISHED",
                                 rp.sender_id, sock_id.SocketName);
                    p2p_fire_conn_established(state, conn);
                    // Drain anything we queued for this peer while connecting.
                    p2p_flush_send_queue(state);
                }
                break;
            }

            case MSG_DATA: {
                // Receiving DATA implies the peer considers us connected; make
                // sure our side is established too (auto-accept path).
                if (conn->state != CONN_STATE_ESTABLISHED &&
                    is_socket_auto_accepted(state, &sock_id)) {
                    conn->state = CONN_STATE_ESTABLISHED;
                    conn->established_at = now;
                    p2p_send_msg(state, conn, MSG_ACCEPT, 0, NULL, 0);
                    EOS_LOG_INFO("P2P: first DATA from %s on '%s' -> ESTABLISHED (auto-accept)",
                                 rp.sender_id, sock_id.SocketName);
                    p2p_fire_conn_established(state, conn);
                }

                ReceivedPacket out;
                memset(&out, 0, sizeof(out));
                out.valid = true;
                out.sender = conn->peer_id;
                strncpy(out.sender_id_string, rp.sender_id, sizeof(out.sender_id_string) - 1);
                copy_socket_id(&out.socket_id, &sock_id);
                out.channel = rp.channel;
                out.size = (rp.data_len > MAX_PACKET_SIZE) ? MAX_PACKET_SIZE : rp.data_len;
                if (rp.data && out.size > 0) {
                    memcpy(out.data, rp.data, out.size);
                }
                queue_received_packet(state, &out);
                EOS_LOG_DEBUG("P2P: recv DATA %u bytes from %s (ch %u)",
                              out.size, rp.sender_id, (unsigned)rp.channel);
                break;
            }

            case MSG_CLOSE: {
                if (conn->state != CONN_STATE_CLOSED && conn->valid) {
                    conn->state = CONN_STATE_CLOSED;
                    EOS_LOG_INFO("P2P: CLOSE from %s on '%s'", rp.sender_id, sock_id.SocketName);
                    p2p_fire_conn_closed(state, conn, EOS_CCR_ClosedByPeer);
                    conn->valid = false;
                    if (state->connection_count > 0) state->connection_count--;
                }
                break;
            }

            default:
                EOS_LOG_WARN("P2P: unknown wire message type %u from %s",
                             (unsigned)rp.message_type, rp.sender_id);
                break;
        }
    }

    // ------------------------------------------------------------------
    // (b) SEND: (re)send CONNECT for connections still waiting for ACCEPT.
    // ------------------------------------------------------------------
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        PeerConnection* conn = &state->connections[i];
        if (!conn->valid) continue;
        if (conn->state != CONN_STATE_REQUESTING) continue;
        if (conn->peer_address[0] == '\0') continue;  // no target yet

        if ((now - state->last_connect_send) >= P2P_CONNECT_RESEND_MS) {
            p2p_send_msg(state, conn, MSG_CONNECT, 0, NULL, 0);
            state->last_connect_send = now;
            EOS_LOG_DEBUG("P2P: sent CONNECT to %s (%s)", conn->peer_id_string, conn->peer_address);
        }
    }

    // (c) Flush queued DATA for any connection that is now established.
    p2p_flush_send_queue(state);
}

//
// EOS API Implementation
//

EOS_EResult EOS_P2P_SendPacket(
    EOS_HP2P Handle,
    const EOS_P2P_SendPacketOptions* Options
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) {
        EOS_LOG_ERROR("P2P_SendPacket: Invalid handle");
        return EOS_InvalidParameters;
    }

    if (!Options) {
        EOS_LOG_ERROR("P2P_SendPacket: Options is NULL");
        return EOS_InvalidParameters;
    }

    // Be lenient about ApiVersion - Palworld is built against an older P2P API
    // and a strict equality check would reject every send.
    if (Options->ApiVersion != EOS_P2P_SENDPACKET_API_LATEST) {
        EOS_LOG_DEBUG("P2P_SendPacket: ApiVersion=%d (latest=%d) - proceeding",
                      Options->ApiVersion, EOS_P2P_SENDPACKET_API_LATEST);
    }

    if (!Options->LocalUserId || !Options->RemoteUserId) {
        EOS_LOG_ERROR("P2P_SendPacket: Invalid user IDs");
        return EOS_InvalidParameters;
    }

    if (!Options->SocketId || !Options->Data) {
        EOS_LOG_ERROR("P2P_SendPacket: Invalid socket ID or data");
        return EOS_InvalidParameters;
    }

    if (Options->DataLengthBytes > EOS_P2P_MAX_PACKET_SIZE) {
        EOS_LOG_ERROR("P2P_SendPacket: Packet too large (%u bytes)", Options->DataLengthBytes);
        return EOS_LimitExceeded;
    }

    // Look up or create connection
    PeerConnection* conn = find_connection(state, Options->RemoteUserId, Options->SocketId);

    if (conn && conn->state == CONN_STATE_ESTABLISHED) {
        // Connection established - transmit immediately over the LAN socket.
        p2p_send_msg(state, conn, MSG_DATA, Options->Channel,
                     (const uint8_t*)Options->Data, Options->DataLengthBytes);
        EOS_LOG_DEBUG("P2P_SendPacket: sent %u bytes to established connection (ch %u)",
                      Options->DataLengthBytes, (unsigned)Options->Channel);
        return EOS_Success;
    }

    // No established connection
    if (Options->bDisableAutoAcceptConnection) {
        EOS_LOG_WARN("P2P_SendPacket: No connection and auto-accept disabled");
        return EOS_NoConnection;
    }

    // Create connection if needed
    if (!conn) {
        conn = create_connection(state, Options->RemoteUserId, Options->SocketId);
        if (!conn) {
            EOS_LOG_ERROR("P2P_SendPacket: Failed to create connection (limit exceeded)");
            return EOS_LimitExceeded;
        }

        // Key the connection on the peer's real 32-char hex id (matches what
        // arrives on the wire), not the internal "%p" dedup form.
        {
            char hex[33];
            int32_t hlen = (int32_t)sizeof(hex);
            if (EOS_ProductUserId_ToString(Options->RemoteUserId, hex, &hlen) == EOS_Success) {
                strncpy(conn->peer_id_string, hex, sizeof(conn->peer_id_string) - 1);
                conn->peer_id_string[sizeof(conn->peer_id_string) - 1] = '\0';
            }
        }

        // Try to get peer address (registered from the lobby/session host_address)
        const char* addr = p2p_get_peer_address(state, Options->RemoteUserId);
        if (addr) {
            strncpy(conn->peer_address, addr, sizeof(conn->peer_address) - 1);
            conn->peer_address[sizeof(conn->peer_address) - 1] = '\0';
        }

        // Move to REQUESTING; p2p_tick sends (and re-sends) the CONNECT message.
        conn->state = CONN_STATE_REQUESTING;
        state->last_connect_send = 0;  // let tick fire the first CONNECT immediately
        EOS_LOG_INFO("P2P_SendPacket: initiating connection to %s (%s)",
                     conn->peer_id_string,
                     conn->peer_address[0] ? conn->peer_address : "addr pending");
    }

    // Queue packet if allowed
    if (Options->bAllowDelayedDelivery) {
        PendingPacket packet = {0};
        packet.valid = true;
        packet.target = Options->RemoteUserId;
        product_user_id_to_string(Options->RemoteUserId, packet.target_id_string, sizeof(packet.target_id_string));
        copy_socket_id(&packet.socket_id, Options->SocketId);
        packet.channel = Options->Channel;
        packet.size = Options->DataLengthBytes;
        packet.reliable = (Options->Reliability != EOS_PR_UnreliableUnordered);
        packet.allow_delayed = true;

        if (Options->DataLengthBytes > 0) {
            memcpy(packet.data, Options->Data, Options->DataLengthBytes);
        }

        if (queue_pending_packet(state, &packet)) {
            EOS_LOG_DEBUG("P2P_SendPacket: Queued packet for delayed delivery");
            return EOS_Success;
        } else {
            EOS_LOG_ERROR("P2P_SendPacket: Failed to queue packet");
            return EOS_LimitExceeded;
        }
    }

    EOS_LOG_WARN("P2P_SendPacket: No connection and delayed delivery not allowed");
    return EOS_NoConnection;
}

EOS_EResult EOS_P2P_GetNextReceivedPacketSize(
    EOS_HP2P Handle,
    const EOS_P2P_GetNextReceivedPacketSizeOptions* Options,
    uint32_t* OutPacketSizeBytes
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) {
        return EOS_InvalidParameters;
    }

    if (!Options || !OutPacketSizeBytes) {
        return EOS_InvalidParameters;
    }

    if (Options->ApiVersion != EOS_P2P_GETNEXTRECEIVEDPACKETSIZE_API_LATEST) {
        EOS_LOG_DEBUG("P2P_GetNextReceivedPacketSize: ApiVersion=%d - proceeding",
                      Options->ApiVersion);
    }

    if (!Options->LocalUserId) {
        return EOS_InvalidParameters;
    }

    // Find first matching packet in queue
    for (int i = 0; i < state->recv_count; i++) {
        int idx = (state->recv_head + i) % MAX_RECV_QUEUE;
        ReceivedPacket* pkt = &state->recv_queue[idx];

        if (!pkt->valid) continue;

        // Check channel filter
        if (Options->RequestedChannel) {
            if (pkt->channel != *Options->RequestedChannel) {
                continue;
            }
        }

        // Found matching packet
        *OutPacketSizeBytes = pkt->size;
        return EOS_Success;
    }

    return EOS_NotFound;
}

EOS_EResult EOS_P2P_ReceivePacket(
    EOS_HP2P Handle,
    const EOS_P2P_ReceivePacketOptions* Options,
    EOS_ProductUserId* OutPeerId,
    EOS_P2P_SocketId* OutSocketId,
    uint8_t* OutChannel,
    void* OutData,
    uint32_t* OutBytesWritten
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) {
        return EOS_InvalidParameters;
    }

    if (!Options || !OutData || !OutBytesWritten) {
        return EOS_InvalidParameters;
    }

    if (Options->ApiVersion != EOS_P2P_RECEIVEPACKET_API_LATEST) {
        EOS_LOG_DEBUG("P2P_ReceivePacket: ApiVersion=%d - proceeding", Options->ApiVersion);
    }

    if (!Options->LocalUserId) {
        return EOS_InvalidParameters;
    }

    // Find first matching packet
    for (int i = 0; i < state->recv_count; i++) {
        int idx = (state->recv_head + i) % MAX_RECV_QUEUE;
        ReceivedPacket* pkt = &state->recv_queue[idx];

        if (!pkt->valid) continue;

        // Check channel filter
        if (Options->RequestedChannel) {
            if (pkt->channel != *Options->RequestedChannel) {
                continue;
            }
        }

        // Found matching packet - copy out
        if (OutPeerId) *OutPeerId = pkt->sender;
        if (OutSocketId) copy_socket_id(OutSocketId, &pkt->socket_id);
        if (OutChannel) *OutChannel = pkt->channel;

        uint32_t copy_size = (pkt->size < Options->MaxDataSizeBytes) ? pkt->size : Options->MaxDataSizeBytes;
        memcpy(OutData, pkt->data, copy_size);
        *OutBytesWritten = copy_size;

        // Update queue size tracking
        state->incoming_queue_current_bytes -= pkt->size;

        // Remove from queue
        pkt->valid = false;

        // Compact queue if at head
        while (state->recv_count > 0 && !state->recv_queue[state->recv_head].valid) {
            state->recv_head = (state->recv_head + 1) % MAX_RECV_QUEUE;
            state->recv_count--;
        }

        return EOS_Success;
    }

    return EOS_NotFound;
}

EOS_NotificationId EOS_P2P_AddNotifyPeerConnectionRequest(
    EOS_HP2P Handle,
    const EOS_P2P_AddNotifyPeerConnectionRequestOptions* Options,
    void* ClientData,
    EOS_P2P_OnIncomingConnectionRequestCallback ConnectionRequestHandler
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) {
        return EOS_INVALID_NOTIFICATIONID;
    }

    if (!Options || !ConnectionRequestHandler) {
        return EOS_INVALID_NOTIFICATIONID;
    }

    if (Options->ApiVersion != EOS_P2P_ADDNOTIFYPEERCONNECTIONREQUEST_API_LATEST) {
        return EOS_INVALID_NOTIFICATIONID;
    }

    // Find free notification slot
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        P2PNotification* notif = &state->conn_request_notifs[i];
        if (notif->active) continue;

        notif->active = true;
        notif->callback = (void*)ConnectionRequestHandler;
        notif->client_data = ClientData;
        notif->id = state->next_notif_id++;
        notif->has_socket_filter = (Options->SocketId != NULL);
        if (notif->has_socket_filter) {
            copy_socket_id(&notif->socket_filter, Options->SocketId);
        }

        EOS_LOG_DEBUG("P2P: Added connection request notification (ID: %llu)", (unsigned long long)notif->id);
        return notif->id;
    }

    EOS_LOG_ERROR("P2P: No free notification slots for connection request");
    return EOS_INVALID_NOTIFICATIONID;
}

void EOS_P2P_RemoveNotifyPeerConnectionRequest(
    EOS_HP2P Handle,
    EOS_NotificationId NotificationId
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) return;

    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        P2PNotification* notif = &state->conn_request_notifs[i];
        if (!notif->active) continue;
        if (notif->id != NotificationId) continue;

        notif->active = false;
        EOS_LOG_DEBUG("P2P: Removed connection request notification (ID: %llu)", (unsigned long long)NotificationId);
        return;
    }
}

EOS_NotificationId EOS_P2P_AddNotifyPeerConnectionEstablished(
    EOS_HP2P Handle,
    const EOS_P2P_AddNotifyPeerConnectionEstablishedOptions* Options,
    void* ClientData,
    EOS_P2P_OnPeerConnectionEstablishedCallback ConnectionEstablishedHandler
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) {
        return EOS_INVALID_NOTIFICATIONID;
    }

    if (!Options || !ConnectionEstablishedHandler) {
        return EOS_INVALID_NOTIFICATIONID;
    }

    if (Options->ApiVersion != EOS_P2P_ADDNOTIFYPEERCONNECTIONESTABLISHED_API_LATEST) {
        return EOS_INVALID_NOTIFICATIONID;
    }

    // Find free notification slot
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        P2PNotification* notif = &state->conn_established_notifs[i];
        if (notif->active) continue;

        notif->active = true;
        notif->callback = (void*)ConnectionEstablishedHandler;
        notif->client_data = ClientData;
        notif->id = state->next_notif_id++;
        notif->has_socket_filter = (Options->SocketId != NULL);
        if (notif->has_socket_filter) {
            copy_socket_id(&notif->socket_filter, Options->SocketId);
        }

        EOS_LOG_DEBUG("P2P: Added connection established notification (ID: %llu)", (unsigned long long)notif->id);
        return notif->id;
    }

    EOS_LOG_ERROR("P2P: No free notification slots for connection established");
    return EOS_INVALID_NOTIFICATIONID;
}

void EOS_P2P_RemoveNotifyPeerConnectionEstablished(
    EOS_HP2P Handle,
    EOS_NotificationId NotificationId
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) return;

    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        P2PNotification* notif = &state->conn_established_notifs[i];
        if (!notif->active) continue;
        if (notif->id != NotificationId) continue;

        notif->active = false;
        EOS_LOG_DEBUG("P2P: Removed connection established notification (ID: %llu)", (unsigned long long)NotificationId);
        return;
    }
}

EOS_NotificationId EOS_P2P_AddNotifyPeerConnectionInterrupted(
    EOS_HP2P Handle,
    const EOS_P2P_AddNotifyPeerConnectionInterruptedOptions* Options,
    void* ClientData,
    EOS_P2P_OnPeerConnectionInterruptedCallback ConnectionInterruptedHandler
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) {
        return EOS_INVALID_NOTIFICATIONID;
    }

    if (!Options || !ConnectionInterruptedHandler) {
        return EOS_INVALID_NOTIFICATIONID;
    }

    if (Options->ApiVersion != EOS_P2P_ADDNOTIFYPEERCONNECTIONINTERRUPTED_API_LATEST) {
        return EOS_INVALID_NOTIFICATIONID;
    }

    // Find free notification slot
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        P2PNotification* notif = &state->conn_interrupted_notifs[i];
        if (notif->active) continue;

        notif->active = true;
        notif->callback = (void*)ConnectionInterruptedHandler;
        notif->client_data = ClientData;
        notif->id = state->next_notif_id++;
        notif->has_socket_filter = (Options->SocketId != NULL);
        if (notif->has_socket_filter) {
            copy_socket_id(&notif->socket_filter, Options->SocketId);
        }

        EOS_LOG_DEBUG("P2P: Added connection interrupted notification (ID: %llu)", (unsigned long long)notif->id);
        return notif->id;
    }

    EOS_LOG_ERROR("P2P: No free notification slots for connection interrupted");
    return EOS_INVALID_NOTIFICATIONID;
}

void EOS_P2P_RemoveNotifyPeerConnectionInterrupted(
    EOS_HP2P Handle,
    EOS_NotificationId NotificationId
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) return;

    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        P2PNotification* notif = &state->conn_interrupted_notifs[i];
        if (!notif->active) continue;
        if (notif->id != NotificationId) continue;

        notif->active = false;
        EOS_LOG_DEBUG("P2P: Removed connection interrupted notification (ID: %llu)", (unsigned long long)NotificationId);
        return;
    }
}

EOS_NotificationId EOS_P2P_AddNotifyPeerConnectionClosed(
    EOS_HP2P Handle,
    const EOS_P2P_AddNotifyPeerConnectionClosedOptions* Options,
    void* ClientData,
    EOS_P2P_OnRemoteConnectionClosedCallback ConnectionClosedHandler
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) {
        return EOS_INVALID_NOTIFICATIONID;
    }

    if (!Options || !ConnectionClosedHandler) {
        return EOS_INVALID_NOTIFICATIONID;
    }

    if (Options->ApiVersion != EOS_P2P_ADDNOTIFYPEERCONNECTIONCLOSED_API_LATEST) {
        return EOS_INVALID_NOTIFICATIONID;
    }

    // Find free notification slot
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        P2PNotification* notif = &state->conn_closed_notifs[i];
        if (notif->active) continue;

        notif->active = true;
        notif->callback = (void*)ConnectionClosedHandler;
        notif->client_data = ClientData;
        notif->id = state->next_notif_id++;
        notif->has_socket_filter = (Options->SocketId != NULL);
        if (notif->has_socket_filter) {
            copy_socket_id(&notif->socket_filter, Options->SocketId);
        }

        EOS_LOG_DEBUG("P2P: Added connection closed notification (ID: %llu)", (unsigned long long)notif->id);
        return notif->id;
    }

    EOS_LOG_ERROR("P2P: No free notification slots for connection closed");
    return EOS_INVALID_NOTIFICATIONID;
}

void EOS_P2P_RemoveNotifyPeerConnectionClosed(
    EOS_HP2P Handle,
    EOS_NotificationId NotificationId
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) return;

    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        P2PNotification* notif = &state->conn_closed_notifs[i];
        if (!notif->active) continue;
        if (notif->id != NotificationId) continue;

        notif->active = false;
        EOS_LOG_DEBUG("P2P: Removed connection closed notification (ID: %llu)", (unsigned long long)NotificationId);
        return;
    }
}

EOS_EResult EOS_P2P_AcceptConnection(
    EOS_HP2P Handle,
    const EOS_P2P_AcceptConnectionOptions* Options
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) {
        return EOS_InvalidParameters;
    }

    if (!Options) {
        return EOS_InvalidParameters;
    }

    if (Options->ApiVersion != EOS_P2P_ACCEPTCONNECTION_API_LATEST) {
        EOS_LOG_DEBUG("P2P_AcceptConnection: ApiVersion=%d (latest=%d) - proceeding",
                      Options->ApiVersion, EOS_P2P_ACCEPTCONNECTION_API_LATEST);
    }

    if (!Options->LocalUserId || !Options->RemoteUserId || !Options->SocketId) {
        return EOS_InvalidParameters;
    }

    // Add socket to accepted list (if not already there)
    bool found = false;
    for (int i = 0; i < MAX_ACCEPTED_SOCKETS; i++) {
        AcceptedSocket* acc = &state->accepted_sockets[i];
        if (!acc->valid) continue;
        if (socket_id_equal(&acc->socket_id, Options->SocketId)) {
            found = true;
            break;
        }
    }

    if (!found) {
        for (int i = 0; i < MAX_ACCEPTED_SOCKETS; i++) {
            AcceptedSocket* acc = &state->accepted_sockets[i];
            if (acc->valid) continue;

            acc->valid = true;
            acc->local_user = Options->LocalUserId;
            copy_socket_id(&acc->socket_id, Options->SocketId);
            state->accepted_socket_count++;
            EOS_LOG_DEBUG("P2P: Accepted socket: %s", Options->SocketId->SocketName);
            break;
        }
    }

    // Find connection. The pending connection was created on the receive path
    // keyed on the peer hex id, so match by hex (pointer identity is unstable).
    PeerConnection* conn = find_connection(state, Options->RemoteUserId, Options->SocketId);
    if (!conn) {
        char hex[33];
        int32_t hlen = (int32_t)sizeof(hex);
        if (EOS_ProductUserId_ToString(Options->RemoteUserId, hex, &hlen) == EOS_Success) {
            conn = find_connection_by_hex(state, hex, Options->SocketId);
        }
    }
    if (conn) {
        if (conn->state == CONN_STATE_PENDING || conn->state == CONN_STATE_REQUESTING) {
            conn->state = CONN_STATE_ESTABLISHED;
            conn->established_at = get_time_ms();
            p2p_send_msg(state, conn, MSG_ACCEPT, 0, NULL, 0);
            EOS_LOG_INFO("P2P: AcceptConnection -> sent ACCEPT, ESTABLISHED with %s",
                         conn->peer_id_string);
            p2p_fire_conn_established(state, conn);
            p2p_flush_send_queue(state);
        }
    }

    return EOS_Success;
}

EOS_EResult EOS_P2P_CloseConnection(
    EOS_HP2P Handle,
    const EOS_P2P_CloseConnectionOptions* Options
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) {
        return EOS_InvalidParameters;
    }

    if (!Options) {
        return EOS_InvalidParameters;
    }

    if (Options->ApiVersion != EOS_P2P_CLOSECONNECTION_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!Options->LocalUserId || !Options->RemoteUserId) {
        return EOS_InvalidParameters;
    }

    // Find and close connection
    if (Options->SocketId) {
        PeerConnection* conn = find_connection(state, Options->RemoteUserId, Options->SocketId);
        if (conn) {
            // TODO: Send CLOSE message
            conn->state = CONN_STATE_CLOSED;
            conn->valid = false;
            state->connection_count--;
            EOS_LOG_DEBUG("P2P: Closed connection to peer on socket %s", Options->SocketId->SocketName);
        }
    } else {
        // Close all connections to this peer
        for (int i = 0; i < MAX_CONNECTIONS; i++) {
            PeerConnection* conn = &state->connections[i];
            if (!conn->valid) continue;
            if (!product_user_id_equal(conn->peer_id, Options->RemoteUserId)) continue;

            // TODO: Send CLOSE message
            conn->state = CONN_STATE_CLOSED;
            conn->valid = false;
            state->connection_count--;
        }
        EOS_LOG_DEBUG("P2P: Closed all connections to peer");
    }

    return EOS_Success;
}

EOS_EResult EOS_P2P_CloseConnections(
    EOS_HP2P Handle,
    const EOS_P2P_CloseConnectionsOptions* Options
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) {
        return EOS_InvalidParameters;
    }

    if (!Options) {
        return EOS_InvalidParameters;
    }

    if (Options->ApiVersion != EOS_P2P_CLOSECONNECTIONS_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!Options->LocalUserId || !Options->SocketId) {
        return EOS_InvalidParameters;
    }

    // Close all connections on this socket
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        PeerConnection* conn = &state->connections[i];
        if (!conn->valid) continue;
        if (!socket_id_equal(&conn->socket_id, Options->SocketId)) continue;

        // TODO: Send CLOSE message
        conn->state = CONN_STATE_CLOSED;
        conn->valid = false;
        state->connection_count--;
    }

    EOS_LOG_DEBUG("P2P: Closed all connections on socket %s", Options->SocketId->SocketName);
    return EOS_Success;
}

void EOS_P2P_QueryNATType(
    EOS_HP2P Handle,
    const EOS_P2P_QueryNATTypeOptions* Options,
    void* ClientData,
    const EOS_P2P_OnQueryNATTypeCompleteCallback CompletionDelegate
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) return;

    if (!Options || !CompletionDelegate) return;

    if (Options->ApiVersion != EOS_P2P_QUERYNATTYPE_API_LATEST) return;

    // For LAN, NAT type is always Open
    state->nat_type = EOS_NAT_Open;
    state->nat_queried = true;

    // TODO: Queue callback via callback system
    EOS_P2P_OnQueryNATTypeCompleteInfo info = {0};
    info.ResultCode = EOS_Success;
    info.ClientData = ClientData;
    info.NATType = EOS_NAT_Open;

    // Call immediately for now (should be queued)
    CompletionDelegate(&info);
    EOS_LOG_DEBUG("P2P: NAT type query completed (Open)");
}

EOS_EResult EOS_P2P_GetNATType(
    EOS_HP2P Handle,
    const EOS_P2P_GetNATTypeOptions* Options,
    EOS_ENATType* OutNATType
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) {
        return EOS_InvalidParameters;
    }

    if (!Options || !OutNATType) {
        return EOS_InvalidParameters;
    }

    if (Options->ApiVersion != EOS_P2P_GETNATTYPE_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!state->nat_queried) {
        return EOS_NotFound;
    }

    *OutNATType = state->nat_type;
    return EOS_Success;
}

EOS_EResult EOS_P2P_SetRelayControl(
    EOS_HP2P Handle,
    const EOS_P2P_SetRelayControlOptions* Options
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) {
        return EOS_InvalidParameters;
    }

    if (!Options) {
        return EOS_InvalidParameters;
    }

    if (Options->ApiVersion != EOS_P2P_SETRELAYCONTROL_API_LATEST) {
        return EOS_InvalidParameters;
    }

    state->relay_control = Options->RelayControl;
    EOS_LOG_DEBUG("P2P: Set relay control to %d", Options->RelayControl);
    return EOS_Success;
}

EOS_EResult EOS_P2P_GetRelayControl(
    EOS_HP2P Handle,
    const EOS_P2P_GetRelayControlOptions* Options,
    EOS_ERelayControl* OutRelayControl
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) {
        return EOS_InvalidParameters;
    }

    if (!Options || !OutRelayControl) {
        return EOS_InvalidParameters;
    }

    if (Options->ApiVersion != EOS_P2P_GETRELAYCONTROL_API_LATEST) {
        return EOS_InvalidParameters;
    }

    *OutRelayControl = state->relay_control;
    return EOS_Success;
}

EOS_EResult EOS_P2P_SetPortRange(
    EOS_HP2P Handle,
    const EOS_P2P_SetPortRangeOptions* Options
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) {
        return EOS_InvalidParameters;
    }

    if (!Options) {
        return EOS_InvalidParameters;
    }

    if (Options->ApiVersion != EOS_P2P_SETPORTRANGE_API_LATEST) {
        return EOS_InvalidParameters;
    }

    state->port_range_start = Options->Port;
    state->port_range_count = Options->MaxAdditionalPortsToTry;
    EOS_LOG_DEBUG("P2P: Set port range to %u + %u", Options->Port, Options->MaxAdditionalPortsToTry);
    return EOS_Success;
}

EOS_EResult EOS_P2P_GetPortRange(
    EOS_HP2P Handle,
    const EOS_P2P_GetPortRangeOptions* Options,
    uint16_t* OutPort,
    uint16_t* OutNumAdditionalPortsToTry
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) {
        return EOS_InvalidParameters;
    }

    if (!Options || !OutPort || !OutNumAdditionalPortsToTry) {
        return EOS_InvalidParameters;
    }

    if (Options->ApiVersion != EOS_P2P_GETPORTRANGE_API_LATEST) {
        return EOS_InvalidParameters;
    }

    *OutPort = state->port_range_start;
    *OutNumAdditionalPortsToTry = state->port_range_count;
    return EOS_Success;
}

EOS_EResult EOS_P2P_SetPacketQueueSize(
    EOS_HP2P Handle,
    const EOS_P2P_SetPacketQueueSizeOptions* Options
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) {
        return EOS_InvalidParameters;
    }

    if (!Options) {
        return EOS_InvalidParameters;
    }

    if (Options->ApiVersion != EOS_P2P_SETPACKETQUEUESIZE_API_LATEST) {
        return EOS_InvalidParameters;
    }

    state->incoming_queue_max_bytes = Options->IncomingPacketQueueMaxSizeBytes;
    state->outgoing_queue_max_bytes = Options->OutgoingPacketQueueMaxSizeBytes;
    EOS_LOG_DEBUG("P2P: Set queue sizes - incoming: %llu, outgoing: %llu",
              (unsigned long long)state->incoming_queue_max_bytes,
              (unsigned long long)state->outgoing_queue_max_bytes);
    return EOS_Success;
}

EOS_EResult EOS_P2P_GetPacketQueueInfo(
    EOS_HP2P Handle,
    const EOS_P2P_GetPacketQueueInfoOptions* Options,
    EOS_P2P_PacketQueueInfo* OutPacketQueueInfo
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) {
        return EOS_InvalidParameters;
    }

    if (!Options || !OutPacketQueueInfo) {
        return EOS_InvalidParameters;
    }

    if (Options->ApiVersion != EOS_P2P_GETPACKETQUEUEINFO_API_LATEST) {
        return EOS_InvalidParameters;
    }

    OutPacketQueueInfo->IncomingPacketQueueMaxSizeBytes = state->incoming_queue_max_bytes;
    OutPacketQueueInfo->IncomingPacketQueueCurrentSizeBytes = state->incoming_queue_current_bytes;
    OutPacketQueueInfo->IncomingPacketQueueCurrentPacketCount = state->recv_count;
    OutPacketQueueInfo->OutgoingPacketQueueMaxSizeBytes = state->outgoing_queue_max_bytes;
    OutPacketQueueInfo->OutgoingPacketQueueCurrentSizeBytes = state->outgoing_queue_current_bytes;
    OutPacketQueueInfo->OutgoingPacketQueueCurrentPacketCount = state->send_count;

    return EOS_Success;
}

EOS_NotificationId EOS_P2P_AddNotifyIncomingPacketQueueFull(
    EOS_HP2P Handle,
    const EOS_P2P_AddNotifyIncomingPacketQueueFullOptions* Options,
    void* ClientData,
    EOS_P2P_OnIncomingPacketQueueFullCallback IncomingPacketQueueFullHandler
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) {
        return EOS_INVALID_NOTIFICATIONID;
    }

    if (!Options || !IncomingPacketQueueFullHandler) {
        return EOS_INVALID_NOTIFICATIONID;
    }

    if (Options->ApiVersion != EOS_P2P_ADDNOTIFYINCOMINGPACKETQUEUEFULL_API_LATEST) {
        return EOS_INVALID_NOTIFICATIONID;
    }

    // Find free notification slot
    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        P2PNotification* notif = &state->queue_full_notifs[i];
        if (notif->active) continue;

        notif->active = true;
        notif->callback = (void*)IncomingPacketQueueFullHandler;
        notif->client_data = ClientData;
        notif->id = state->next_notif_id++;
        notif->has_socket_filter = false;

        EOS_LOG_DEBUG("P2P: Added queue full notification (ID: %llu)", (unsigned long long)notif->id);
        return notif->id;
    }

    EOS_LOG_ERROR("P2P: No free notification slots for queue full");
    return EOS_INVALID_NOTIFICATIONID;
}

void EOS_P2P_RemoveNotifyIncomingPacketQueueFull(
    EOS_HP2P Handle,
    EOS_NotificationId NotificationId
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) return;

    for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
        P2PNotification* notif = &state->queue_full_notifs[i];
        if (!notif->active) continue;
        if (notif->id != NotificationId) continue;

        notif->active = false;
        EOS_LOG_DEBUG("P2P: Removed queue full notification (ID: %llu)", (unsigned long long)NotificationId);
        return;
    }
}

EOS_EResult EOS_P2P_ClearPacketQueue(
    EOS_HP2P Handle,
    const EOS_P2P_ClearPacketQueueOptions* Options
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != P2P_MAGIC) {
        return EOS_InvalidParameters;
    }

    if (!Options) {
        return EOS_InvalidParameters;
    }

    if (Options->ApiVersion != EOS_P2P_CLEARPACKETQUEUE_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!Options->LocalUserId) {
        return EOS_InvalidUser;
    }

    // Clear packets matching filters
    if (Options->RemoteUserId) {
        // Clear packets for specific remote user
        for (int i = 0; i < MAX_RECV_QUEUE; i++) {
            ReceivedPacket* pkt = &state->recv_queue[i];
            if (!pkt->valid) continue;
            if (!product_user_id_equal(pkt->sender, Options->RemoteUserId)) continue;
            if (Options->SocketId && !socket_id_equal(&pkt->socket_id, Options->SocketId)) continue;

            pkt->valid = false;
            state->incoming_queue_current_bytes -= pkt->size;
        }

        for (int i = 0; i < MAX_SEND_QUEUE; i++) {
            PendingPacket* pkt = &state->send_queue[i];
            if (!pkt->valid) continue;
            if (!product_user_id_equal(pkt->target, Options->RemoteUserId)) continue;
            if (Options->SocketId && !socket_id_equal(&pkt->socket_id, Options->SocketId)) continue;

            pkt->valid = false;
            state->outgoing_queue_current_bytes -= pkt->size;
        }
    } else {
        // Clear all packets
        for (int i = 0; i < MAX_RECV_QUEUE; i++) {
            state->recv_queue[i].valid = false;
        }
        for (int i = 0; i < MAX_SEND_QUEUE; i++) {
            state->send_queue[i].valid = false;
        }
        state->recv_count = 0;
        state->send_count = 0;
        state->recv_head = 0;
        state->recv_tail = 0;
        state->send_head = 0;
        state->send_tail = 0;
        state->incoming_queue_current_bytes = 0;
        state->outgoing_queue_current_bytes = 0;
    }

    EOS_LOG_DEBUG("P2P: Cleared packet queues");
    return EOS_Success;
}
