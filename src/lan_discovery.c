#include "internal/lan_discovery.h"
#include "lan_common.h"
#include "internal/sessions_internal.h"
#include "internal/logging.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#define close closesocket
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#endif

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
#ifdef _WIN32
    SOCKET socket_fd;
#else
    int socket_fd;
#endif
    uint16_t port;
    char broadcast_addr[16];
    bool localhost_mode;  // Enable localhost unicast for Wine/Proton
    bool query_received;  // Flag to indicate a query was received and we should broadcast

    CachedSession cache[MAX_CACHED_SESSIONS];
    int cache_count;

    uint8_t recv_buffer[MAX_PACKET_SIZE];
    uint8_t send_buffer[MAX_PACKET_SIZE];
};

// Helper function to serialize session attribute
static int serialize_attribute(uint8_t* buf, const SessionAttribute* attr) {
    int offset = 0;

    // Key
    memset(buf + offset, 0, 64);
    strncpy((char*)(buf + offset), attr->key, 64);
    offset += 64;

    // Type
    buf[offset++] = (uint8_t)attr->type;

    // Advertisement type
    buf[offset++] = (uint8_t)attr->advertisement;

    // Value (based on type)
    switch (attr->type) {
        case EOS_SAT_Boolean:
            buf[offset++] = attr->value.as_bool ? 1 : 0;
            break;
        case EOS_SAT_Int64:
            *(int64_t*)(buf + offset) = (int64_t)attr->value.as_int64;
            offset += 8;
            break;
        case EOS_SAT_Double:
            *(double*)(buf + offset) = attr->value.as_double;
            offset += 8;
            break;
        case EOS_SAT_String:
            memset(buf + offset, 0, 256);
            strncpy((char*)(buf + offset), attr->value.as_string, 256);
            offset += 256;
            break;
        default:
            break;
    }

    return offset;
}

// Helper function to deserialize session attribute
static int deserialize_attribute(const uint8_t* buf, int max_len, SessionAttribute* attr) {
    if (max_len < 66) return -1;  // Minimum size
    int offset = 0;

    // Key
    memcpy(attr->key, buf + offset, 64);
    attr->key[63] = '\0';
    offset += 64;

    // Type
    attr->type = (EOS_ESessionAttributeType)buf[offset++];

    // Advertisement type
    attr->advertisement = (EOS_ESessionAttributeAdvertisementType)buf[offset++];

    // Value
    switch (attr->type) {
        case EOS_SAT_Boolean:
            if (offset + 1 > max_len) return -1;
            attr->value.as_bool = buf[offset++] != 0;
            break;
        case EOS_SAT_Int64:
            if (offset + 8 > max_len) return -1;
            attr->value.as_int64 = *(int64_t*)(buf + offset);
            offset += 8;
            break;
        case EOS_SAT_Double:
            if (offset + 8 > max_len) return -1;
            attr->value.as_double = *(double*)(buf + offset);
            offset += 8;
            break;
        case EOS_SAT_String:
            if (offset + 256 > max_len) return -1;
            memcpy(attr->value.as_string, buf + offset, 256);
            attr->value.as_string[255] = '\0';
            offset += 256;
            break;
        default:
            return -1;
    }

    return offset;
}

// Parse announcement packet into Session
static bool parse_announcement(const uint8_t* buf, int len, Session* session) {
    if (len < 643) return false;  // Minimum size for session header
    int offset = 0;

    // Session ID
    memcpy(session->session_id, buf + offset, 64);
    session->session_id[63] = '\0';
    offset += 64;

    // Session name
    memcpy(session->session_name, buf + offset, 256);
    session->session_name[255] = '\0';
    offset += 256;

    // Bucket ID
    memcpy(session->bucket_id, buf + offset, 256);
    session->bucket_id[255] = '\0';
    offset += 256;

    // Host address
    memcpy(session->host_address, buf + offset, 64);
    session->host_address[63] = '\0';
    offset += 64;

    // Owner ID
    memcpy(session->owner_id_string, buf + offset, 32);
    session->owner_id_string[31] = '\0';
    offset += 32;

    if (offset + 10 > len) return false;

    // Max players
    session->max_players = ntohl(*(uint32_t*)(buf + offset));
    offset += 4;

    // Registered player count
    session->registered_player_count = ntohl(*(uint32_t*)(buf + offset));
    offset += 4;

    // State
    session->state = (EOS_EOnlineSessionState)buf[offset++];

    // Flags
    uint8_t flags = buf[offset++];
    session->join_in_progress_allowed = (flags & 0x01) != 0;
    session->permission_level = (flags & 0x02) ? EOS_OSPF_PublicAdvertised : EOS_OSPF_InviteOnly;

    if (offset + 2 > len) return false;

    // Attributes
    uint16_t attr_count = ntohs(*(uint16_t*)(buf + offset));
    offset += 2;

    session->attribute_count = 0;
    for (int i = 0; i < attr_count && i < 64; i++) {
        int consumed = deserialize_attribute(buf + offset, len - offset, &session->attributes[i]);
        if (consumed < 0) break;
        offset += consumed;
        session->attribute_count++;
    }

    // Mark session as valid so it passes search filtering
    session->valid = true;

    return true;
}

