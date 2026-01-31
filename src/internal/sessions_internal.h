#ifndef EOS_LAN_SESSIONS_INTERNAL_H
#define EOS_LAN_SESSIONS_INTERNAL_H

#include "eos/eos_sessions_types.h"
#include "platform_internal.h"
#include <stdbool.h>
#include <stdint.h>

// Forward declaration to avoid circular dependency
typedef struct DiscoveryService DiscoveryService;

// Constants
#define MAX_SESSION_ATTRIBUTES 64
#define MAX_SESSION_ATTRIBUTE_KEY_LEN 64
#define MAX_SESSION_ATTRIBUTE_VALUE_LEN 256
#define MAX_REGISTERED_PLAYERS 64
#define MAX_LOCAL_SESSIONS 8
#define MAX_DISCOVERED_SESSIONS 32
#define MAX_SEARCH_PARAMS 16
#define SESSION_ID_LEN 64
#define SESSION_NAME_LEN 256
#define BUCKET_ID_LEN 256
#define HOST_ADDRESS_LEN 64
#define OWNER_ID_STRING_LEN 33
#define SOURCE_IP_LEN 16

// Session attribute structure
typedef struct {
    char key[MAX_SESSION_ATTRIBUTE_KEY_LEN + 1];
    EOS_ESessionAttributeType type;
    union {
        EOS_Bool as_bool;
        int64_t as_int64;
        double as_double;
        char as_string[MAX_SESSION_ATTRIBUTE_VALUE_LEN + 1];
    } value;
    EOS_ESessionAttributeAdvertisementType advertisement;
} SessionAttribute;

// Full session data structure
typedef struct {
    // Identity
    char session_id[SESSION_ID_LEN + 1];
    char session_name[SESSION_NAME_LEN];

    // Configuration
    char bucket_id[BUCKET_ID_LEN];
    char host_address[HOST_ADDRESS_LEN];  // "IP:port"
    uint32_t max_players;
    EOS_EOnlineSessionPermissionLevel permission_level;
    EOS_EOnlineSessionState state;
    bool join_in_progress_allowed;
    bool invites_allowed;
    bool presence_enabled;
    bool sanctions_enabled;

    // Owner
    EOS_ProductUserId owner_id;
    char owner_id_string[OWNER_ID_STRING_LEN];

    // Attributes
    SessionAttribute attributes[MAX_SESSION_ATTRIBUTES];
    int attribute_count;

    // Registered players
    EOS_ProductUserId registered_players[MAX_REGISTERED_PLAYERS];
    int registered_player_count;

    // Timing
    uint64_t created_at;
    uint64_t last_updated;

    // LAN tracking (for discovered sessions)
    char source_ip[SOURCE_IP_LEN];
    uint64_t last_seen;

    bool valid;
} Session;

// Session modification handle
typedef struct {
    uint32_t magic;  // 0x534D4F44 = "SMOD"
    Session session;
    bool is_new;  // true = creating, false = updating
} SessionModificationHandle;

// Search parameter
typedef struct {
    char key[MAX_SESSION_ATTRIBUTE_KEY_LEN + 1];
    EOS_ESessionAttributeType type;
    union {
        EOS_Bool as_bool;
        int64_t as_int64;
        double as_double;
        char as_string[MAX_SESSION_ATTRIBUTE_VALUE_LEN + 1];
    } value;
    EOS_EComparisonOp comparison;
} SearchParameter;

// Session search handle
typedef struct {
    uint32_t magic;  // 0x53534348 = "SSCH"
    SessionsState* sessions_state;

    // Search configuration
    char target_session_id[SESSION_ID_LEN + 1];
    EOS_ProductUserId target_user_id;
    SearchParameter params[MAX_SEARCH_PARAMS];
    int param_count;
    uint32_t max_results;

    // Results
    Session* results;
    int result_count;
    bool search_complete;
} SessionSearchHandle;

// Session details handle
typedef struct {
    uint32_t magic;  // 0x53445448 = "SDTH"
    Session session;  // Copy of session data
} SessionDetailsHandle;

// Active session handle
typedef struct {
    uint32_t magic;  // 0x41435448 = "ACTH"
    Session* session;  // Pointer to session in local_sessions
    char session_name[SESSION_NAME_LEN];
} ActiveSessionHandle;

// Notification callback entry
typedef struct NotificationEntry {
    EOS_NotificationId id;
    void* callback;
    void* client_data;
    struct NotificationEntry* next;
} NotificationEntry;

// Main sessions state
typedef struct SessionsState {
    uint32_t magic;  // 0x53455353 = "SESS"
    PlatformState* platform;

    // Local sessions (ones we created or joined)
    Session local_sessions[MAX_LOCAL_SESSIONS];
    int local_session_count;

    // Discovered sessions from LAN
    Session discovered_sessions[MAX_DISCOVERED_SESSIONS];
    int discovered_session_count;

    // LAN discovery service
    DiscoveryService* discovery;

    // Announcement timing
    uint64_t last_announce_time;
    uint32_t announce_interval_ms;  // Default: 2000

    // Notification handlers
    NotificationEntry* invite_received_notifications;
    NotificationEntry* invite_accepted_notifications;
    NotificationEntry* join_accepted_notifications;
    EOS_NotificationId next_notification_id;
} SessionsState;

// Creation/destruction
SessionsState* sessions_create(PlatformState* platform);
void sessions_destroy(SessionsState* state);
void sessions_tick(SessionsState* state);

// Helper functions
Session* find_local_session_by_name(SessionsState* state, const char* name);
Session* find_local_session_by_id(SessionsState* state, const char* id);
void generate_session_id(char* buffer, size_t buffer_size);

#endif // EOS_LAN_SESSIONS_INTERNAL_H
