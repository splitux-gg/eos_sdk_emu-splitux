#include "lan_p2p.h"
#include "lan_common.h"
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

#define P2P_MAGIC "EOSP2P"
#define P2P_VERSION 0x01

#define MAX_P2P_PACKET 4096

struct P2PSocketManager {
#ifdef _WIN32
    SOCKET socket_fd;
#else
    int socket_fd;
#endif
    uint16_t port;
    char local_ip[16];

    uint8_t recv_buffer[MAX_P2P_PACKET];
    uint8_t send_buffer[MAX_P2P_PACKET];

    // Last received packet data (for returning to caller)
    uint8_t last_recv_data[MAX_P2P_PACKET];
    uint32_t last_recv_len;
};

P2PSocketManager* lan_p2p_create(uint16_t base_port) {
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

    P2PSocketManager* mgr = calloc(1, sizeof(P2PSocketManager));
    if (!mgr) return NULL;

    // Create UDP socket
    mgr->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
#ifdef _WIN32
    if (mgr->socket_fd == INVALID_SOCKET) {
#else
    if (mgr->socket_fd < 0) {
#endif
        free(mgr);
        return NULL;
    }

    // Enable address reuse
    int reuse = 1;
    setsockopt(mgr->socket_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

    // Try to bind to port (try several if busy)
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;

    bool bound = false;
    for (int i = 0; i < 10; i++) {
        mgr->port = base_port + i;
        addr.sin_port = htons(mgr->port);

        if (bind(mgr->socket_fd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
            bound = true;
            break;
        }
    }

    if (!bound) {
        close(mgr->socket_fd);
        free(mgr);
        return NULL;
    }

    // Set non-blocking
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(mgr->socket_fd, FIONBIO, &mode);
#else
    int flags = fcntl(mgr->socket_fd, F_GETFL, 0);
    fcntl(mgr->socket_fd, F_SETFL, flags | O_NONBLOCK);
#endif

    // Get local IP
    get_local_ip(mgr->local_ip, sizeof(mgr->local_ip));

    return mgr;
}

void lan_p2p_destroy(P2PSocketManager* mgr) {
    if (!mgr) return;

#ifdef _WIN32
    if (mgr->socket_fd != INVALID_SOCKET) {
#else
    if (mgr->socket_fd >= 0) {
#endif
        close(mgr->socket_fd);
    }

    free(mgr);
}

uint16_t lan_p2p_get_port(P2PSocketManager* mgr) {
    return mgr ? mgr->port : 0;
}

const char* lan_p2p_get_local_ip(P2PSocketManager* mgr) {
    return mgr ? mgr->local_ip : "127.0.0.1";
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

    // Header: magic + version + message_type
    memcpy(buf + offset, P2P_MAGIC, 6); offset += 6;
    buf[offset++] = P2P_VERSION;
    buf[offset++] = packet->message_type;

    // Sender ID (32 bytes)
    memset(buf + offset, 0, 32);
    if (packet->sender_id) {
        strncpy((char*)(buf + offset), packet->sender_id, 32);
    }
    offset += 32;

    // Socket name (32 bytes)
    memset(buf + offset, 0, 32);
    if (packet->socket_name) {
        strncpy((char*)(buf + offset), packet->socket_name, 32);
    }
    offset += 32;

    // Channel
    buf[offset++] = packet->channel;

    // Flags
    uint8_t flags = 0;
    if (packet->reliable) flags |= 0x01;
    buf[offset++] = flags;

    // Sequence number
    *(uint32_t*)(buf + offset) = htonl(packet->sequence); offset += 4;

    // Data length
    *(uint32_t*)(buf + offset) = htonl(packet->data_len); offset += 4;

    // Payload
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

#ifdef _WIN32
    int sent = sendto(mgr->socket_fd, (const char*)buf, offset, 0, (struct sockaddr*)&dest, sizeof(dest));
    return sent == offset;
#else
    ssize_t sent = sendto(mgr->socket_fd, buf, offset, 0, (struct sockaddr*)&dest, sizeof(dest));
    return sent == offset;
#endif
}

bool lan_p2p_recv(P2PSocketManager* mgr, P2PReceivedPacket* out) {
    if (!mgr || !out) return false;

    struct sockaddr_in from;
    socklen_t from_len = sizeof(from);

#ifdef _WIN32
    int len = recvfrom(mgr->socket_fd, (char*)mgr->recv_buffer, MAX_P2P_PACKET, 0,
                      (struct sockaddr*)&from, &from_len);
    if (len == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) return false;
        return false;
    }
#else
    ssize_t len = recvfrom(mgr->socket_fd, mgr->recv_buffer, MAX_P2P_PACKET, 0,
                           (struct sockaddr*)&from, &from_len);
    if (len <= 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return false;
        return false;
    }
#endif

    // Parse header
    if (len < 82) return false;  // Minimum header size
    if (memcmp(mgr->recv_buffer, P2P_MAGIC, 6) != 0) return false;
    if (mgr->recv_buffer[6] != P2P_VERSION) return false;

    uint8_t* buf = mgr->recv_buffer;
    int offset = 7;

    // Message type
    out->message_type = buf[offset++];

    // Sender ID
    memcpy(out->sender_id, buf + offset, 32);
    out->sender_id[32] = '\0';
    offset += 32;

    // Socket name
    memcpy(out->socket_name, buf + offset, 32);
    out->socket_name[32] = '\0';
    offset += 32;

    // Channel
    out->channel = buf[offset++];

    // Flags
    uint8_t flags = buf[offset++];
    out->reliable = (flags & 0x01) != 0;

    // Sequence number
    out->sequence = ntohl(*(uint32_t*)(buf + offset)); offset += 4;

    // Data length
    out->data_len = ntohl(*(uint32_t*)(buf + offset)); offset += 4;

    // Get sender address
    char ip[16];
    inet_ntop(AF_INET, &from.sin_addr, ip, sizeof(ip));
    snprintf(out->sender_addr, sizeof(out->sender_addr), "%s:%d", ip, ntohs(from.sin_port));

    // Copy payload
    if (out->data_len > 0 && offset + out->data_len <= len) {
        if (out->data_len > MAX_P2P_PACKET) {
            out->data_len = MAX_P2P_PACKET;
        }
        memcpy(mgr->last_recv_data, buf + offset, out->data_len);
        out->data = mgr->last_recv_data;
    } else {
        out->data = NULL;
        out->data_len = 0;
    }

    return true;
}
