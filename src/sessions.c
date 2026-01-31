#include "eos/eos_sessions.h"
#include "eos/eos_sessions_types.h"
#include "internal/sessions_internal.h"
#include "internal/callbacks.h"
#include "internal/lan_discovery.h"
#include "internal/logging.h"
#include "lan_common.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

// Helper functions

void generate_session_id(char* buffer, size_t buffer_size) {
    static const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    size_t len = (buffer_size > 32) ? 32 : buffer_size - 1;

    srand((unsigned int)time(NULL) ^ (unsigned int)get_time_ms());

    for (size_t i = 0; i < len; i++) {
        buffer[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    buffer[len] = '\0';
}

Session* find_local_session_by_name(SessionsState* state, const char* name) {
    if (!state || !name) {
        return NULL;
    }

    for (int i = 0; i < state->local_session_count; i++) {
        if (state->local_sessions[i].valid &&
            strcmp(state->local_sessions[i].session_name, name) == 0) {
            return &state->local_sessions[i];
        }
    }

    return NULL;
}

Session* find_local_session_by_id(SessionsState* state, const char* id) {
    if (!state || !id) {
        return NULL;
    }

    for (int i = 0; i < state->local_session_count; i++) {
        if (state->local_sessions[i].valid &&
            strcmp(state->local_sessions[i].session_id, id) == 0) {
            return &state->local_sessions[i];
        }
    }

    return NULL;
}

// SessionsState lifecycle

SessionsState* sessions_create(PlatformState* platform) {
    SessionsState* state = calloc(1, sizeof(SessionsState));
    if (!state) {
        EOS_LOG_ERROR("Failed to allocate SessionsState");
        return NULL;
    }

    state->magic = 0x53455353;
    state->platform = platform;

    // Get config from platform (set from environment variables)
    uint16_t port = platform->lan_config.discovery_port;
    const char* broadcast_addr = platform->lan_config.broadcast_address;
    uint32_t interval = platform->lan_config.announcement_interval_ms;

    state->announce_interval_ms = interval;
    state->last_announce_time = 0;
    state->next_notification_id = 1;

    // Initialize LAN discovery service with configured port
    state->discovery = discovery_create(port);
    if (!state->discovery) {
        EOS_LOG_ERROR("Failed to create discovery service on port %u", port);
        free(state);
        return NULL;
    }

    // Set broadcast address from config
    discovery_set_broadcast_addr(state->discovery, broadcast_addr);

    EOS_LOG_INFO("SessionsState created with LAN discovery: port=%u, broadcast=%s, interval=%ums",
                 port, broadcast_addr, interval);
    return state;
}

void sessions_destroy(SessionsState* state) {
    if (!state || state->magic != 0x53455353) {
        return;
    }

    // Free notification lists
    NotificationEntry* entry = state->invite_received_notifications;
    while (entry) {
        NotificationEntry* next = entry->next;
        free(entry);
        entry = next;
    }

    entry = state->invite_accepted_notifications;
    while (entry) {
        NotificationEntry* next = entry->next;
        free(entry);
        entry = next;
    }

    entry = state->join_accepted_notifications;
    while (entry) {
        NotificationEntry* next = entry->next;
        free(entry);
        entry = next;
    }

    // Destroy LAN discovery service
    if (state->discovery) {
        discovery_destroy(state->discovery);
        state->discovery = NULL;
    }

    state->magic = 0;
    free(state);
}

void sessions_tick(SessionsState* state) {
    if (!state || state->magic != 0x53455353) {
        return;
    }

    if (!state->discovery) {
        return;
    }

    // Poll for incoming discovery packets (announcements from hosts, queries from clients)
    discovery_poll(state->discovery);

    // Check if a query was received - if so, broadcast immediately
    bool should_broadcast_now = discovery_should_broadcast_now(state->discovery);

    // Broadcast local sessions periodically (or immediately if query received)
    uint64_t now = get_time_ms();
    if (should_broadcast_now || (now - state->last_announce_time >= state->announce_interval_ms)) {
        // Broadcast all local sessions
        for (int i = 0; i < state->local_session_count; i++) {
            Session* s = &state->local_sessions[i];
            if (s->valid && s->state == EOS_OSS_Pending) {
                discovery_broadcast_session(state->discovery, s);
                EOS_LOG_DEBUG("Broadcasted session: %s", s->session_name);
            }
        }
        state->last_announce_time = now;
    }

    // Copy discovered sessions from discovery cache to our array
    int discovered_count = 0;
    Session* discovered = discovery_get_sessions(state->discovery, &discovered_count);

    // Copy up to MAX_DISCOVERED_SESSIONS
    state->discovered_session_count = (discovered_count > MAX_DISCOVERED_SESSIONS)
        ? MAX_DISCOVERED_SESSIONS : discovered_count;

    for (int i = 0; i < state->discovered_session_count; i++) {
        state->discovered_sessions[i] = discovered[i];
    }

    if (state->discovered_session_count > 0) {
        EOS_LOG_TRACE("Found %d discovered sessions in cache", state->discovered_session_count);
    }
}

// Session creation and modification

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CreateSessionModification(
    EOS_HSessions Handle,
    const EOS_Sessions_CreateSessionModificationOptions* Options,
    EOS_HSessionModification* OutSessionModificationHandle
) {
    SessionsState* state = (SessionsState*)Handle;

    if (!state || state->magic != 0x53455353) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONS_CREATESESSIONMODIFICATION_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!Options->SessionName || !Options->BucketId || !OutSessionModificationHandle) {
        return EOS_InvalidParameters;
    }

    if (Options->MaxPlayers == 0) {
        return EOS_InvalidParameters;
    }

    SessionModificationHandle* mod = calloc(1, sizeof(SessionModificationHandle));
    if (!mod) {
        return EOS_LimitExceeded;
    }

    mod->magic = 0x534D4F44;
    mod->is_new = true;

    // Initialize session from options
    strncpy(mod->session.session_name, Options->SessionName, sizeof(mod->session.session_name) - 1);
    strncpy(mod->session.bucket_id, Options->BucketId, sizeof(mod->session.bucket_id) - 1);
    mod->session.max_players = Options->MaxPlayers;
    mod->session.owner_id = Options->LocalUserId;
    mod->session.presence_enabled = (Options->bPresenceEnabled == EOS_TRUE);
    mod->session.sanctions_enabled = (Options->bSanctionsEnabled == EOS_TRUE);
    mod->session.state = EOS_OSS_Pending;
    mod->session.permission_level = EOS_OSPF_PublicAdvertised;  // Default
    mod->session.join_in_progress_allowed = true;  // Default
    mod->session.invites_allowed = true;  // Default

    // Generate session ID if not provided
    if (Options->SessionId && strlen(Options->SessionId) > 0) {
        strncpy(mod->session.session_id, Options->SessionId, sizeof(mod->session.session_id) - 1);
    } else {
        generate_session_id(mod->session.session_id, sizeof(mod->session.session_id));
    }

    *OutSessionModificationHandle = (EOS_HSessionModification)mod;
    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_UpdateSessionModification(
    EOS_HSessions Handle,
    const EOS_Sessions_UpdateSessionModificationOptions* Options,
    EOS_HSessionModification* OutSessionModificationHandle
) {
    SessionsState* state = (SessionsState*)Handle;

    if (!state || state->magic != 0x53455353) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONS_UPDATESESSIONMODIFICATION_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!Options->SessionName || !OutSessionModificationHandle) {
        return EOS_InvalidParameters;
    }

    // Find existing session
    Session* existing = find_local_session_by_name(state, Options->SessionName);
    if (!existing) {
        return EOS_NotFound;
    }

    SessionModificationHandle* mod = calloc(1, sizeof(SessionModificationHandle));
    if (!mod) {
        return EOS_LimitExceeded;
    }

    mod->magic = 0x534D4F44;
    mod->is_new = false;
    mod->session = *existing;  // Copy existing session data

    *OutSessionModificationHandle = (EOS_HSessionModification)mod;
    return EOS_Success;
}

EOS_DECLARE_FUNC(void) EOS_Sessions_UpdateSession(
    EOS_HSessions Handle,
    const EOS_Sessions_UpdateSessionOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnUpdateSessionCallback CompletionDelegate
) {
    SessionsState* state = (SessionsState*)Handle;

    EOS_Sessions_UpdateSessionCallbackInfo info = {0};
    info.ResultCode = EOS_InvalidParameters;
    info.ClientData = ClientData;

    if (!state || state->magic != 0x53455353) {
        goto queue_callback;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONS_UPDATESESSION_API_LATEST) {
        goto queue_callback;
    }

    SessionModificationHandle* mod = (SessionModificationHandle*)Options->SessionModificationHandle;
    if (!mod || mod->magic != 0x534D4F44) {
        goto queue_callback;
    }

    if (mod->is_new) {
        // Creating new session
        if (state->local_session_count >= MAX_LOCAL_SESSIONS) {
            info.ResultCode = EOS_LimitExceeded;
            goto queue_callback;
        }

        // Check if session name already exists
        if (find_local_session_by_name(state, mod->session.session_name) != NULL) {
            info.ResultCode = EOS_Sessions_SessionAlreadyExists;
            goto queue_callback;
        }

        // Add to local sessions
        Session* s = &state->local_sessions[state->local_session_count];
        *s = mod->session;
        s->valid = true;
        s->created_at = get_time_ms();
        s->last_updated = s->created_at;
        s->state = EOS_OSS_Pending;

        state->local_session_count++;

        info.ResultCode = EOS_Success;
        info.SessionName = s->session_name;
        info.SessionId = s->session_id;

        EOS_LOG_INFO("Session created: %s (ID: %s)", s->session_name, s->session_id);
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
    if (CompletionDelegate && state && state->platform && state->platform->callbacks) {
        callback_queue_push(state->platform->callbacks, (void*)CompletionDelegate, &info, sizeof(info));
    }
}

EOS_DECLARE_FUNC(void) EOS_Sessions_DestroySession(
    EOS_HSessions Handle,
    const EOS_Sessions_DestroySessionOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnDestroySessionCallback CompletionDelegate
) {
    SessionsState* state = (SessionsState*)Handle;

    EOS_Sessions_DestroySessionCallbackInfo info = {0};
    info.ResultCode = EOS_InvalidParameters;
    info.ClientData = ClientData;

    if (!state || state->magic != 0x53455353) {
        goto queue_callback;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONS_DESTROYSESSION_API_LATEST) {
        goto queue_callback;
    }

    if (!Options->SessionName) {
        goto queue_callback;
    }

    Session* session = find_local_session_by_name(state, Options->SessionName);
    if (!session) {
        info.ResultCode = EOS_NotFound;
        goto queue_callback;
    }

    if (session->state == EOS_OSS_Destroying) {
        info.ResultCode = EOS_AlreadyPending;
        goto queue_callback;
    }

    // Mark session as invalid (destroyed)
    session->state = EOS_OSS_Destroying;
    session->valid = false;

    // Remove from array by shifting
    int index = (int)(session - state->local_sessions);
    if (index >= 0 && index < state->local_session_count) {
        memmove(&state->local_sessions[index],
                &state->local_sessions[index + 1],
                sizeof(Session) * (state->local_session_count - index - 1));
        state->local_session_count--;
    }

    info.ResultCode = EOS_Success;

queue_callback:
    if (CompletionDelegate && state && state->platform && state->platform->callbacks) {
        callback_queue_push(state->platform->callbacks, (void*)CompletionDelegate, &info, sizeof(info));
    }
}

EOS_DECLARE_FUNC(void) EOS_Sessions_StartSession(
    EOS_HSessions Handle,
    const EOS_Sessions_StartSessionOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnStartSessionCallback CompletionDelegate
) {
    SessionsState* state = (SessionsState*)Handle;

    EOS_Sessions_StartSessionCallbackInfo info = {0};
    info.ResultCode = EOS_InvalidParameters;
    info.ClientData = ClientData;

    if (!state || state->magic != 0x53455353) {
        goto queue_callback;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONS_STARTSESSION_API_LATEST) {
        goto queue_callback;
    }

    if (!Options->SessionName) {
        goto queue_callback;
    }

    Session* session = find_local_session_by_name(state, Options->SessionName);
    if (!session) {
        info.ResultCode = EOS_NotFound;
        goto queue_callback;
    }

    // Transition to InProgress state
    session->state = EOS_OSS_InProgress;
    session->last_updated = get_time_ms();

    info.ResultCode = EOS_Success;

queue_callback:
    if (CompletionDelegate && state && state->platform && state->platform->callbacks) {
        callback_queue_push(state->platform->callbacks, (void*)CompletionDelegate, &info, sizeof(info));
    }
}

EOS_DECLARE_FUNC(void) EOS_Sessions_EndSession(
    EOS_HSessions Handle,
    const EOS_Sessions_EndSessionOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnEndSessionCallback CompletionDelegate
) {
    SessionsState* state = (SessionsState*)Handle;

    EOS_Sessions_EndSessionCallbackInfo info = {0};
    info.ResultCode = EOS_InvalidParameters;
    info.ClientData = ClientData;

    if (!state || state->magic != 0x53455353) {
        goto queue_callback;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONS_ENDSESSION_API_LATEST) {
        goto queue_callback;
    }

    if (!Options->SessionName) {
        goto queue_callback;
    }

    Session* session = find_local_session_by_name(state, Options->SessionName);
    if (!session) {
        info.ResultCode = EOS_NotFound;
        goto queue_callback;
    }

    // Transition to Ended state
    session->state = EOS_OSS_Ended;
    session->last_updated = get_time_ms();

    info.ResultCode = EOS_Success;

queue_callback:
    if (CompletionDelegate && state && state->platform && state->platform->callbacks) {
        callback_queue_push(state->platform->callbacks, (void*)CompletionDelegate, &info, sizeof(info));
    }
}

// Session joining

EOS_DECLARE_FUNC(void) EOS_Sessions_JoinSession(
    EOS_HSessions Handle,
    const EOS_Sessions_JoinSessionOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnJoinSessionCallback CompletionDelegate
) {
    SessionsState* state = (SessionsState*)Handle;

    EOS_Sessions_JoinSessionCallbackInfo info = {0};
    info.ResultCode = EOS_InvalidParameters;
    info.ClientData = ClientData;

    if (!state || state->magic != 0x53455353) {
        goto queue_callback;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONS_JOINSESSION_API_LATEST) {
        goto queue_callback;
    }

    if (!Options->SessionName || !Options->SessionHandle) {
        goto queue_callback;
    }

    // Check if session name already exists
    if (find_local_session_by_name(state, Options->SessionName) != NULL) {
        info.ResultCode = EOS_Sessions_SessionAlreadyExists;
        goto queue_callback;
    }

    // Get session details from handle
    SessionDetailsHandle* details = (SessionDetailsHandle*)Options->SessionHandle;
    if (!details || details->magic != 0x53445448) {
        info.ResultCode = EOS_InvalidParameters;
        goto queue_callback;
    }

    if (state->local_session_count >= MAX_LOCAL_SESSIONS) {
        info.ResultCode = EOS_LimitExceeded;
        goto queue_callback;
    }

    // Add session to local sessions
    Session* s = &state->local_sessions[state->local_session_count];
    *s = details->session;
    strncpy(s->session_name, Options->SessionName, sizeof(s->session_name) - 1);
    s->valid = true;
    s->created_at = get_time_ms();
    s->last_updated = s->created_at;
    s->presence_enabled = (Options->bPresenceEnabled == EOS_TRUE);

    state->local_session_count++;

    info.ResultCode = EOS_Success;

queue_callback:
    if (CompletionDelegate && state && state->platform && state->platform->callbacks) {
        callback_queue_push(state->platform->callbacks, (void*)CompletionDelegate, &info, sizeof(info));
    }
}

// Player registration

EOS_DECLARE_FUNC(void) EOS_Sessions_RegisterPlayers(
    EOS_HSessions Handle,
    const EOS_Sessions_RegisterPlayersOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnRegisterPlayersCallback CompletionDelegate
) {
    SessionsState* state = (SessionsState*)Handle;

    EOS_Sessions_RegisterPlayersCallbackInfo info = {0};
    info.ResultCode = EOS_InvalidParameters;
    info.ClientData = ClientData;

    if (!state || state->magic != 0x53455353) {
        goto queue_callback;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONS_REGISTERPLAYERS_API_LATEST) {
        goto queue_callback;
    }

    if (!Options->SessionName || !Options->PlayersToRegister) {
        goto queue_callback;
    }

    Session* session = find_local_session_by_name(state, Options->SessionName);
    if (!session) {
        info.ResultCode = EOS_NotFound;
        goto queue_callback;
    }

    // Register players
    EOS_ProductUserId* registered = calloc(Options->PlayersToRegisterCount, sizeof(EOS_ProductUserId));
    if (!registered) {
        info.ResultCode = EOS_LimitExceeded;
        goto queue_callback;
    }

    uint32_t registered_count = 0;

    for (uint32_t i = 0; i < Options->PlayersToRegisterCount; i++) {
        // Check if player already registered
        bool already_registered = false;
        for (int j = 0; j < session->registered_player_count; j++) {
            if (session->registered_players[j] == Options->PlayersToRegister[i]) {
                already_registered = true;
                break;
            }
        }

        if (!already_registered && session->registered_player_count < MAX_REGISTERED_PLAYERS) {
            session->registered_players[session->registered_player_count++] = Options->PlayersToRegister[i];
            registered[registered_count++] = Options->PlayersToRegister[i];
        }
    }

    session->last_updated = get_time_ms();

    info.ResultCode = EOS_Success;
    info.RegisteredPlayers = registered;
    info.RegisteredPlayersCount = registered_count;
    info.SanctionedPlayers = NULL;
    info.SanctionedPlayersCount = 0;

queue_callback:
    if (CompletionDelegate && state && state->platform && state->platform->callbacks) {
        callback_queue_push(state->platform->callbacks, (void*)CompletionDelegate, &info, sizeof(info));
    }

    if (info.RegisteredPlayers) {
        free(info.RegisteredPlayers);
    }
}

EOS_DECLARE_FUNC(void) EOS_Sessions_UnregisterPlayers(
    EOS_HSessions Handle,
    const EOS_Sessions_UnregisterPlayersOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnUnregisterPlayersCallback CompletionDelegate
) {
    SessionsState* state = (SessionsState*)Handle;

    EOS_Sessions_UnregisterPlayersCallbackInfo info = {0};
    info.ResultCode = EOS_InvalidParameters;
    info.ClientData = ClientData;

    if (!state || state->magic != 0x53455353) {
        goto queue_callback;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONS_UNREGISTERPLAYERS_API_LATEST) {
        goto queue_callback;
    }

    if (!Options->SessionName || !Options->PlayersToUnregister) {
        goto queue_callback;
    }

    Session* session = find_local_session_by_name(state, Options->SessionName);
    if (!session) {
        info.ResultCode = EOS_NotFound;
        goto queue_callback;
    }

    // Unregister players
    EOS_ProductUserId* unregistered = calloc(Options->PlayersToUnregisterCount, sizeof(EOS_ProductUserId));
    if (!unregistered) {
        info.ResultCode = EOS_LimitExceeded;
        goto queue_callback;
    }

    uint32_t unregistered_count = 0;

    for (uint32_t i = 0; i < Options->PlayersToUnregisterCount; i++) {
        // Find and remove player
        for (int j = 0; j < session->registered_player_count; j++) {
            if (session->registered_players[j] == Options->PlayersToUnregister[i]) {
                // Shift remaining players down
                memmove(&session->registered_players[j],
                        &session->registered_players[j + 1],
                        sizeof(EOS_ProductUserId) * (session->registered_player_count - j - 1));
                session->registered_player_count--;
                unregistered[unregistered_count++] = Options->PlayersToUnregister[i];
                break;
            }
        }
    }

    session->last_updated = get_time_ms();

    info.ResultCode = EOS_Success;
    info.UnregisteredPlayers = unregistered;
    info.UnregisteredPlayersCount = unregistered_count;

queue_callback:
    if (CompletionDelegate && state && state->platform && state->platform->callbacks) {
        callback_queue_push(state->platform->callbacks, (void*)CompletionDelegate, &info, sizeof(info));
    }

    if (info.UnregisteredPlayers) {
        free(info.UnregisteredPlayers);
    }
}

// Session search

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CreateSessionSearch(
    EOS_HSessions Handle,
    const EOS_Sessions_CreateSessionSearchOptions* Options,
    EOS_HSessionSearch* OutSessionSearchHandle
) {
    SessionsState* state = (SessionsState*)Handle;

    if (!state || state->magic != 0x53455353) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONS_CREATESESSIONSEARCH_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!OutSessionSearchHandle) {
        return EOS_InvalidParameters;
    }

    SessionSearchHandle* search = calloc(1, sizeof(SessionSearchHandle));
    if (!search) {
        return EOS_LimitExceeded;
    }

    search->magic = 0x53534348;
    search->sessions_state = state;
    search->max_results = (Options->MaxSearchResults > 0) ? Options->MaxSearchResults : 10;

    *OutSessionSearchHandle = (EOS_HSessionSearch)search;
    return EOS_Success;
}

// Active session handle

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CopyActiveSessionHandle(
    EOS_HSessions Handle,
    const EOS_Sessions_CopyActiveSessionHandleOptions* Options,
    EOS_HActiveSession* OutSessionHandle
) {
    SessionsState* state = (SessionsState*)Handle;

    if (!state || state->magic != 0x53455353) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONS_COPYACTIVESESSIONHANDLE_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!Options->SessionName || !OutSessionHandle) {
        return EOS_InvalidParameters;
    }

    Session* session = find_local_session_by_name(state, Options->SessionName);
    if (!session) {
        return EOS_NotFound;
    }

    ActiveSessionHandle* active = calloc(1, sizeof(ActiveSessionHandle));
    if (!active) {
        return EOS_LimitExceeded;
    }

    active->magic = 0x41435448;
    active->session = session;
    strncpy(active->session_name, session->session_name, sizeof(active->session_name) - 1);

    *OutSessionHandle = (EOS_HActiveSession)active;
    return EOS_Success;
}

