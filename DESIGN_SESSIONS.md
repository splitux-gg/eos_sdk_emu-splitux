# Sessions Layer Implementation Guide

## Scope
You are implementing the **Sessions Layer** - the core matchmaking system for LAN multiplayer.

**Your responsibility:**
- Session creation, modification, and destruction
- Session search and discovery over LAN
- Session joining
- Session state machine (Pending → Starting → InProgress → Ending → Ended)
- Player registration within sessions
- LAN broadcast of session announcements

**NOT your responsibility:**
- P2P packet delivery (that's the P2P module)
- Low-level network I/O (use the network module)
- Callback queue (use platform's callback system)

**This is the most complex module and the core of the LAN emulator.**

---

## File Locations

**You will create:**
- `src/sessions.c` - Main implementation
- `src/sessions_internal.h` - Internal structures
- `src/session_modification.c` - Session modification handle
- `src/session_search.c` - Session search handle
- `src/session_details.c` - Session details handle

**You will use:**
- `include/eos/eos_sessions.h` - Official EOS headers
- `include/eos/eos_sessions_types.h` - Type definitions
- `src/callbacks.h` - For queuing callbacks
- `src/platform_internal.h` - Platform state access
- `src/lan_discovery.h` - LAN discovery service

---

## API Functions to Implement

### Session Lifecycle

```c
// Create a session modification handle (used to configure a new session)
EOS_EResult EOS_Sessions_CreateSessionModification(
    EOS_HSessions Handle,
    const EOS_Sessions_CreateSessionModificationOptions* Options,
    EOS_HSessionModification* OutSessionModificationHandle
);

// Create a modification handle for an existing session
EOS_EResult EOS_Sessions_UpdateSessionModification(
    EOS_HSessions Handle,
    const EOS_Sessions_UpdateSessionModificationOptions* Options,
    EOS_HSessionModification* OutSessionModificationHandle
);

// Commit session changes (creates or updates session)
void EOS_Sessions_UpdateSession(
    EOS_HSessions Handle,
    const EOS_Sessions_UpdateSessionOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnUpdateSessionCallback CompletionDelegate
);

// Destroy a session
void EOS_Sessions_DestroySession(
    EOS_HSessions Handle,
    const EOS_Sessions_DestroySessionOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnDestroySessionCallback CompletionDelegate
);

// Start a session (marks as InProgress)
void EOS_Sessions_StartSession(
    EOS_HSessions Handle,
    const EOS_Sessions_StartSessionOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnStartSessionCallback CompletionDelegate
);

// End a session (marks as Ended)
void EOS_Sessions_EndSession(
    EOS_HSessions Handle,
    const EOS_Sessions_EndSessionOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnEndSessionCallback CompletionDelegate
);
```

### Session Modification Handle

```c
EOS_EResult EOS_SessionModification_SetBucketId(EOS_HSessionModification Handle, const EOS_SessionModification_SetBucketIdOptions* Options);
EOS_EResult EOS_SessionModification_SetHostAddress(EOS_HSessionModification Handle, const EOS_SessionModification_SetHostAddressOptions* Options);
EOS_EResult EOS_SessionModification_SetPermissionLevel(EOS_HSessionModification Handle, const EOS_SessionModification_SetPermissionLevelOptions* Options);
EOS_EResult EOS_SessionModification_SetJoinInProgressAllowed(EOS_HSessionModification Handle, const EOS_SessionModification_SetJoinInProgressAllowedOptions* Options);
EOS_EResult EOS_SessionModification_SetMaxPlayers(EOS_HSessionModification Handle, const EOS_SessionModification_SetMaxPlayersOptions* Options);
EOS_EResult EOS_SessionModification_SetInvitesAllowed(EOS_HSessionModification Handle, const EOS_SessionModification_SetInvitesAllowedOptions* Options);
EOS_EResult EOS_SessionModification_AddAttribute(EOS_HSessionModification Handle, const EOS_SessionModification_AddAttributeOptions* Options);
EOS_EResult EOS_SessionModification_RemoveAttribute(EOS_HSessionModification Handle, const EOS_SessionModification_RemoveAttributeOptions* Options);
void EOS_SessionModification_Release(EOS_HSessionModification SessionModificationHandle);
```

### Session Search

```c
// Create a search handle
EOS_EResult EOS_Sessions_CreateSessionSearch(
    EOS_HSessions Handle,
    const EOS_Sessions_CreateSessionSearchOptions* Options,
    EOS_HSessionSearch* OutSessionSearchHandle
);

// Configure search
EOS_EResult EOS_SessionSearch_SetSessionId(EOS_HSessionSearch Handle, const EOS_SessionSearch_SetSessionIdOptions* Options);
EOS_EResult EOS_SessionSearch_SetTargetUserId(EOS_HSessionSearch Handle, const EOS_SessionSearch_SetTargetUserIdOptions* Options);
EOS_EResult EOS_SessionSearch_SetParameter(EOS_HSessionSearch Handle, const EOS_SessionSearch_SetParameterOptions* Options);
EOS_EResult EOS_SessionSearch_RemoveParameter(EOS_HSessionSearch Handle, const EOS_SessionSearch_RemoveParameterOptions* Options);
EOS_EResult EOS_SessionSearch_SetMaxResults(EOS_HSessionSearch Handle, const EOS_SessionSearch_SetMaxResultsOptions* Options);

// Execute search
void EOS_SessionSearch_Find(
    EOS_HSessionSearch Handle,
    const EOS_SessionSearch_FindOptions* Options,
    void* ClientData,
    const EOS_SessionSearch_OnFindCallback CompletionDelegate
);

// Get results
uint32_t EOS_SessionSearch_GetSearchResultCount(EOS_HSessionSearch Handle, const EOS_SessionSearch_GetSearchResultCountOptions* Options);
EOS_EResult EOS_SessionSearch_CopySearchResultByIndex(EOS_HSessionSearch Handle, const EOS_SessionSearch_CopySearchResultByIndexOptions* Options, EOS_HSessionDetails* OutSessionHandle);
void EOS_SessionSearch_Release(EOS_HSessionSearch SessionSearchHandle);
```

### Session Join

```c
void EOS_Sessions_JoinSession(
    EOS_HSessions Handle,
    const EOS_Sessions_JoinSessionOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnJoinSessionCallback CompletionDelegate
);
```

### Session Details Handle

```c
EOS_EResult EOS_SessionDetails_CopyInfo(EOS_HSessionDetails Handle, const EOS_SessionDetails_CopyInfoOptions* Options, EOS_SessionDetails_Info** OutSessionInfo);
uint32_t EOS_SessionDetails_GetSessionAttributeCount(EOS_HSessionDetails Handle, const EOS_SessionDetails_GetSessionAttributeCountOptions* Options);
EOS_EResult EOS_SessionDetails_CopySessionAttributeByIndex(EOS_HSessionDetails Handle, const EOS_SessionDetails_CopySessionAttributeByIndexOptions* Options, EOS_SessionDetails_Attribute** OutSessionAttribute);
EOS_EResult EOS_SessionDetails_CopySessionAttributeByKey(EOS_HSessionDetails Handle, const EOS_SessionDetails_CopySessionAttributeByKeyOptions* Options, EOS_SessionDetails_Attribute** OutSessionAttribute);
void EOS_SessionDetails_Release(EOS_HSessionDetails SessionHandle);
void EOS_SessionDetails_Info_Release(EOS_SessionDetails_Info* SessionInfo);
void EOS_SessionDetails_Attribute_Release(EOS_SessionDetails_Attribute* SessionAttribute);
```

### Active Session Handle

```c
EOS_EResult EOS_Sessions_CopyActiveSessionHandle(EOS_HSessions Handle, const EOS_Sessions_CopyActiveSessionHandleOptions* Options, EOS_HActiveSession* OutSessionHandle);
EOS_EResult EOS_ActiveSession_CopyInfo(EOS_HActiveSession Handle, const EOS_ActiveSession_CopyInfoOptions* Options, EOS_ActiveSession_Info** OutActiveSessionInfo);
uint32_t EOS_ActiveSession_GetRegisteredPlayerCount(EOS_HActiveSession Handle, const EOS_ActiveSession_GetRegisteredPlayerCountOptions* Options);
EOS_ProductUserId EOS_ActiveSession_GetRegisteredPlayerByIndex(EOS_HActiveSession Handle, const EOS_ActiveSession_GetRegisteredPlayerByIndexOptions* Options);
void EOS_ActiveSession_Release(EOS_HActiveSession ActiveSessionHandle);
void EOS_ActiveSession_Info_Release(EOS_ActiveSession_Info* ActiveSessionInfo);
```

### Player Registration

```c
void EOS_Sessions_RegisterPlayers(
    EOS_HSessions Handle,
    const EOS_Sessions_RegisterPlayersOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnRegisterPlayersCallback CompletionDelegate
);

void EOS_Sessions_UnregisterPlayers(
    EOS_HSessions Handle,
    const EOS_Sessions_UnregisterPlayersOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnUnregisterPlayersCallback CompletionDelegate
);
```

### Invites (Stub implementations)

```c
void EOS_Sessions_SendInvite(...);      // Queue success callback
void EOS_Sessions_RejectInvite(...);    // Queue success callback
void EOS_Sessions_QueryInvites(...);    // Queue success callback, empty results
uint32_t EOS_Sessions_GetInviteCount(...);  // Return 0
EOS_EResult EOS_Sessions_GetInviteIdByIndex(...);  // Return EOS_NotFound
EOS_EResult EOS_Sessions_CopySessionHandleByInviteId(...);  // Return EOS_NotFound
```

### Notifications

```c
EOS_NotificationId EOS_Sessions_AddNotifySessionInviteReceived(...);
EOS_NotificationId EOS_Sessions_AddNotifySessionInviteAccepted(...);
EOS_NotificationId EOS_Sessions_AddNotifyJoinSessionAccepted(...);
void EOS_Sessions_RemoveNotifySessionInviteReceived(...);
void EOS_Sessions_RemoveNotifySessionInviteAccepted(...);
void EOS_Sessions_RemoveNotifyJoinSessionAccepted(...);
```

---

## Data Structures

### sessions_internal.h

```c
#ifndef EOS_LAN_SESSIONS_INTERNAL_H
#define EOS_LAN_SESSIONS_INTERNAL_H

#include "eos_sessions_types.h"
#include "platform_internal.h"
#include <stdbool.h>
#include <stdint.h>

#define MAX_SESSION_ATTRIBUTES 64
#define MAX_SESSION_ATTRIBUTE_KEY_LEN 64
#define MAX_SESSION_ATTRIBUTE_VALUE_LEN 256
#define MAX_REGISTERED_PLAYERS 64
#define MAX_LOCAL_SESSIONS 8
#define MAX_DISCOVERED_SESSIONS 32
#define MAX_SEARCH_PARAMS 16

// Session attribute
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

// Full session data
typedef struct {
    // Identity
    char session_id[65];
    char session_name[256];

    // Configuration
    char bucket_id[256];
    char host_address[64];  // "IP:port"
    uint32_t max_players;
    EOS_EOnlineSessionPermissionLevel permission_level;
    EOS_EOnlineSessionState state;
    bool join_in_progress_allowed;
    bool invites_allowed;
    bool presence_enabled;
    bool sanctions_enabled;

    // Owner
    EOS_ProductUserId owner_id;
    char owner_id_string[33];

    // Attributes
    SessionAttribute attributes[MAX_SESSION_ATTRIBUTES];
    int attribute_count;

    // Registered players
    EOS_ProductUserId registered_players[MAX_REGISTERED_PLAYERS];
    int registered_player_count;

    // Timing
    uint64_t created_at;
    uint64_t last_updated;

    // LAN tracking
    char source_ip[16];  // For discovered sessions
    uint64_t last_seen;  // For discovered sessions

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
    char target_session_id[65];
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
    char session_name[256];
} ActiveSessionHandle;

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

    // Announcement timing
    uint64_t last_announce_time;
    uint32_t announce_interval_ms;  // Default: 2000

    // Notification handlers
    // ... (similar to connect)
} SessionsState;

// Creation/destruction
SessionsState* sessions_create(PlatformState* platform);
void sessions_destroy(SessionsState* state);
void sessions_tick(SessionsState* state);

#endif // EOS_LAN_SESSIONS_INTERNAL_H
```

---

## LAN Discovery Protocol

### Session Announcement Packet

Broadcast every 2 seconds while session is active:

```
Offset  Size  Field
------  ----  -----
0       6     Magic "EOSLAN"
6       2     Version (0x0001)
8       1     Message Type (0x01 = ANNOUNCE)
9       64    Session ID (null-padded)
73      256   Session Name (null-padded)
329     256   Bucket ID (null-padded)
585     64    Host Address "IP:port" (null-padded)
649     32    Owner ID (null-padded)
681     4     Max Players (uint32 LE)
685     4     Current Players (uint32 LE)
689     1     State (EOS_EOnlineSessionState)
690     1     Flags (bit 0: join_in_progress, bit 1: public)
691     2     Attribute Count (uint16 LE)
693     N     Attributes (see below)
        4     CRC32 checksum

Each Attribute:
  64    Key (null-padded)
  1     Type (0=bool, 1=int64, 2=double, 3=string)
  1     Advertisement (0=don't, 1=advertise)
  256   Value (format depends on type)
```

### Session Query Packet

Sent when searching:

```
Offset  Size  Field
------  ----  -----
0       6     Magic "EOSLAN"
6       2     Version (0x0001)
8       1     Message Type (0x02 = QUERY)
9       32    Requester ID (null-padded)
41      256   Bucket ID Filter (null-padded, empty = all)
```

### Implementation

```c
void sessions_tick(SessionsState* state) {
    uint64_t now = get_time_ms();

    // Broadcast announcements for our sessions
    if (now - state->last_announce_time >= state->announce_interval_ms) {
        for (int i = 0; i < state->local_session_count; i++) {
            Session* s = &state->local_sessions[i];
            if (s->valid && s->state != EOS_OSS_Destroying && s->state != EOS_OSS_NoSession) {
                broadcast_session_announcement(state, s);
            }
        }
        state->last_announce_time = now;
    }

    // Process incoming announcements
    discovery_poll(state->platform->discovery);

    // Copy new discoveries to our list
    int discovered_count;
    Session* discovered = discovery_get_sessions(state->platform->discovery, &discovered_count);
    for (int i = 0; i < discovered_count; i++) {
        merge_discovered_session(state, &discovered[i]);
    }

    // Age out stale discoveries (not seen in 10 seconds)
    for (int i = 0; i < state->discovered_session_count; i++) {
        if (now - state->discovered_sessions[i].last_seen > 10000) {
            // Remove from list
            memmove(&state->discovered_sessions[i],
                    &state->discovered_sessions[i + 1],
                    sizeof(Session) * (state->discovered_session_count - i - 1));
            state->discovered_session_count--;
            i--;
        }
    }
}
```

---

## Key Implementation Details

### EOS_Sessions_CreateSessionModification

```c
EOS_EResult EOS_Sessions_CreateSessionModification(
    EOS_HSessions Handle,
    const EOS_Sessions_CreateSessionModificationOptions* Options,
    EOS_HSessionModification* OutSessionModificationHandle
) {
    SessionsState* state = (SessionsState*)Handle;
    if (!state || state->magic != 0x53455353) return EOS_InvalidParameters;
    if (!Options || !OutSessionModificationHandle) return EOS_InvalidParameters;
    if (!Options->SessionName || !Options->BucketId) return EOS_InvalidParameters;

    SessionModificationHandle* mod = calloc(1, sizeof(SessionModificationHandle));
    if (!mod) return EOS_OutOfMemory;

    mod->magic = 0x534D4F44;
    mod->is_new = true;

    // Initialize session from options
    strncpy(mod->session.session_name, Options->SessionName, sizeof(mod->session.session_name) - 1);
    strncpy(mod->session.bucket_id, Options->BucketId, sizeof(mod->session.bucket_id) - 1);
    mod->session.max_players = Options->MaxPlayers;
    mod->session.owner_id = Options->LocalUserId;
    mod->session.presence_enabled = Options->bPresenceEnabled;
    mod->session.sanctions_enabled = Options->bSanctionsEnabled;
    mod->session.state = EOS_OSS_Pending;

    // Generate session ID if not provided
    if (Options->SessionId && strlen(Options->SessionId) > 0) {
        strncpy(mod->session.session_id, Options->SessionId, 64);
    } else {
        generate_session_id(mod->session.session_id);
    }

    *OutSessionModificationHandle = (EOS_HSessionModification)mod;
    return EOS_Success;
}
```

### EOS_Sessions_UpdateSession

```c
void EOS_Sessions_UpdateSession(
    EOS_HSessions Handle,
    const EOS_Sessions_UpdateSessionOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnUpdateSessionCallback CompletionDelegate
) {
    SessionsState* state = (SessionsState*)Handle;
    SessionModificationHandle* mod = (SessionModificationHandle*)Options->SessionModificationHandle;

    EOS_Sessions_UpdateSessionCallbackInfo info = {0};
    info.ClientData = ClientData;

    if (!state || state->magic != 0x53455353 || !mod || mod->magic != 0x534D4F44) {
        info.ResultCode = EOS_InvalidParameters;
        goto queue_callback;
    }

    if (mod->is_new) {
        // Creating new session
        if (state->local_session_count >= MAX_LOCAL_SESSIONS) {
            info.ResultCode = EOS_LimitExceeded;
            goto queue_callback;
        }

        // Add to local sessions
        Session* s = &state->local_sessions[state->local_session_count];
        *s = mod->session;
        s->valid = true;
        s->created_at = get_time_ms();
        s->last_updated = s->created_at;

        // Generate host address if not set
        if (strlen(s->host_address) == 0) {
            get_local_ip_and_port(s->host_address, sizeof(s->host_address));
        }

        state->local_session_count++;

        info.ResultCode = EOS_Success;
        info.SessionName = s->session_name;
        info.SessionId = s->session_id;
    } else {
        // Updating existing session
        Session* existing = find_local_session_by_name(state, mod->session.session_name);
        if (!existing) {
            info.ResultCode = EOS_NotFound;
            goto queue_callback;
        }

        // Apply modifications
        *existing = mod->session;
        existing->last_updated = get_time_ms();

        info.ResultCode = EOS_Success;
        info.SessionName = existing->session_name;
        info.SessionId = existing->session_id;
    }

queue_callback:
    if (CompletionDelegate) {
        callback_queue_push(state->platform->callbacks, (void*)CompletionDelegate, &info, sizeof(info));
    }
}
```

### EOS_SessionSearch_Find

```c
void EOS_SessionSearch_Find(
    EOS_HSessionSearch Handle,
    const EOS_SessionSearch_FindOptions* Options,
    void* ClientData,
    const EOS_SessionSearch_OnFindCallback CompletionDelegate
) {
    SessionSearchHandle* search = (SessionSearchHandle*)Handle;

    EOS_SessionSearch_FindCallbackInfo info = {0};
    info.ClientData = ClientData;

    if (!search || search->magic != 0x53534348) {
        info.ResultCode = EOS_InvalidParameters;
        goto queue_callback;
    }

    SessionsState* state = search->sessions_state;

    // Send query broadcast to trigger fresh announcements
    discovery_send_query(state->platform->discovery, search->bucket_id_filter);

    // Wait a bit for responses (or use cached)
    // For now, use cached discoveries

    // Filter discovered sessions
    search->results = calloc(search->max_results, sizeof(Session));
    search->result_count = 0;

    for (int i = 0; i < state->discovered_session_count && search->result_count < search->max_results; i++) {
        Session* s = &state->discovered_sessions[i];

        // Check if matches search criteria
        if (search->target_session_id[0] != '\0') {
            if (strcmp(s->session_id, search->target_session_id) != 0) continue;
        }

        if (search->target_user_id != NULL) {
            // TODO: Check if user is in session
        }

        // Check parameter filters
        bool matches = true;
        for (int p = 0; p < search->param_count && matches; p++) {
            matches = session_matches_param(s, &search->params[p]);
        }

        if (matches) {
            search->results[search->result_count++] = *s;
        }
    }

    search->search_complete = true;
    info.ResultCode = (search->result_count > 0) ? EOS_Success : EOS_NotFound;

queue_callback:
    if (CompletionDelegate) {
        callback_queue_push(state->platform->callbacks, (void*)CompletionDelegate, &info, sizeof(info));
    }
}
```

---

## Testing Criteria

1. **Session creation:**
   ```c
   // Create session
   EOS_Sessions_CreateSessionModificationOptions opts = {...};
   opts.SessionName = "TestSession";
   opts.BucketId = "TestBucket";
   opts.MaxPlayers = 4;

   EOS_HSessionModification mod;
   assert(EOS_Sessions_CreateSessionModification(sessions, &opts, &mod) == EOS_Success);

   EOS_Sessions_UpdateSessionOptions update = {...};
   update.SessionModificationHandle = mod;
   EOS_Sessions_UpdateSession(sessions, &update, NULL, OnUpdate);
   EOS_Platform_Tick(platform);
   // Verify callback received with success
   ```

2. **Session discovery (dual instance):**
   ```bash
   # Terminal 1: Host
   ./mock-game --host --name "TestSession"

   # Terminal 2: Client (after a few seconds)
   ./mock-game --join
   # Should find "TestSession" in search results
   ```

3. **Session join:**
   ```c
   // After finding session
   EOS_Sessions_JoinSessionOptions join = {...};
   join.SessionHandle = found_session_handle;
   join.LocalUserId = local_user;
   EOS_Sessions_JoinSession(sessions, &join, NULL, OnJoin);
   EOS_Platform_Tick(platform);
   // Verify callback received with success
   ```

---

## Build Instructions

```makefile
$(BUILD_DIR)/sessions.o: src/sessions.c src/sessions_internal.h
	$(CC) $(CFLAGS) -I include -c $< -o $@

$(BUILD_DIR)/session_modification.o: src/session_modification.c
	$(CC) $(CFLAGS) -I include -c $< -o $@

$(BUILD_DIR)/session_search.o: src/session_search.c
	$(CC) $(CFLAGS) -I include -c $< -o $@

$(BUILD_DIR)/session_details.o: src/session_details.c
	$(CC) $(CFLAGS) -I include -c $< -o $@
```

Dependencies:
- Standard C library
- EOS SDK headers
- callbacks.h
- platform_internal.h
- lan_discovery.h (from network module)
