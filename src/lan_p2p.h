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

// P2P message types
#define P2P_MSG_DATA 0x01
#define P2P_MSG_CONNECT 0x02
#define P2P_MSG_ACCEPT 0x03
#define P2P_MSG_CLOSE 0x04

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
