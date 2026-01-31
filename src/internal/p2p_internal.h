#ifndef EOS_LAN_P2P_INTERNAL_H
#define EOS_LAN_P2P_INTERNAL_H

#include "eos/eos_p2p_types.h"
#include "platform_internal.h"
#include <stdbool.h>
#include <stdint.h>

#define MAX_PACKET_SIZE 4096
#define MAX_RECV_QUEUE 512
#define MAX_SEND_QUEUE 512
#define MAX_CONNECTIONS 64
#define MAX_ACCEPTED_SOCKETS 16
#define SOCKET_NAME_MAX 32
#define MAX_NOTIFICATIONS 8

// Received packet
typedef struct {
    EOS_ProductUserId sender;
    char sender_id_string[33];
    EOS_P2P_SocketId socket_id;
    uint8_t channel;
    uint8_t data[MAX_PACKET_SIZE];
    uint32_t size;
    bool valid;
} ReceivedPacket;

// Pending outgoing packet
typedef struct {
    EOS_ProductUserId target;
    char target_id_string[33];
    EOS_P2P_SocketId socket_id;
    uint8_t channel;
    uint8_t data[MAX_PACKET_SIZE];
    uint32_t size;
    bool reliable;
    bool allow_delayed;
    bool valid;
} PendingPacket;

// Connection state
typedef enum {
    CONN_STATE_NONE,
    CONN_STATE_REQUESTING,   // We sent request, waiting for accept
    CONN_STATE_PENDING,      // They sent request, waiting for our accept
    CONN_STATE_ESTABLISHED,
    CONN_STATE_CLOSED
} ConnectionState;

// Connection to a peer
typedef struct {
    EOS_ProductUserId peer_id;
    char peer_id_string[33];
    char peer_address[64];  // "IP:port"
    EOS_P2P_SocketId socket_id;
    ConnectionState state;
    uint64_t established_at;
    uint64_t last_activity;
    bool valid;
} PeerConnection;

// Accepted socket for auto-accept
typedef struct {
    EOS_P2P_SocketId socket_id;
    EOS_ProductUserId local_user;
    bool valid;
} AcceptedSocket;

// Notification handler
typedef struct {
    void* callback;
    void* client_data;
    EOS_NotificationId id;
    EOS_P2P_SocketId socket_filter;  // NULL = all sockets
    bool has_socket_filter;
    bool active;
} P2PNotification;

// Main P2P state
typedef struct P2PState {
    uint32_t magic;  // 0x50325032 = "P2P2"
    PlatformState* platform;

    // Local user
    EOS_ProductUserId local_user;

    // Connections
    PeerConnection connections[MAX_CONNECTIONS];
    int connection_count;

    // Auto-accept
    bool auto_accept_all;
    AcceptedSocket accepted_sockets[MAX_ACCEPTED_SOCKETS];
    int accepted_socket_count;

    // Receive queue
    ReceivedPacket recv_queue[MAX_RECV_QUEUE];
    int recv_head;
    int recv_tail;
    int recv_count;

    // Send queue (for packets to unconnected peers)
    PendingPacket send_queue[MAX_SEND_QUEUE];
    int send_head;
    int send_tail;
    int send_count;

    // Notifications
    P2PNotification conn_request_notifs[MAX_NOTIFICATIONS];
    P2PNotification conn_established_notifs[MAX_NOTIFICATIONS];
    P2PNotification conn_interrupted_notifs[MAX_NOTIFICATIONS];
    P2PNotification conn_closed_notifs[MAX_NOTIFICATIONS];
    P2PNotification queue_full_notifs[MAX_NOTIFICATIONS];
    EOS_NotificationId next_notif_id;

    // NAT type (always Open for LAN)
    EOS_ENATType nat_type;
    bool nat_queried;

    // Relay settings (ignored for LAN)
    EOS_ERelayControl relay_control;
    uint16_t port_range_start;
    uint16_t port_range_count;

    // Queue size limits
    uint64_t incoming_queue_max_bytes;
    uint64_t outgoing_queue_max_bytes;

    // Current queue sizes (in bytes)
    uint64_t incoming_queue_current_bytes;
    uint64_t outgoing_queue_current_bytes;

} P2PState;

// Creation/destruction
P2PState* p2p_create(PlatformState* platform);
void p2p_destroy(P2PState* state);
void p2p_tick(P2PState* state);

// Address book
void p2p_register_peer_address(P2PState* state, EOS_ProductUserId peer, const char* address);
const char* p2p_get_peer_address(P2PState* state, EOS_ProductUserId peer);

#endif // EOS_LAN_P2P_INTERNAL_H