// Add session to cache
static void add_to_cache(DiscoveryService* ds, const Session* session, const char* source_ip) {
    if (!ds || !session) return;

    // Check if session already exists in cache (update it)
    for (int i = 0; i < ds->cache_count; i++) {
        if (strcmp(ds->cache[i].session.session_id, session->session_id) == 0) {
            memcpy(&ds->cache[i].session, session, sizeof(Session));
            strncpy(ds->cache[i].source_ip, source_ip, sizeof(ds->cache[i].source_ip) - 1);
            ds->cache[i].received_at = get_time_ms();
            return;
        }
    }

    // Add new session if space available
    if (ds->cache_count < MAX_CACHED_SESSIONS) {
        memcpy(&ds->cache[ds->cache_count].session, session, sizeof(Session));
        strncpy(ds->cache[ds->cache_count].source_ip, source_ip, sizeof(ds->cache[ds->cache_count].source_ip) - 1);
        ds->cache[ds->cache_count].received_at = get_time_ms();
        ds->cache_count++;
    }
}

DiscoveryService* discovery_create(uint16_t port) {
#ifdef _WIN32
    // Initialize Winsock
    static bool winsock_initialized = false;
    if (!winsock_initialized) {
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
            return NULL;
        }
        winsock_initialized = true;
    }
#endif

    DiscoveryService* ds = calloc(1, sizeof(DiscoveryService));
    if (!ds) return NULL;

    ds->port = port ? port : 23456;
    strcpy(ds->broadcast_addr, "255.255.255.255");
    ds->localhost_mode = should_use_localhost_mode();

    if (ds->localhost_mode) {
        EOS_LOG_INFO("Localhost discovery mode enabled (EOSLAN_LOCALHOST_MODE=1)");
    }

    // Create UDP socket
    ds->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
#ifdef _WIN32
    if (ds->socket_fd == INVALID_SOCKET) {
#else
    if (ds->socket_fd < 0) {
#endif
        free(ds);
        return NULL;
    }

    // Enable broadcast
    int broadcast = 1;
    setsockopt(ds->socket_fd, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast, sizeof(broadcast));

    // Enable address reuse (for multiple instances on same machine)
    int reuse = 1;
    setsockopt(ds->socket_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));
#ifdef SO_REUSEPORT
    setsockopt(ds->socket_fd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse));
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
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(ds->socket_fd, FIONBIO, &mode);
#else
    int flags = fcntl(ds->socket_fd, F_GETFL, 0);
    fcntl(ds->socket_fd, F_SETFL, flags | O_NONBLOCK);
#endif

    return ds;
}

void discovery_destroy(DiscoveryService* ds) {
    if (!ds) return;

    if (ds->socket_fd >= 0) {
        close(ds->socket_fd);
    }

    free(ds);
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
    for (int i = 0; i < session->attribute_count && i < 64; i++) {
        offset += serialize_attribute(buf + offset, &session->attributes[i]);
    }

    // Send broadcast
    struct sockaddr_in dest = {0};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(ds->port);
    inet_pton(AF_INET, ds->broadcast_addr, &dest.sin_addr);

    sendto(ds->socket_fd, (const char*)buf, offset, 0, (struct sockaddr*)&dest, sizeof(dest));

    // If localhost mode, also send to loopback broadcast for Wine/Proton support
    if (ds->localhost_mode) {
        struct sockaddr_in lo = {0};
        lo.sin_family = AF_INET;
        lo.sin_port = htons(ds->port);
        // Use loopback broadcast (127.255.255.255) instead of unicast (127.0.0.1)
        // This allows packets to reach all sockets bound to the port on loopback
        inet_pton(AF_INET, "127.255.255.255", &lo.sin_addr);
        sendto(ds->socket_fd, (const char*)buf, offset, 0, (struct sockaddr*)&lo, sizeof(lo));
    }
}

