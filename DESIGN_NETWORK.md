# LAN Network Layer Implementation Guide

## Scope
You are implementing the **LAN Network Layer** - shared infrastructure for UDP networking.

**Your responsibility:**
- Discovery service (UDP broadcast for session announcements)
- P2P socket manager (UDP for game traffic)
- Address resolution and caching
- Cross-platform socket abstraction

**NOT your responsibility:**
- Session logic (that's the sessions module)
- P2P connection state machine (that's the P2P module)
- EOS API (this is internal infrastructure)

This module is **shared infrastructure** used by Sessions and P2P modules.

---

## File Locations

**You will create:**
- `src/lan_discovery.c` - Session discovery service
- `src/lan_discovery.h` - Discovery API
- `src/lan_p2p.c` - P2P socket manager
- `src/lan_p2p.h` - P2P socket API
- `src/lan_common.c` - Shared utilities (IP, sockets)
- `src/lan_common.h` - Shared definitions

---

## Component 1: Discovery Service

### API (lan_discovery.h)

```c
#ifndef EOS_LAN_DISCOVERY_H
#define EOS_LAN_DISCOVERY_H

#include <stdint.h>
#include <stdbool.h>

// Forward declaration
typedef struct Session Session;

typedef struct DiscoveryService DiscoveryService;

/**
 * Create discovery service.
 *
 * @param port UDP port for discovery (default: 23456)
 * @return Service handle or NULL on failure
 */
DiscoveryService* discovery_create(uint16_t port);

/**
 * Destroy discovery service.
 */
void discovery_destroy(DiscoveryService* ds);

/**
 * Broadcast a session announcement.
 * Called periodically by sessions module for active sessions.
 *
 * @param ds Service handle
 * @param session Session to announce
 */
void discovery_broadcast_session(DiscoveryService* ds, const Session* session);

/**
 * Send a query to request session announcements.
 * Other instances should respond with their sessions.
 *
 * @param ds Service handle
 * @param bucket_filter Only request sessions with this bucket ID (NULL = all)
 */
void discovery_send_query(DiscoveryService* ds, const char* bucket_filter);

/**
 * Poll for incoming announcements.
 * Call this regularly (e.g., every tick).
 *
 * @param ds Service handle
 */
void discovery_poll(DiscoveryService* ds);

/**
 * Get discovered sessions.
 * Returns internal array, do not free.
 *
 * @param ds Service handle
 * @param out_count Receives number of sessions
 * @return Array of discovered sessions
 */
Session* discovery_get_sessions(DiscoveryService* ds, int* out_count);

/**
 * Clear all discovered sessions.
 */
void discovery_clear_sessions(DiscoveryService* ds);

/**
 * Set broadcast address (default: 255.255.255.255).
 */
void discovery_set_broadcast_addr(DiscoveryService* ds, const char* addr);

#endif // EOS_LAN_DISCOVERY_H
```

### Internal Structure

```c
// In lan_discovery.c

#define DISCOVERY_MAGIC "EOSLAN"
#define DISCOVERY_VERSION 0x0001
#define MSG_ANNOUNCE 0x01
#define MSG_QUERY 0x02

#define MAX_CACHED_SESSIONS 64
#define MAX_PACKET_SIZE 4096

typedef struct {
    Session session;
    char source_ip[16];
    uint64_t received_at;
} CachedSession;

struct DiscoveryService {
    int socket_fd;
    uint16_t port;
    char broadcast_addr[16];

    CachedSession cache[MAX_CACHED_SESSIONS];
    int cache_count;

    uint8_t recv_buffer[MAX_PACKET_SIZE];
    uint8_t send_buffer[MAX_PACKET_SIZE];
};
```

### Implementation Details

```c
DiscoveryService* discovery_create(uint16_t port) {
    DiscoveryService* ds = calloc(1, sizeof(DiscoveryService));
    if (!ds) return NULL;

    ds->port = port ? port : 23456;
    strcpy(ds->broadcast_addr, "255.255.255.255");

    // Create UDP socket
    ds->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (ds->socket_fd < 0) {
        free(ds);
        return NULL;
    }

    // Enable broadcast
    int broadcast = 1;
    setsockopt(ds->socket_fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

    // Enable address reuse (for multiple instances on same machine)
    int reuse = 1;
    setsockopt(ds->socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
#ifdef SO_REUSEPORT
    setsockopt(ds->socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
#endif

    // Bind to port
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(ds->port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(ds->socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(ds->socket_fd);
        free(ds);
        return NULL;
    }

    // Set non-blocking
    int flags = fcntl(ds->socket_fd, F_GETFL, 0);
    fcntl(ds->socket_fd, F_SETFL, flags | O_NONBLOCK);

    return ds;
}

void discovery_broadcast_session(DiscoveryService* ds, const Session* session) {
    if (!ds || !session) return;

    // Build announcement packet
    uint8_t* buf = ds->send_buffer;
    int offset = 0;

    // Header
    memcpy(buf + offset, DISCOVERY_MAGIC, 6); offset += 6;
    *(uint16_t*)(buf + offset) = htons(DISCOVERY_VERSION); offset += 2;
    buf[offset++] = MSG_ANNOUNCE;

    // Session data
    memset(buf + offset, 0, 64);
    strncpy((char*)(buf + offset), session->session_id, 64); offset += 64;

    memset(buf + offset, 0, 256);
    strncpy((char*)(buf + offset), session->session_name, 256); offset += 256;

    memset(buf + offset, 0, 256);
    strncpy((char*)(buf + offset), session->bucket_id, 256); offset += 256;

    memset(buf + offset, 0, 64);
    strncpy((char*)(buf + offset), session->host_address, 64); offset += 64;

    memset(buf + offset, 0, 32);
    strncpy((char*)(buf + offset), session->owner_id_string, 32); offset += 32;

    *(uint32_t*)(buf + offset) = htonl(session->max_players); offset += 4;
    *(uint32_t*)(buf + offset) = htonl(session->registered_player_count); offset += 4;
    buf[offset++] = (uint8_t)session->state;

    uint8_t flags = 0;
    if (session->join_in_progress_allowed) flags |= 0x01;
    if (session->permission_level == EOS_OSPF_PublicAdvertised) flags |= 0x02;
    buf[offset++] = flags;

    // Attributes
    *(uint16_t*)(buf + offset) = htons(session->attribute_count); offset += 2;
    for (int i = 0; i < session->attribute_count; i++) {
        offset += serialize_attribute(buf + offset, &session->attributes[i]);
    }

    // Send broadcast
    struct sockaddr_in dest = {0};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(ds->port);
    inet_pton(AF_INET, ds->broadcast_addr, &dest.sin_addr);

    sendto(ds->socket_fd, buf, offset, 0, (struct sockaddr*)&dest, sizeof(dest));
}

void discovery_poll(DiscoveryService* ds) {
    if (!ds) return;

    struct sockaddr_in from;
    socklen_t from_len = sizeof(from);

    while (true) {
        ssize_t len = recvfrom(ds->socket_fd, ds->recv_buffer, MAX_PACKET_SIZE, 0,
                               (struct sockaddr*)&from, &from_len);
        if (len <= 0) break;

        // Parse header
        if (len < 9) continue;
        if (memcmp(ds->recv_buffer, DISCOVERY_MAGIC, 6) != 0) continue;

        uint16_t version = ntohs(*(uint16_t*)(ds->recv_buffer + 6));
        if (version != DISCOVERY_VERSION) continue;

        uint8_t msg_type = ds->recv_buffer[8];

        if (msg_type == MSG_ANNOUNCE) {
            Session session;
            if (parse_announcement(ds->recv_buffer + 9, len - 9, &session)) {
                // Get source IP
                char source_ip[16];
                inet_ntop(AF_INET, &from.sin_addr, source_ip, sizeof(source_ip));

                // Don't cache our own announcements
                // (check by comparing host_address or owner_id)

                // Add to cache
                add_to_cache(ds, &session, source_ip);
            }
        } else if (msg_type == MSG_QUERY) {
            // Another instance is searching - respond with our sessions
            // (This is handled by the sessions module calling broadcast_session)
        }
    }
}
```

---

## Component 2: P2P Socket Manager

### API (lan_p2p.h)

```c
#ifndef EOS_LAN_P2P_H
#define EOS_LAN_P2P_H

#include <stdint.h>
#include <stdbool.h>

typedef struct P2PSocketManager P2PSocketManager;

// Received packet info
typedef struct {
    char sender_id[33];
    char sender_addr[64];
    char socket_name[33];
    uint8_t channel;
    uint8_t message_type;  // DATA, CONNECT, ACCEPT, CLOSE
    uint8_t* data;
    uint32_t data_len;
    uint32_t sequence;
    bool reliable;
} P2PReceivedPacket;

// Packet to send
typedef struct {
    const char* target_addr;  // "IP:port"
    const char* sender_id;
    const char* socket_name;
    uint8_t channel;
    uint8_t message_type;
    const uint8_t* data;
    uint32_t data_len;
    uint32_t sequence;
    bool reliable;
} P2PSendPacket;

/**
 * Create P2P socket manager.
 *
 * @param port Base port for P2P (will try sequential if busy)
 * @return Manager handle or NULL on failure
 */
P2PSocketManager* lan_p2p_create(uint16_t port);

/**
 * Destroy P2P socket manager.
 */
void lan_p2p_destroy(P2PSocketManager* mgr);

/**
 * Get the port we're bound to.
 */
uint16_t lan_p2p_get_port(P2PSocketManager* mgr);

/**
 * Get local IP address (for host_address).
 */
const char* lan_p2p_get_local_ip(P2PSocketManager* mgr);

/**
 * Send a packet to a peer.
 *
 * @param mgr Manager handle
 * @param packet Packet to send
 * @return true if sent successfully
 */
bool lan_p2p_send(P2PSocketManager* mgr, const P2PSendPacket* packet);

/**
 * Receive a packet.
 * Returns false if no packet available.
 *
 * @param mgr Manager handle
 * @param out_packet Receives packet data (buffer owned by manager, valid until next call)
 * @return true if packet received
 */
bool lan_p2p_recv(P2PSocketManager* mgr, P2PReceivedPacket* out_packet);

#endif // EOS_LAN_P2P_H
```

### Internal Structure

```c
// In lan_p2p.c

#define P2P_MAGIC "EOSP2P"
#define P2P_VERSION 0x01
#define P2P_MSG_DATA 0x01
#define P2P_MSG_CONNECT 0x02
#define P2P_MSG_ACCEPT 0x03
#define P2P_MSG_CLOSE 0x04

#define MAX_P2P_PACKET 4096

struct P2PSocketManager {
    int socket_fd;
    uint16_t port;
    char local_ip[16];

    uint8_t recv_buffer[MAX_P2P_PACKET];
    uint8_t send_buffer[MAX_P2P_PACKET];

    // Last received packet data (for returning to caller)
    uint8_t last_recv_data[MAX_P2P_PACKET];
    uint32_t last_recv_len;
};
```

### Implementation Details

```c
P2PSocketManager* lan_p2p_create(uint16_t base_port) {
    P2PSocketManager* mgr = calloc(1, sizeof(P2PSocketManager));
    if (!mgr) return NULL;

    // Create UDP socket
    mgr->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (mgr->socket_fd < 0) {
        free(mgr);
        return NULL;
    }

    // Enable address reuse
    int reuse = 1;
    setsockopt(mgr->socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // Try to bind to port (try several if busy)
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;

    for (int i = 0; i < 10; i++) {
        mgr->port = base_port + i;
        addr.sin_port = htons(mgr->port);

        if (bind(mgr->socket_fd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
            break;
        }
        if (i == 9) {
            close(mgr->socket_fd);
            free(mgr);
            return NULL;
        }
    }

    // Set non-blocking
    int flags = fcntl(mgr->socket_fd, F_GETFL, 0);
    fcntl(mgr->socket_fd, F_SETFL, flags | O_NONBLOCK);

    // Get local IP
    get_local_ip(mgr->local_ip, sizeof(mgr->local_ip));

    return mgr;
}

bool lan_p2p_send(P2PSocketManager* mgr, const P2PSendPacket* packet) {
    if (!mgr || !packet || !packet->target_addr) return false;

    // Parse target address "IP:port"
    char ip[16];
    uint16_t port;
    if (!parse_address(packet->target_addr, ip, &port)) {
        return false;
    }

    // Build packet
    uint8_t* buf = mgr->send_buffer;
    int offset = 0;

    memcpy(buf + offset, P2P_MAGIC, 6); offset += 6;
    buf[offset++] = P2P_VERSION;
    buf[offset++] = packet->message_type;

    memset(buf + offset, 0, 32);
    if (packet->sender_id) strncpy((char*)(buf + offset), packet->sender_id, 32);
    offset += 32;

    memset(buf + offset, 0, 32);
    if (packet->socket_name) strncpy((char*)(buf + offset), packet->socket_name, 32);
    offset += 32;

    buf[offset++] = packet->channel;

    uint8_t flags = 0;
    if (packet->reliable) flags |= 0x01;
    buf[offset++] = flags;

    *(uint32_t*)(buf + offset) = htonl(packet->sequence); offset += 4;
    *(uint32_t*)(buf + offset) = htonl(packet->data_len); offset += 4;

    if (packet->data && packet->data_len > 0) {
        if (packet->data_len > MAX_P2P_PACKET - offset) {
            return false;  // Too large
        }
        memcpy(buf + offset, packet->data, packet->data_len);
        offset += packet->data_len;
    }

    // Send
    struct sockaddr_in dest = {0};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    inet_pton(AF_INET, ip, &dest.sin_addr);

    ssize_t sent = sendto(mgr->socket_fd, buf, offset, 0, (struct sockaddr*)&dest, sizeof(dest));
    return sent == offset;
}

bool lan_p2p_recv(P2PSocketManager* mgr, P2PReceivedPacket* out) {
    if (!mgr || !out) return false;

    struct sockaddr_in from;
    socklen_t from_len = sizeof(from);

    ssize_t len = recvfrom(mgr->socket_fd, mgr->recv_buffer, MAX_P2P_PACKET, 0,
                           (struct sockaddr*)&from, &from_len);
    if (len <= 0) return false;

    // Parse header
    if (len < 82) return false;  // Minimum header size
    if (memcmp(mgr->recv_buffer, P2P_MAGIC, 6) != 0) return false;
    if (mgr->recv_buffer[6] != P2P_VERSION) return false;

    uint8_t* buf = mgr->recv_buffer;
    int offset = 7;

    out->message_type = buf[offset++];

    memcpy(out->sender_id, buf + offset, 32);
    out->sender_id[32] = '\0';
    offset += 32;

    memcpy(out->socket_name, buf + offset, 32);
    out->socket_name[32] = '\0';
    offset += 32;

    out->channel = buf[offset++];

    uint8_t flags = buf[offset++];
    out->reliable = (flags & 0x01) != 0;

    out->sequence = ntohl(*(uint32_t*)(buf + offset)); offset += 4;
    out->data_len = ntohl(*(uint32_t*)(buf + offset)); offset += 4;

    // Get sender address
    char ip[16];
    inet_ntop(AF_INET, &from.sin_addr, ip, sizeof(ip));
    snprintf(out->sender_addr, sizeof(out->sender_addr), "%s:%d", ip, ntohs(from.sin_port));

    // Copy payload
    if (out->data_len > 0 && offset + out->data_len <= len) {
        memcpy(mgr->last_recv_data, buf + offset, out->data_len);
        out->data = mgr->last_recv_data;
    } else {
        out->data = NULL;
        out->data_len = 0;
    }

    return true;
}
```

---

## Component 3: Common Utilities

### API (lan_common.h)

```c
#ifndef EOS_LAN_COMMON_H
#define EOS_LAN_COMMON_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Get local IP address (first non-loopback interface).
 */
bool get_local_ip(char* out_ip, int out_size);

/**
 * Parse "IP:port" string.
 */
bool parse_address(const char* addr, char* out_ip, uint16_t* out_port);

/**
 * Format "IP:port" string.
 */
void format_address(char* out, int out_size, const char* ip, uint16_t port);

/**
 * Get current time in milliseconds.
 */
uint64_t get_time_ms(void);

/**
 * CRC32 checksum.
 */
uint32_t crc32(const void* data, size_t len);

#endif // EOS_LAN_COMMON_H
```

### Implementation

```c
// In lan_common.c

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

bool get_local_ip(char* out_ip, int out_size) {
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1) {
        strcpy(out_ip, "127.0.0.1");
        return false;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        if (ifa->ifa_addr->sa_family != AF_INET) continue;

        struct sockaddr_in* addr = (struct sockaddr_in*)ifa->ifa_addr;

        // Skip loopback
        if (addr->sin_addr.s_addr == htonl(INADDR_LOOPBACK)) continue;

        inet_ntop(AF_INET, &addr->sin_addr, out_ip, out_size);
        freeifaddrs(ifaddr);
        return true;
    }

    freeifaddrs(ifaddr);
    strcpy(out_ip, "127.0.0.1");
    return false;
}

bool parse_address(const char* addr, char* out_ip, uint16_t* out_port) {
    if (!addr) return false;

    const char* colon = strchr(addr, ':');
    if (!colon) return false;

    int ip_len = colon - addr;
    if (ip_len >= 16) return false;

    strncpy(out_ip, addr, ip_len);
    out_ip[ip_len] = '\0';

    *out_port = (uint16_t)atoi(colon + 1);
    return *out_port > 0;
}

void format_address(char* out, int out_size, const char* ip, uint16_t port) {
    snprintf(out, out_size, "%s:%d", ip, port);
}

uint64_t get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}
```

---

## Windows Compatibility

For Windows builds, replace POSIX socket calls:

```c
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

// Replace close() with closesocket()
// Replace fcntl() with ioctlsocket()
// Use WSAStartup() at init

#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#endif
```

---

## Testing Criteria

1. **Discovery broadcast/receive:**
   ```c
   DiscoveryService* ds1 = discovery_create(23456);
   DiscoveryService* ds2 = discovery_create(23456);

   Session session = { .session_id = "TEST123", .session_name = "Test" };
   discovery_broadcast_session(ds1, &session);

   // Wait a bit
   usleep(10000);

   discovery_poll(ds2);
   int count;
   Session* found = discovery_get_sessions(ds2, &count);
   assert(count == 1);
   assert(strcmp(found[0].session_id, "TEST123") == 0);
   ```

2. **P2P send/receive:**
   ```c
   P2PSocketManager* p1 = lan_p2p_create(23457);
   P2PSocketManager* p2 = lan_p2p_create(23458);

   char addr[64];
   format_address(addr, sizeof(addr), lan_p2p_get_local_ip(p2), lan_p2p_get_port(p2));

   P2PSendPacket pkt = {
       .target_addr = addr,
       .sender_id = "SENDER1",
       .socket_name = "Game",
       .message_type = P2P_MSG_DATA,
       .data = (uint8_t*)"Hello",
       .data_len = 5
   };
   assert(lan_p2p_send(p1, &pkt));

   usleep(10000);

   P2PReceivedPacket recv;
   assert(lan_p2p_recv(p2, &recv));
   assert(recv.data_len == 5);
   assert(memcmp(recv.data, "Hello", 5) == 0);
   ```

---

## Build Instructions

```makefile
$(BUILD_DIR)/lan_common.o: src/lan_common.c src/lan_common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lan_discovery.o: src/lan_discovery.c src/lan_discovery.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lan_p2p.o: src/lan_p2p.c src/lan_p2p.h
	$(CC) $(CFLAGS) -c $< -o $@
```

Dependencies:
- Standard C library
- POSIX sockets (or Winsock on Windows)
- sessions_internal.h (for Session struct definition)
