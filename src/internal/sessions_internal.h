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
// Presence bridging: the host's real join-info string + data records ride the
// LAN announce so the joiner's EOS_Presence_GetJoinInfo/CopyPresence return
// exactly what the host published (games parse their own join-info format).
#define PRESENCE_JOININFO_LEN 256   // EOS_PRESENCE_DATA_MAX_VALUE_LENGTH + 1
// 24 (was 8): a presence-enabled lobby host advertises ~14 attributes
// (CUSTOMJOININFO, MapName, conn counts, GameMode, SESSIONTEMPLATENAME, OSSv1,
// ALLOWEDPLATFORM, BUILDCL, MATCHTIMEOUT, BuildUniqueId, ...). UE copies every
// presence record verbatim into FOnlineUserPresence.Status.Properties, and the
// game's own join code reads those keys to decide joinable — so truncating to 8
// dropped keys the game may gate on (platform/build/template). Beacon buffer is
// 64KB; 24 records (~7.7KB) fits, and prec_count is a uint8_t (<=255).
#define MAX_PRESENCE_RECORDS 24
#define PRESENCE_KEY_LEN 65         // EOS_PRESENCE_DATA_MAX_KEY_LENGTH + 1
#define PRESENCE_VALUE_LEN 256      // EOS_PRESENCE_DATA_MAX_VALUE_LENGTH + 1

typedef struct {
    char key[PRESENCE_KEY_LEN];
    char value[PRESENCE_VALUE_LEN];
} PresenceRecord;

// User beacon: announces a user's EXISTENCE + presence on the LAN, independent
// of any session. Needed because games (e.g. Satisfactory) build their join UI
// from friends+presence — a host running a non-EOS session (Steam/Goldberg)
// would otherwise never appear as a peer.
#define MAX_USER_BEACONS 32
#define PEER_DISPLAY_NAME_LEN 64

typedef struct UserBeacon {
    char epic_id[33];                       // 32-hex EpicAccountId string
    char puid[33];                          // 32-hex ProductUserId string
    char steam_id[24];                      // decimal Steam ID (EOSLAN_STEAM_ID), "" if unset
    char display_name[PEER_DISPLAY_NAME_LEN];
    char join_info[PRESENCE_JOININFO_LEN];
    PresenceRecord records[MAX_PRESENCE_RECORDS];
    int record_count;
    char source_ip[SOURCE_IP_LEN];
    uint64_t last_seen;
    bool valid;
} UserBeacon;

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

    // Owner's presence as published via EOS_Presence_SetPresence (host side:
    // filled from social_bridge local presence before broadcast; joiner side:
    // parsed from the announce packet).
    char join_info[PRESENCE_JOININFO_LEN];
    PresenceRecord presence_records[MAX_PRESENCE_RECORDS];
    int presence_record_count;

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
    Session session;  // Copy of session data (snapshot at creation time)
    // Back-pointer to the owning sessions state so CopyInfo can re-pull the
    // LATEST advertised session from the live discovery cache by session_id.
    // Without this the handle is frozen at search/Find time; a joiner that
    // searched while the host's session was still incomplete (e.g. host mid-load,
    // 17 of 27 attrs) would otherwise read stale attributes and fail to migrate.
    struct SessionsState* sessions_state;
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

// Implemented in social_bridge.c — the local user's presence as published via
// EOS_Presence_SetPresence. sessions.c stamps these onto the LAN announce so
// the joiner's GetJoinInfo/CopyPresence return the host's real data.
const char* social_bridge_local_join_info(void);
const PresenceRecord* social_bridge_local_records(int* out_count);

// Auto-derive the local user's published presence from a presence-enabled lobby
// (real EOS behavior). Called by lobby.c on create/update/join of a
// bPresenceEnabled lobby; no-op if the game sets presence itself. Lets games that
// never touch the Presence interface (e.g. StarRupture) still expose a joinable
// presence record + join-info to friends' CopyPresence/GetJoinInfo.
void social_bridge_set_presence_from_lobby(const char* join_info,
                                           const PresenceRecord* records, int record_count);
void social_bridge_clear_lobby_presence(void);

// Resolve an external (Epic) account id string -> the matching ProductUserId:
// the local user's own Connect id, or a discovered peer's (carried in its LAN
// beacon as epic_id+puid). Used by EOS_Connect_GetExternalAccountMapping so the
// game can resolve a joinable host's (and its own) identity during an EOS join.
EOS_ProductUserId social_bridge_resolve_puid(PlatformState* platform, const char* epic_id);
// Resolve a STEAM external account id -> ProductUserId (peer Steam ID relayed in
// its LAN beacon). Used by EOS_Connect_GetExternalAccountMapping for type=STEAM so
// Steam-friend-driven join UIs can attribute a discovered lobby to a friend.
EOS_ProductUserId social_bridge_resolve_puid_by_steam(PlatformState* platform, const char* steam_id);

// Reverse of the above: given a ProductUserId string, return the EpicAccountId
// string it belongs to (the local user, or a discovered peer carried in its LAN
// beacon), or NULL if unknown. Used by EOS_Connect_GetProductUserIdMapping so the
// game can turn a session owner's puid back into an Epic account during a join.
const char* social_bridge_resolve_epic_by_puid(PlatformState* platform, const char* puid);

#endif // EOS_LAN_SESSIONS_INTERNAL_H
