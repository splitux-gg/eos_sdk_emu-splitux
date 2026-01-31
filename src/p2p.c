// Copyright Epic Games, Inc. All Rights Reserved.

#include "eos/eos_p2p.h"
#include "eos/eos_p2p_types.h"
#include "internal/p2p_internal.h"
#include "internal/platform_internal.h"
#include "internal/logging.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Magic number for state validation
#define P2P_MAGIC 0x50325032  // "P2P2"

// Default queue sizes (in bytes)
#define DEFAULT_INCOMING_QUEUE_MAX (1024 * 1024)  // 1MB
#define DEFAULT_OUTGOING_QUEUE_MAX (1024 * 1024)  // 1MB

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

// Helper: Copy ProductUserId to string
static void product_user_id_to_string(EOS_ProductUserId user_id, char* out, size_t out_size) {
    if (!user_id || !out || out_size < 33) return;
    snprintf(out, out_size, "%p", (void*)user_id);
}

// Helper: Compare ProductUserIds
static bool product_user_id_equal(EOS_ProductUserId a, EOS_ProductUserId b) {
    return a == b;
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
    state->port_range_start = 7777;
    state->port_range_count = 99;
    state->incoming_queue_max_bytes = DEFAULT_INCOMING_QUEUE_MAX;
    state->outgoing_queue_max_bytes = DEFAULT_OUTGOING_QUEUE_MAX;

    EOS_LOG_INFO("P2P: Created P2P state");
    return state;
}

// Destroy P2P state
void p2p_destroy(P2PState* state) {
    if (!state || state->magic != P2P_MAGIC) return;

    EOS_LOG_INFO("P2P: Destroying P2P state");
    state->magic = 0;
    free(state);
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

    // TODO: Receive packets from network using lan_p2p
    // TODO: Process pending send queue for established connections
    // TODO: Check for connection timeouts
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

    if (Options->ApiVersion != EOS_P2P_SENDPACKET_API_LATEST) {
        EOS_LOG_ERROR("P2P_SendPacket: Invalid API version");
        return EOS_InvalidParameters;
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
        // Connection established - send immediately
        // TODO: Send via lan_p2p
        EOS_LOG_DEBUG("P2P_SendPacket: Sending %u bytes to established connection", Options->DataLengthBytes);
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

        // Try to get peer address
        const char* addr = p2p_get_peer_address(state, Options->RemoteUserId);
        if (addr) {
            strncpy(conn->peer_address, addr, sizeof(conn->peer_address) - 1);
            conn->peer_address[sizeof(conn->peer_address) - 1] = '\0';
        }

        // TODO: Send CONNECT message
        conn->state = CONN_STATE_REQUESTING;
        EOS_LOG_DEBUG("P2P_SendPacket: Initiating connection request");
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
        return EOS_InvalidParameters;
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
        return EOS_InvalidParameters;
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
        return EOS_InvalidParameters;
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

    // Find or create connection
    PeerConnection* conn = find_connection(state, Options->RemoteUserId, Options->SocketId);
    if (conn) {
        if (conn->state == CONN_STATE_PENDING) {
            // TODO: Send ACCEPT message
            conn->state = CONN_STATE_ESTABLISHED;
            conn->established_at = get_time_ms();
            EOS_LOG_DEBUG("P2P: Connection accepted and established");
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
