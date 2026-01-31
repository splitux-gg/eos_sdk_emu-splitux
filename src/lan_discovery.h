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
