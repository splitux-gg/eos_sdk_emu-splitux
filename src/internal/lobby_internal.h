#ifndef EOS_LAN_LOBBY_INTERNAL_H
#define EOS_LAN_LOBBY_INTERNAL_H

#include "eos/eos_lobby_types.h"
#include "eos/eos_common.h"
#include "platform_internal.h"
#include <stdbool.h>
#include <stdint.h>

// Forward declaration to avoid circular dependency (shared LAN discovery layer)
typedef struct DiscoveryService DiscoveryService;

// Constants (mirror sessions_internal.h; values from eos_lobby_types.h limits)
#define MAX_LOBBY_ATTRIBUTES 64           // EOS_LOBBYMODIFICATION_MAX_ATTRIBUTES
#define MAX_LOBBY_ATTRIBUTE_KEY_LEN 64    // EOS_LOBBYMODIFICATION_MAX_ATTRIBUTE_LENGTH
#define MAX_LOBBY_ATTRIBUTE_VALUE_LEN 256
#define MAX_LOBBY_MEMBERS 64
#define MAX_MEMBER_ATTRIBUTES 16
#define MAX_LOCAL_LOBBIES 8
#define MAX_DISCOVERED_LOBBIES 32
#define MAX_LOBBY_SEARCH_PARAMS 16
#define LOBBY_ID_LEN 64
#define LOBBY_BUCKET_ID_LEN 256
#define LOBBY_HOST_ADDRESS_LEN 64         // "IP:port" for P2P address book
#define LOBBY_OWNER_ID_STRING_LEN 33
#define LOBBY_SOURCE_IP_LEN 16

// A single lobby/member attribute (lobby uses EOS_EAttributeType, not the sessions enum)
typedef struct {
    char key[MAX_LOBBY_ATTRIBUTE_KEY_LEN + 1];
    EOS_EAttributeType type;
    union {
        EOS_Bool as_bool;
        int64_t as_int64;
        double as_double;
        char as_string[MAX_LOBBY_ATTRIBUTE_VALUE_LEN + 1];
    } value;
    EOS_ELobbyAttributeVisibility visibility;
} LobbyAttribute;

// A member of a lobby (lobbies carry members + per-member attributes; sessions do not)
typedef struct {
    EOS_ProductUserId member_id;
    char member_id_string[LOBBY_OWNER_ID_STRING_LEN];
    LobbyAttribute attributes[MAX_MEMBER_ATTRIBUTES];
    int attribute_count;
    bool valid;
} LobbyMember;

// Full lobby data structure
typedef struct {
    // Identity
    char lobby_id[LOBBY_ID_LEN + 1];

    // Owner
    EOS_ProductUserId owner_id;
    char owner_id_string[LOBBY_OWNER_ID_STRING_LEN];

    // Configuration
    char bucket_id[LOBBY_BUCKET_ID_LEN];
    char host_address[LOBBY_HOST_ADDRESS_LEN];  // "IP:port" handed to the P2P address book
    uint32_t max_members;
    EOS_ELobbyPermissionLevel permission_level;
    bool allow_invites;
    bool allow_host_migration;
    bool allow_join_by_id;
    bool presence_enabled;
    bool rtc_room_enabled;

    // Attributes (lobby-level)
    LobbyAttribute attributes[MAX_LOBBY_ATTRIBUTES];
    int attribute_count;

    // Members
    LobbyMember members[MAX_LOBBY_MEMBERS];
    int member_count;

    // Timing
    uint64_t created_at;
    uint64_t last_updated;

    // LAN tracking (for discovered lobbies)
    char source_ip[LOBBY_SOURCE_IP_LEN];
    uint64_t last_seen;

    bool valid;
} Lobby;