void discovery_send_query(DiscoveryService* ds, const char* bucket_filter) {
    if (!ds) return;

    // Build query packet
    uint8_t* buf = ds->send_buffer;
    int offset = 0;

    // Header
    memcpy(buf + offset, DISCOVERY_MAGIC, 6); offset += 6;
    *(uint16_t*)(buf + offset) = htons(DISCOVERY_VERSION); offset += 2;
    buf[offset++] = MSG_QUERY;

    // Bucket filter (optional)
    if (bucket_filter) {
        memset(buf + offset, 0, 256);
        strncpy((char*)(buf + offset), bucket_filter, 256);
    } else {
        memset(buf + offset, 0, 256);
    }
    offset += 256;

    // Send broadcast
    struct sockaddr_in dest = {0};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(ds->port);
    inet_pton(AF_INET, ds->broadcast_addr, &dest.sin_addr);

    sendto(ds->socket_fd, (const char*)buf, offset, 0, (struct sockaddr*)&dest, sizeof(dest));

    // If localhost mode, also send to loopback broadcast for Wine/Proton support
    if (ds->localhost_mode) {
        struct sockaddr_in lo = {0};
        lo.sin_family = AF_INET;
        lo.sin_port = htons(ds->port);
        // Use loopback broadcast (127.255.255.255) instead of unicast (127.0.0.1)
        inet_pton(AF_INET, "127.255.255.255", &lo.sin_addr);
        sendto(ds->socket_fd, (const char*)buf, offset, 0, (struct sockaddr*)&lo, sizeof(lo));
    }
}

void discovery_poll(DiscoveryService* ds) {
    if (!ds) return;

    struct sockaddr_in from;
    socklen_t from_len = sizeof(from);

    while (true) {
#ifdef _WIN32
        int len = recvfrom(ds->socket_fd, (char*)ds->recv_buffer, MAX_PACKET_SIZE, 0,
                          (struct sockaddr*)&from, &from_len);
        if (len == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) break;
            break;
        }
#else
        ssize_t len = recvfrom(ds->socket_fd, ds->recv_buffer, MAX_PACKET_SIZE, 0,
                               (struct sockaddr*)&from, &from_len);
        if (len <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            break;
        }
#endif

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

                EOS_LOG_DEBUG("Received session announcement from %s: %s", source_ip, session.session_name);

                // Add to cache
                add_to_cache(ds, &session, source_ip);
            }
        } else if (msg_type == MSG_QUERY) {
            // Another instance is searching - set flag to trigger immediate broadcast
            ds->query_received = true;
            EOS_LOG_DEBUG("Received query from peer, will broadcast sessions immediately");
        }
    }
}

Session* discovery_get_sessions(DiscoveryService* ds, int* out_count) {
    if (!ds || !out_count) return NULL;

    *out_count = ds->cache_count;
    if (ds->cache_count == 0) return NULL;

    // Return pointer to first session in cache
    return &ds->cache[0].session;
}

void discovery_clear_sessions(DiscoveryService* ds) {
    if (!ds) return;
    ds->cache_count = 0;
}

void discovery_set_broadcast_addr(DiscoveryService* ds, const char* addr) {
    if (!ds || !addr) return;
    strncpy(ds->broadcast_addr, addr, sizeof(ds->broadcast_addr) - 1);
    ds->broadcast_addr[sizeof(ds->broadcast_addr) - 1] = '\0';
}

bool discovery_should_broadcast_now(DiscoveryService* ds) {
    if (!ds) return false;
    bool should = ds->query_received;
    ds->query_received = false;  // Clear flag after check
    return should;
}