// Invite stubs (not implemented for LAN)

EOS_DECLARE_FUNC(void) EOS_Sessions_SendInvite(
    EOS_HSessions Handle,
    const EOS_Sessions_SendInviteOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnSendInviteCallback CompletionDelegate
) {
    SessionsState* state = (SessionsState*)Handle;

    EOS_Sessions_SendInviteCallbackInfo info = {0};
    info.ResultCode = EOS_Success;  // Pretend it worked
    info.ClientData = ClientData;

    if (CompletionDelegate && state && state->platform && state->platform->callbacks) {
        callback_queue_push(state->platform->callbacks, (void*)CompletionDelegate, &info, sizeof(info));
    }
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RejectInvite(
    EOS_HSessions Handle,
    const EOS_Sessions_RejectInviteOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnRejectInviteCallback CompletionDelegate
) {
    SessionsState* state = (SessionsState*)Handle;

    EOS_Sessions_RejectInviteCallbackInfo info = {0};
    info.ResultCode = EOS_Success;
    info.ClientData = ClientData;

    if (CompletionDelegate && state && state->platform && state->platform->callbacks) {
        callback_queue_push(state->platform->callbacks, (void*)CompletionDelegate, &info, sizeof(info));
    }
}

EOS_DECLARE_FUNC(void) EOS_Sessions_QueryInvites(
    EOS_HSessions Handle,
    const EOS_Sessions_QueryInvitesOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnQueryInvitesCallback CompletionDelegate
) {
    SessionsState* state = (SessionsState*)Handle;

    EOS_Sessions_QueryInvitesCallbackInfo info = {0};
    info.ResultCode = EOS_Success;
    info.ClientData = ClientData;
    info.LocalUserId = Options ? Options->LocalUserId : NULL;

    if (CompletionDelegate && state && state->platform && state->platform->callbacks) {
        callback_queue_push(state->platform->callbacks, (void*)CompletionDelegate, &info, sizeof(info));
    }
}

EOS_DECLARE_FUNC(uint32_t) EOS_Sessions_GetInviteCount(
    EOS_HSessions Handle,
    const EOS_Sessions_GetInviteCountOptions* Options
) {
    (void)Handle;
    (void)Options;
    return 0;  // No invites in LAN mode
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_GetInviteIdByIndex(
    EOS_HSessions Handle,
    const EOS_Sessions_GetInviteIdByIndexOptions* Options,
    char* OutBuffer,
    int32_t* InOutBufferLength
) {
    (void)Handle;
    (void)Options;
    (void)OutBuffer;
    (void)InOutBufferLength;
    return EOS_NotFound;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CopySessionHandleByInviteId(
    EOS_HSessions Handle,
    const EOS_Sessions_CopySessionHandleByInviteIdOptions* Options,
    EOS_HSessionDetails* OutSessionHandle
) {
    (void)Handle;
    (void)Options;
    (void)OutSessionHandle;
    return EOS_NotFound;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CopySessionHandleByUiEventId(
    EOS_HSessions Handle,
    const EOS_Sessions_CopySessionHandleByUiEventIdOptions* Options,
    EOS_HSessionDetails* OutSessionHandle
) {
    (void)Handle;
    (void)Options;
    (void)OutSessionHandle;
    return EOS_NotFound;
}

// Notification management

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifySessionInviteReceived(
    EOS_HSessions Handle,
    const EOS_Sessions_AddNotifySessionInviteReceivedOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnSessionInviteReceivedCallback NotificationFn
) {
    SessionsState* state = (SessionsState*)Handle;

    if (!state || state->magic != 0x53455353 || !NotificationFn) {
        return EOS_INVALID_NOTIFICATIONID;
    }

    NotificationEntry* entry = calloc(1, sizeof(NotificationEntry));
    if (!entry) {
        return EOS_INVALID_NOTIFICATIONID;
    }

    entry->id = state->next_notification_id++;
    entry->callback = (void*)NotificationFn;
    entry->client_data = ClientData;
    entry->next = state->invite_received_notifications;
    state->invite_received_notifications = entry;

    return entry->id;
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifySessionInviteReceived(
    EOS_HSessions Handle,
    EOS_NotificationId InId
) {
    SessionsState* state = (SessionsState*)Handle;

    if (!state || state->magic != 0x53455353) {
        return;
    }

    NotificationEntry** curr = &state->invite_received_notifications;
    while (*curr) {
        if ((*curr)->id == InId) {
            NotificationEntry* to_remove = *curr;
            *curr = (*curr)->next;
            free(to_remove);
            return;
        }
        curr = &(*curr)->next;
    }
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifySessionInviteAccepted(
    EOS_HSessions Handle,
    const EOS_Sessions_AddNotifySessionInviteAcceptedOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnSessionInviteAcceptedCallback NotificationFn
) {
    SessionsState* state = (SessionsState*)Handle;

    if (!state || state->magic != 0x53455353 || !NotificationFn) {
        return EOS_INVALID_NOTIFICATIONID;
    }

    NotificationEntry* entry = calloc(1, sizeof(NotificationEntry));
    if (!entry) {
        return EOS_INVALID_NOTIFICATIONID;
    }

    entry->id = state->next_notification_id++;
    entry->callback = (void*)NotificationFn;
    entry->client_data = ClientData;
    entry->next = state->invite_accepted_notifications;
    state->invite_accepted_notifications = entry;

    return entry->id;
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifySessionInviteAccepted(
    EOS_HSessions Handle,
    EOS_NotificationId InId
) {
    SessionsState* state = (SessionsState*)Handle;

    if (!state || state->magic != 0x53455353) {
        return;
    }

    NotificationEntry** curr = &state->invite_accepted_notifications;
    while (*curr) {
        if ((*curr)->id == InId) {
            NotificationEntry* to_remove = *curr;
            *curr = (*curr)->next;
            free(to_remove);
            return;
        }
        curr = &(*curr)->next;
    }
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifyJoinSessionAccepted(
    EOS_HSessions Handle,
    const EOS_Sessions_AddNotifyJoinSessionAcceptedOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnJoinSessionAcceptedCallback NotificationFn
) {
    SessionsState* state = (SessionsState*)Handle;

    if (!state || state->magic != 0x53455353 || !NotificationFn) {
        return EOS_INVALID_NOTIFICATIONID;
    }

    NotificationEntry* entry = calloc(1, sizeof(NotificationEntry));
    if (!entry) {
        return EOS_INVALID_NOTIFICATIONID;
    }

    entry->id = state->next_notification_id++;
    entry->callback = (void*)NotificationFn;
    entry->client_data = ClientData;
    entry->next = state->join_accepted_notifications;
    state->join_accepted_notifications = entry;

    return entry->id;
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifyJoinSessionAccepted(
    EOS_HSessions Handle,
    EOS_NotificationId InId
) {
    SessionsState* state = (SessionsState*)Handle;

    if (!state || state->magic != 0x53455353) {
        return;
    }

    NotificationEntry** curr = &state->join_accepted_notifications;
    while (*curr) {
        if ((*curr)->id == InId) {
            NotificationEntry* to_remove = *curr;
            *curr = (*curr)->next;
            free(to_remove);
            return;
        }
        curr = &(*curr)->next;
    }
}