// Lobby modification handle (staged changes before UpdateLobby commits them)
typedef struct {
    uint32_t magic;  // 0x4C4D4F44 = "LMOD"
    EOS_ProductUserId local_user;
    char lobby_id[LOBBY_ID_LEN + 1];
    Lobby staged;          // staged lobby-level fields + attributes
    LobbyAttribute member_attributes[MAX_MEMBER_ATTRIBUTES];  // staged local-member attrs
    int member_attribute_count;
    bool is_new;           // true = CreateLobby path, false = UpdateLobbyModification path
} LobbyModificationHandle;

// Search parameter
typedef struct {
    char key[MAX_LOBBY_ATTRIBUTE_KEY_LEN + 1];
    EOS_EAttributeType type;
    union {
        EOS_Bool as_bool;
        int64_t as_int64;
        double as_double;
        char as_string[MAX_LOBBY_ATTRIBUTE_VALUE_LEN + 1];
    } value;
    EOS_EComparisonOp comparison;
} LobbySearchParameter;

// Lobby search handle
typedef struct {
    uint32_t magic;  // 0x4C534348 = "LSCH"
    LobbyState* lobby_state;

    // Search configuration
    char target_lobby_id[LOBBY_ID_LEN + 1];
    EOS_ProductUserId target_user_id;
    LobbySearchParameter params[MAX_LOBBY_SEARCH_PARAMS];
    int param_count;
    uint32_t max_results;

    // Results
    Lobby* results;
    int result_count;
    bool search_complete;
} LobbySearchHandle;

// Lobby details handle (snapshot of a lobby for the game to read)
typedef struct {
    uint32_t magic;  // 0x4C445448 = "LDTH"
    Lobby lobby;                 // copy of lobby data
    EOS_ProductUserId local_user;
} LobbyDetailsHandle;

// Notification callback entry (same shape as sessions)
typedef struct LobbyNotificationEntry {
    EOS_NotificationId id;
    void* callback;
    void* client_data;
    struct LobbyNotificationEntry* next;
} LobbyNotificationEntry;

// Main lobby state
typedef struct LobbyState {
    uint32_t magic;  // 0x4C4F4259 = "LOBY"
    PlatformState* platform;

    // Local lobbies (created or joined)
    Lobby local_lobbies[MAX_LOCAL_LOBBIES];
    int local_lobby_count;

    // Discovered lobbies from LAN
    Lobby discovered_lobbies[MAX_DISCOVERED_LOBBIES];
    int discovered_lobby_count;

    // LAN discovery service (separate announce stream from sessions)
    DiscoveryService* discovery;

    // Announcement timing
    uint64_t last_announce_time;
    uint32_t announce_interval_ms;  // Default: 2000

    // Notification handlers (these are the ones currently dead-stubbed)
    LobbyNotificationEntry* lobby_update_notifications;
    LobbyNotificationEntry* member_update_notifications;
    LobbyNotificationEntry* member_status_notifications;
    LobbyNotificationEntry* invite_received_notifications;
    LobbyNotificationEntry* invite_accepted_notifications;
    LobbyNotificationEntry* join_accepted_notifications;
    EOS_NotificationId next_notification_id;
} LobbyState;

// Creation/destruction/tick
LobbyState* lobby_create(PlatformState* platform);
void lobby_destroy(LobbyState* state);
void lobby_tick(LobbyState* state);

// Helpers
Lobby* find_local_lobby_by_id(LobbyState* state, const char* id);
Lobby* find_local_lobby_by_owner(LobbyState* state, EOS_ProductUserId owner);
LobbyMember* lobby_find_member(Lobby* lobby, EOS_ProductUserId member);
void generate_lobby_id(char* buffer, size_t buffer_size);

// Fire a member-status notification (JOINED/LEFT/KICKED/PROMOTED/...) to the game
void lobby_fire_member_status(LobbyState* state, const char* lobby_id,
                              EOS_ProductUserId target, EOS_ELobbyMemberStatus status);

#endif // EOS_LAN_LOBBY_INTERNAL_H
