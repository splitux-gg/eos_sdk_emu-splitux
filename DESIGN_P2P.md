# P2P Layer Implementation Guide

## Scope
You are implementing the **P2P Layer** - peer-to-peer packet delivery for game traffic.

**Your responsibility:**
- Sending packets to peers (by ProductUserId)
- Receiving packets from peers
- Connection lifecycle (request, accept, established, closed)
- Packet queuing and delivery
- Socket/Channel abstraction

**NOT your responsibility:**
- NAT traversal (LAN = direct connection)
- Relay servers (not needed for LAN)
- Session discovery (that's the sessions module)
- Low-level socket I/O (use the network module)

---

## File Locations

**You will create:**
- `src/p2p.c` - Implementation
- `src/p2p_internal.h` - Internal structures

**You will use:**
- `include/eos/eos_p2p.h` - Official EOS headers
- `include/eos/eos_p2p_types.h` - Type definitions
- `src/callbacks.h` - For queuing callbacks
- `src/platform_internal.h` - Platform state access
- `src/lan_p2p.h` - LAN P2P socket manager

---

## API Functions to Implement

### Packet Transmission

```c
/**
 * Send a packet to a peer.
 *
 * For LAN:
 * - Look up peer's address in address book
 * - Send directly via UDP
 * - Queue packet if connection not yet established
 *
 * @return EOS_Success if queued/sent
 *         EOS_InvalidParameters if options invalid
 *         EOS_LimitExceeded if packet too large or queue full
 *         EOS_NoConnection if auto-accept disabled and not connected
 */
EOS_EResult EOS_P2P_SendPacket(
    EOS_HP2P Handle,
    const EOS_P2P_SendPacketOptions* Options
);

/**
 * Get size of next packet waiting to be received.
 *
 * @return EOS_Success if packet available
 *         EOS_NotFound if no packets waiting
 */
EOS_EResult EOS_P2P_GetNextReceivedPacketSize(
    EOS_HP2P Handle,
    const EOS_P2P_GetNextReceivedPacketSizeOptions* Options,
    uint32_t* OutPacketSizeBytes
);

/**
 * Receive a packet.
 *
 * @return EOS_Success if packet received
 *         EOS_NotFound if no packets waiting
 */
EOS_EResult EOS_P2P_ReceivePacket(
    EOS_HP2P Handle,
    const EOS_P2P_ReceivePacketOptions* Options,
    EOS_ProductUserId* OutPeerId,
    EOS_P2P_SocketId* OutSocketId,
    uint8_t* OutChannel,
    void* OutData,
    uint32_t* OutBytesWritten
);
```

### Connection Management

```c
/**
 * Accept a connection from a peer on a specific socket.
 *
 * For LAN:
 * - Add socket to accepted list
 * - Auto-accept is usually enabled, so this may not be called
 */
EOS_EResult EOS_P2P_AcceptConnection(
    EOS_HP2P Handle,
    const EOS_P2P_AcceptConnectionOptions* Options
);

/**
 * Close connection to a specific peer on a socket.
 */
EOS_EResult EOS_P2P_CloseConnection(
    EOS_HP2P Handle,
    const EOS_P2P_CloseConnectionOptions* Options
);

/**
 * Close all connections on a socket.
 */
EOS_EResult EOS_P2P_CloseConnections(
    EOS_HP2P Handle,
    const EOS_P2P_CloseConnectionsOptions* Options
);

/**
 * Get connection info for a peer.
 */
EOS_EResult EOS_P2P_GetConnectionEstablishedInfo(
    EOS_HP2P Handle,
    const EOS_P2P_GetConnectionEstablishedInfoOptions* Options,
    EOS_P2P_ConnectionEstablishedInfo* OutConnectionEstablishedInfo
);
```

### Notifications

```c
/**
 * Called when a peer requests to connect.
 * For LAN with auto-accept, this fires then immediately establishes.
 */
EOS_NotificationId EOS_P2P_AddNotifyPeerConnectionRequest(
    EOS_HP2P Handle,
    const EOS_P2P_AddNotifyPeerConnectionRequestOptions* Options,
    void* ClientData,
    EOS_P2P_OnIncomingConnectionRequestCallback ConnectionRequestHandler
);
void EOS_P2P_RemoveNotifyPeerConnectionRequest(EOS_HP2P Handle, EOS_NotificationId NotificationId);

/**
 * Called when connection is established.
 */
EOS_NotificationId EOS_P2P_AddNotifyPeerConnectionEstablished(
    EOS_HP2P Handle,
    const EOS_P2P_AddNotifyPeerConnectionEstablishedOptions* Options,
    void* ClientData,
    EOS_P2P_OnPeerConnectionEstablishedCallback ConnectionEstablishedHandler
);
void EOS_P2P_RemoveNotifyPeerConnectionEstablished(EOS_HP2P Handle, EOS_NotificationId NotificationId);

/**
 * Called when connection is interrupted (temporary).
 */
EOS_NotificationId EOS_P2P_AddNotifyPeerConnectionInterrupted(
    EOS_HP2P Handle,
    const EOS_P2P_AddNotifyPeerConnectionInterruptedOptions* Options,
    void* ClientData,
    EOS_P2P_OnPeerConnectionInterruptedCallback ConnectionInterruptedHandler
);
void EOS_P2P_RemoveNotifyPeerConnectionInterrupted(EOS_HP2P Handle, EOS_NotificationId NotificationId);

/**
 * Called when connection is closed.
 */
EOS_NotificationId EOS_P2P_AddNotifyPeerConnectionClosed(
    EOS_HP2P Handle,
    const EOS_P2P_AddNotifyPeerConnectionClosedOptions* Options,
    void* ClientData,
    EOS_P2P_OnRemoteConnectionClosedCallback ConnectionClosedHandler
);
void EOS_P2P_RemoveNotifyPeerConnectionClosed(EOS_HP2P Handle, EOS_NotificationId NotificationId);

/**
 * Called when an incoming packet is ready.
 */
EOS_NotificationId EOS_P2P_AddNotifyIncomingPacketQueueFull(
    EOS_HP2P Handle,
    const EOS_P2P_AddNotifyIncomingPacketQueueFullOptions* Options,
    void* ClientData,
    EOS_P2P_OnIncomingPacketQueueFullCallback IncomingPacketQueueFullHandler
);
void EOS_P2P_RemoveNotifyIncomingPacketQueueFull(EOS_HP2P Handle, EOS_NotificationId NotificationId);
```

### NAT Type (Fake for LAN)

```c
/**
 * Query NAT type. For LAN, always returns Open.
 */
void EOS_P2P_QueryNATType(
    EOS_HP2P Handle,
    const EOS_P2P_QueryNATTypeOptions* Options,
    void* ClientData,
    const EOS_P2P_OnQueryNATTypeCompleteCallback NATTypeQueriedHandler
);

/**
 * Get cached NAT type.
 */
EOS_EResult EOS_P2P_GetNATType(
    EOS_HP2P Handle,
    const EOS_P2P_GetNATTypeOptions* Options,
    EOS_ENATType* OutNATType
);
```

### Relay Control (Stub)

```c
EOS_EResult EOS_P2P_SetRelayControl(EOS_HP2P Handle, const EOS_P2P_SetRelayControlOptions* Options);
EOS_EResult EOS_P2P_GetRelayControl(EOS_HP2P Handle, const EOS_P2P_GetRelayControlOptions* Options, EOS_ERelayControl* OutRelayControl);
EOS_EResult EOS_P2P_SetPortRange(EOS_HP2P Handle, const EOS_P2P_SetPortRangeOptions* Options);
EOS_EResult EOS_P2P_GetPortRange(EOS_HP2P Handle, const EOS_P2P_GetPortRangeOptions* Options, uint16_t* OutPort, uint16_t* OutNumAdditionalPortsToTry);
EOS_EResult EOS_P2P_SetPacketQueueSize(EOS_HP2P Handle, const EOS_P2P_SetPacketQueueSizeOptions* Options);
EOS_EResult EOS_P2P_GetPacketQueueInfo(EOS_HP2P Handle, const EOS_P2P_GetPacketQueueInfoOptions* Options, EOS_P2P_PacketQueueInfo* OutPacketQueueInfo);
void EOS_P2P_ClearPacketQueue(EOS_HP2P Handle, const EOS_P2P_ClearPacketQueueOptions* Options);
```

---

## Data Structures

### p2p_internal.h

```c
#ifndef EOS_LAN_P2P_INTERNAL_H
#define EOS_LAN_P2P_INTERNAL_H

#include "eos_p2p_types.h"
#include "platform_internal.h"
#include <stdbool.h>
#include <stdint.h>

#define MAX_PACKET_SIZE 4096
#define MAX_RECV_QUEUE 512
#define MAX_SEND_QUEUE 512
#define MAX_CONNECTIONS 64
#define MAX_ACCEPTED_SOCKETS 16
#define SOCKET_NAME_MAX 32

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
    P2PNotification conn_request_notifs[8];
    P2PNotification conn_established_notifs[8];
    P2PNotification conn_interrupted_notifs[8];
    P2PNotification conn_closed_notifs[8];
    P2PNotification queue_full_notifs[8];
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

} P2PState;

// Creation/destruction
P2PState* p2p_create(PlatformState* platform);
void p2p_destroy(P2PState* state);
void p2p_tick(P2PState* state);

// Address book
void p2p_register_peer_address(P2PState* state, EOS_ProductUserId peer, const char* address);
const char* p2p_get_peer_address(P2PState* state, EOS_ProductUserId peer);

#endif // EOS_LAN_P2P_INTERNAL_H
```

---

## LAN P2P Protocol

### Packet Header

```
Offset  Size  Field
------  ----  -----
0       6     Magic "EOSP2P"
6       1     Version (0x01)
7       1     Message Type (DATA=0x01, CONNECT=0x02, ACCEPT=0x03, CLOSE=0x04)
8       32    Sender ID (null-padded)
40      32    Socket Name (null-padded)
72      1     Channel
73      1     Flags (bit 0: reliable, bit 1: fragment)
74      4     Sequence Number (uint32 LE)
78      4     Payload Length (uint32 LE)
82      N     Payload
```

### Connection Handshake

```
Peer A                                    Peer B
   |                                         |
   |-- CONNECT (socket="GameSocket") ------->|
   |                                         |
   |<-- ACCEPT (socket="GameSocket") --------|
   |                                         |
   |   [Connection Established]              |
   |                                         |
   |-- DATA (payload=...) ------------------>|
   |<-- DATA (payload=...) ------------------|
   |                                         |
```

---

## Key Implementation Details

### EOS_P2P_SendPacket

```c
EOS_EResult EOS_P2P_SendPacket(
    EOS_HP2P Handle,
    const EOS_P2P_SendPacketOptions* Options
) {
    P2PState* state = (P2PState*)Handle;
    if (!state || state->magic != 0x50325032) return EOS_InvalidParameters;
    if (!Options || !Options->RemoteUserId || !Options->Data) return EOS_InvalidParameters;
    if (Options->DataLengthBytes > MAX_PACKET_SIZE) return EOS_LimitExceeded;

    // Look up connection
    PeerConnection* conn = find_connection(state, Options->RemoteUserId, &Options->SocketId);

    if (conn && conn->state == CONN_STATE_ESTABLISHED) {
        // Send directly
        return send_packet_to_peer(state, conn, Options);
    }

    // No established connection
    if (Options->bDisableAutoAcceptConnection) {
        return EOS_NoConnection;
    }

    // Start connection if needed
    if (!conn) {
        conn = create_connection(state, Options->RemoteUserId, &Options->SocketId);
        if (!conn) return EOS_LimitExceeded;

        // Look up address from address book
        const char* addr = p2p_get_peer_address(state, Options->RemoteUserId);
        if (addr) {
            strncpy(conn->peer_address, addr, sizeof(conn->peer_address) - 1);
        }

        // Send CONNECT
        send_connect_message(state, conn);
        conn->state = CONN_STATE_REQUESTING;
    }

    // Queue packet for when connected
    if (Options->bAllowDelayedDelivery) {
        return queue_pending_packet(state, Options);
    }

    return EOS_NoConnection;
}
```

### EOS_P2P_ReceivePacket

```c
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
    if (!state || state->magic != 0x50325032) return EOS_InvalidParameters;

    // Find first packet matching filters
    for (int i = 0; i < state->recv_count; i++) {
        int idx = (state->recv_head + i) % MAX_RECV_QUEUE;
        ReceivedPacket* pkt = &state->recv_queue[idx];

        if (!pkt->valid) continue;

        // Check socket filter
        if (Options->RequestedChannel && *Options->RequestedChannel != pkt->channel) {
            continue;
        }

        // Check size
        if (pkt->size > Options->MaxDataSizeBytes) {
            // Packet will be truncated
        }

        // Copy out
        if (OutPeerId) *OutPeerId = pkt->sender;
        if (OutSocketId) *OutSocketId = pkt->socket_id;
        if (OutChannel) *OutChannel = pkt->channel;

        uint32_t copy_size = (pkt->size < Options->MaxDataSizeBytes) ? pkt->size : Options->MaxDataSizeBytes;
        memcpy(OutData, pkt->data, copy_size);
        if (OutBytesWritten) *OutBytesWritten = copy_size;

        // Remove from queue
        pkt->valid = false;
        while (state->recv_count > 0 && !state->recv_queue[state->recv_head].valid) {
            state->recv_head = (state->recv_head + 1) % MAX_RECV_QUEUE;
            state->recv_count--;
        }

        return EOS_Success;
    }

    return EOS_NotFound;
}
```

### p2p_tick

```c
void p2p_tick(P2PState* state) {
    // Receive from network
    while (true) {
        ReceivedPacket pkt;
        if (!lan_p2p_recv(state->platform->p2p_socket, &pkt)) {
            break;
        }

        // Handle based on message type
        switch (pkt.message_type) {
            case MSG_CONNECT:
                handle_connect_request(state, &pkt);
                break;
            case MSG_ACCEPT:
                handle_connect_accept(state, &pkt);
                break;
            case MSG_CLOSE:
                handle_close(state, &pkt);
                break;
            case MSG_DATA:
                handle_data_packet(state, &pkt);
                break;
        }
    }

    // Process pending send queue for newly established connections
    for (int i = 0; i < state->send_count; i++) {
        int idx = (state->send_head + i) % MAX_SEND_QUEUE;
        PendingPacket* pkt = &state->send_queue[idx];
        if (!pkt->valid) continue;

        PeerConnection* conn = find_connection(state, pkt->target, &pkt->socket_id);
        if (conn && conn->state == CONN_STATE_ESTABLISHED) {
            send_queued_packet(state, conn, pkt);
            pkt->valid = false;
        }
    }

    // Fire notifications for queue changes, timeouts, etc.
    check_connection_timeouts(state);
}
```

---

## Address Book Integration

The P2P module needs to know peer IP addresses. These come from:

1. **Session host_address** - When joining a session
2. **Session announcements** - Discovered sessions include host IP
3. **Incoming packets** - Learn address from sender

```c
// Called by sessions module when session is discovered/joined
void p2p_register_peer_address(P2PState* state, EOS_ProductUserId peer, const char* address) {
    // Store in address book (find or create entry)
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (state->connections[i].valid &&
            strcmp(state->connections[i].peer_id_string, get_id_string(peer)) == 0) {
            strncpy(state->connections[i].peer_address, address, 63);
            return;
        }
    }

    // Create new entry
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (!state->connections[i].valid) {
            state->connections[i].valid = true;
            state->connections[i].peer_id = peer;
            copy_id_string(peer, state->connections[i].peer_id_string);
            strncpy(state->connections[i].peer_address, address, 63);
            state->connections[i].state = CONN_STATE_NONE;
            state->connection_count++;
            return;
        }
    }
}
```

---

## Testing Criteria

1. **Send/receive loop:**
   ```c
   // Instance A sends
   EOS_P2P_SendPacketOptions opts = {...};
   opts.RemoteUserId = peer_b;
   opts.SocketId = &socket;
   opts.Data = "Hello";
   opts.DataLengthBytes = 5;
   assert(EOS_P2P_SendPacket(p2p_a, &opts) == EOS_Success);

   // Tick both
   EOS_Platform_Tick(platform_a);
   EOS_Platform_Tick(platform_b);

   // Instance B receives
   uint32_t size;
   assert(EOS_P2P_GetNextReceivedPacketSize(p2p_b, &opts2, &size) == EOS_Success);
   assert(size == 5);

   char buf[64];
   uint32_t written;
   assert(EOS_P2P_ReceivePacket(p2p_b, &recv_opts, &from, &sock, &chan, buf, &written) == EOS_Success);
   assert(written == 5);
   assert(memcmp(buf, "Hello", 5) == 0);
   ```

2. **Connection notification:**
   ```c
   static bool connected = false;
   void OnConnected(const EOS_P2P_OnPeerConnectionEstablishedInfo* Data) {
       connected = true;
   }

   EOS_P2P_AddNotifyPeerConnectionEstablished(p2p, &opts, NULL, OnConnected);
   // Send packet to trigger connection
   EOS_P2P_SendPacket(p2p, &send_opts);
   // Tick until connected
   while (!connected) EOS_Platform_Tick(platform);
   ```

3. **NAT type query:**
   ```c
   void OnNAT(const EOS_P2P_OnQueryNATTypeCompleteInfo* Data) {
       assert(Data->NATType == EOS_NAT_Open);  // Always Open for LAN
   }
   EOS_P2P_QueryNATType(p2p, &opts, NULL, OnNAT);
   EOS_Platform_Tick(platform);
   ```

---

## Build Instructions

```makefile
$(BUILD_DIR)/p2p.o: src/p2p.c src/p2p_internal.h
	$(CC) $(CFLAGS) -I include -c $< -o $@
```

Dependencies:
- Standard C library
- EOS SDK headers
- callbacks.h
- platform_internal.h
- lan_p2p.h (from network module)
