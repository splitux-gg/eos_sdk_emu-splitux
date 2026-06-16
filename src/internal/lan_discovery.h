#ifndef EOS_LAN_DISCOVERY_H
#define EOS_LAN_DISCOVERY_H

#include <stdint.h>
#include <stdbool.h>

// Get Session definition
#include "sessions_internal.h"

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
 */
void discovery_broadcast_session(DiscoveryService* ds, const Session* session);

/**
 * Send a query to request session announcements.
 */
void discovery_send_query(DiscoveryService* ds, const char* bucket_filter);

/**
 * Poll for incoming announcements.
 */
void discovery_poll(DiscoveryService* ds);

/**
 * Get discovered sessions.
 */
Session* discovery_get_sessions(DiscoveryService* ds, int* out_count);

/**
 * Find a single discovered session by its session_id in the live cache, or NULL.
 * Used to re-pull the freshest advertised attributes for a session the game is
 * about to read/join, instead of a stale search-time snapshot.
 */
const Session* discovery_find_cached_session(DiscoveryService* ds, const char* session_id);

/**
 * Clear all discovered sessions.
 */
void discovery_clear(DiscoveryService* ds);

/**
 * Set broadcast address (default: 255.255.255.255).
 */
void discovery_set_broadcast_addr(DiscoveryService* ds, const char* addr);

/**
 * Broadcast a user beacon (user existence + presence, independent of sessions).
 */
void discovery_broadcast_user(DiscoveryService* ds, const UserBeacon* user);

/**
 * Get discovered user beacons.
 */
UserBeacon* discovery_get_users(DiscoveryService* ds, int* out_count);

/**
 * Check if we should broadcast immediately (a query was received).
 * Also clears the flag after checking.
 */
bool discovery_should_broadcast_now(DiscoveryService* ds);

#endif // EOS_LAN_DISCOVERY_H
