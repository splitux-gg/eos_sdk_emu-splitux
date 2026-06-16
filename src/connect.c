#include "internal/connect_internal.h"
#include "internal/auth_internal.h"
#include "internal/logging.h"
#include "internal/callbacks.h"
#include "internal/sessions_internal.h"  // discovered_sessions for external-account mapping
#include "eos/eos_connect.h"
#include "eos/eos_connect_types.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>  // SEH (__try/__except, EXCEPTION_EXECUTE_HANDLER) for IsValid
#endif

// Magic number for validation
#define CONNECT_MAGIC 0x434F4E4E  // "CONN"
#define PUID_MAGIC 0x50554944     // "PUID"

// ============================================================================
// Internal Helper Functions
// ============================================================================

// Deterministic identity helpers (defined in auth.c). Same string -> same id
// across every process of an instance and across platforms.
extern uint64_t eosdet_seed(const char* salt, const char* s);
extern void eosdet_fill_hex(char* out, int len, uint64_t seed, const char* digits16);

static void connect_init_instance_id(ConnectState* state) {
    // Deterministic per-instance id (16 hex) derived from the stable
    // EOSLAN_USERNAME, so EVERY process of this instance (bootstrap launcher +
    // shipping game) mints the SAME ProductUserId. Was srand(time) -> a random
    // id per process, so the two processes could mint different puids for one
    // identity, and the host's session owner wouldn't match the friend the
    // joiner resolved (UserKeyHandle:-1 / E011).
    const char* user = getenv("EOSLAN_USERNAME");
    if (!user || !user[0]) user = "LAN_Player";
    eosdet_fill_hex(state->instance_id, 16, eosdet_seed("puid:", user), "0123456789ABCDEF");

    EOS_LOG_DEBUG("Generated instance ID: %s", state->instance_id);
}

EOS_ProductUserId connect_generate_user_id(ConnectState* state, int user_index) {
    if (!state || user_index < 0 || user_index >= MAX_LOCAL_USERS) {
        return NULL;
    }

    EOS_ProductUserIdDetails* id = calloc(1, sizeof(EOS_ProductUserIdDetails));
    if (!id) {
        EOS_LOG_ERROR("Failed to allocate ProductUserId");
        return NULL;
    }

    id->magic = PUID_MAGIC;

    // Deterministic, instance-stable ProductUserId: {instance_id:16hex}{zeros:16}.
    // A single game instance is ONE identity, so every Connect_Login (boot login
    // with no creds + the join-time token login) MUST resolve to the same puid,
    // and it must match what we advertise in the LAN beacon and stamp as the
    // session owner. Previously this mixed in user_index + a random part, so the
    // two logins minted two different puids for one account -> the game's
    // join-time user resolution couldn't match the identity (UserKeyHandle:-1)
    // and aborted the EOS join with E011.
    (void)user_index;
    snprintf(id->id_string, sizeof(id->id_string),
             "%s0000000000000000",
             state->instance_id);

    EOS_LOG_DEBUG("Generated ProductUserId: %s", id->id_string);
    return (EOS_ProductUserId)id;
}

bool connect_validate_user_id(EOS_ProductUserId id) {
    if (!id) return false;

    EOS_ProductUserIdDetails* details = (EOS_ProductUserIdDetails*)id;
    if (details->magic != PUID_MAGIC) return false;

    // Check string is valid hex, correct length
    size_t len = strlen(details->id_string);
    if (len != PRODUCT_USER_ID_LENGTH) return false;

    for (size_t i = 0; i < PRODUCT_USER_ID_LENGTH; i++) {
        char c = details->id_string[i];
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
            return false;
        }
    }

    return true;
}

LocalUser* connect_find_user_by_id(ConnectState* state, EOS_ProductUserId id) {
    if (!state || !id) return NULL;

    EOS_ProductUserIdDetails* search_id = (EOS_ProductUserIdDetails*)id;

    for (int i = 0; i < MAX_LOCAL_USERS; i++) {
        if (state->users[i].in_use &&
            strcmp(state->users[i].user_id.id_string, search_id->id_string) == 0) {
            return &state->users[i];
        }
    }

    return NULL;
}

void connect_fire_login_status_notifications(ConnectState* state, EOS_ProductUserId user_id,
                                              EOS_ELoginStatus prev_status, EOS_ELoginStatus curr_status) {
    if (!state) return;

    // For now, call callbacks directly (synchronously)
    // In a full implementation, these would be queued via the callback system
    for (int i = 0; i < state->login_notification_count; i++) {
        if (state->login_notifications[i].active && state->login_notifications[i].callback) {
            EOS_Connect_LoginStatusChangedCallbackInfo notif = {0};
            notif.ClientData = state->login_notifications[i].client_data;
            notif.LocalUserId = user_id;
            notif.PreviousStatus = prev_status;
            notif.CurrentStatus = curr_status;

            state->login_notifications[i].callback(&notif);
        }
    }
}

// ============================================================================
// Creation/Destruction
// ============================================================================

ConnectState* connect_create(PlatformState* platform) {
    if (!platform) {
        EOS_LOG_ERROR("Cannot create ConnectState: NULL platform");
        return NULL;
    }

    ConnectState* state = calloc(1, sizeof(ConnectState));
    if (!state) {
        EOS_LOG_ERROR("Failed to allocate ConnectState");
        return NULL;
    }

    state->magic = CONNECT_MAGIC;
    state->platform = platform;
    state->user_count = 0;
    state->login_notification_count = 0;
    state->auth_expiration_notification_count = 0;
    state->next_notification_id = 1;

    // Initialize all users as not in use
    for (int i = 0; i < MAX_LOCAL_USERS; i++) {
        state->users[i].in_use = false;
        state->users[i].status = EOS_LS_NotLoggedIn;
        state->users[i].user_id.magic = PUID_MAGIC;
    }

    // Initialize all notifications as inactive
    for (int i = 0; i < MAX_LOGIN_NOTIFICATIONS; i++) {
        state->login_notifications[i].active = false;
    }
    for (int i = 0; i < MAX_AUTH_EXPIRATION_NOTIFICATIONS; i++) {
        state->auth_expiration_notifications[i].active = false;
    }

    connect_init_instance_id(state);

    EOS_LOG_INFO("ConnectState created");
    return state;
}

void connect_destroy(ConnectState* state) {
    if (!state) return;

    // Free any allocated user IDs (none in current implementation as we store inline)

    free(state);
    EOS_LOG_INFO("ConnectState destroyed");
}

// ============================================================================
// Core Authentication Functions
// ============================================================================

EOS_DECLARE_FUNC(void) EOS_Connect_Login(
    EOS_HConnect Handle,
    const EOS_Connect_LoginOptions* Options,
    void* ClientData,
    const EOS_Connect_OnLoginCallback CompletionDelegate
) {
    EOS_LOG_INFO("Connect_Login CALLED!");

    ConnectState* state = (ConnectState*)Handle;
    if (!state || state->magic != CONNECT_MAGIC) {
        EOS_LOG_ERROR("Connect_Login: Invalid Connect handle");
        return;
    }

    if (!Options) {
        EOS_LOG_ERROR("Connect_Login: Invalid LoginOptions: NULL");
        if (CompletionDelegate) {
            EOS_Connect_LoginCallbackInfo info = {0};
            info.ResultCode = EOS_InvalidParameters;
            info.ClientData = ClientData;
            info.LocalUserId = NULL;
            CompletionDelegate(&info);
        }
        return;
    }

    // Accept any API version - games may use older versions
    EOS_LOG_INFO("Connect_Login called with ApiVersion=%d (latest=%d)",
                  Options->ApiVersion, EOS_CONNECT_LOGIN_API_LATEST);

    // Log credential type
    if (Options->Credentials) {
        EOS_LOG_INFO(">>> Connect_Login credentials: Type=%d, Token=%s",
                     (int)Options->Credentials->Type,
                     Options->Credentials->Token ? "(present)" : "(null)");

        // For Steam auth (Type=18), ensure auth is set up as if Auth_Login completed
        // This makes EpicAccountId available immediately when game queries it
        if (Options->Credentials->Type == 18) {  // EOS_ECT_STEAM_SESSION_TICKET
            extern AuthState g_auth_state;
            if (!g_auth_state.logged_in) {
                extern void auth_auto_login(void);
                auth_auto_login();
                EOS_LOG_INFO(">>> Steam auth: Performed auth_auto_login for Steam credentials");
            }
        }
    } else {
        EOS_LOG_INFO(">>> Connect_Login credentials: NULL (no credentials provided)");
    }

    // Find free user slot
    int slot = -1;
    for (int i = 0; i < MAX_LOCAL_USERS; i++) {
        if (!state->users[i].in_use) {
            slot = i;
            break;
        }
    }

    // A game logs into Connect TWICE (boot + join-time token). Each call grabs a
    // NEW slot here, so after two logins we report two logged-in users that share
    // the deterministic puid.
    {
        int occupied = 0;
        for (int i = 0; i < MAX_LOCAL_USERS; i++) if (state->users[i].in_use) occupied++;
    }

    EOS_Connect_LoginCallbackInfo info = {0};
    info.ClientData = ClientData;

    if (slot < 0) {
        // No free slots
        EOS_LOG_WARN("Login failed: maximum local users (%d) reached", MAX_LOCAL_USERS);
        info.ResultCode = EOS_LimitExceeded;
        info.LocalUserId = NULL;
    } else {
        // Generate user ID
        EOS_ProductUserId user_id = connect_generate_user_id(state, slot);
        if (!user_id) {
            EOS_LOG_ERROR("Failed to generate user ID");
            info.ResultCode = EOS_UnexpectedError;
            info.LocalUserId = NULL;
        } else {
            // Store user state
            state->users[slot].user_id = *(EOS_ProductUserIdDetails*)user_id;
            state->users[slot].status = EOS_LS_LoggedIn;
            state->users[slot].in_use = true;
            state->user_count++;

            // Free the temporary allocation
            free(user_id);

            info.ResultCode = EOS_Success;
            info.LocalUserId = (EOS_ProductUserId)&state->users[slot].user_id;

            EOS_LOG_INFO("User logged in successfully (slot %d, total users: %d)", slot, state->user_count);

            // Fire login status notifications
            connect_fire_login_status_notifications(state, info.LocalUserId,
                                                   EOS_LS_NotLoggedIn, EOS_LS_LoggedIn);
        }
    }

    // For Steam auth (Type=18), mark that Steam login completed
    // This flag will trigger notification firing when game registers for LoginStatusChanged
    if (Options->Credentials && Options->Credentials->Type == 18 && info.ResultCode == EOS_Success) {
        EOS_LOG_INFO(">>> STEAM AUTH COMPLETED <<<");
        EOS_LOG_INFO("    Setting steam_login_completed = true");
        EOS_LOG_INFO("    g_auth_state.account_id = %s (ptr=%p)",
                     g_auth_state.account_id.id_string, (void*)&g_auth_state.account_id);
        g_auth_state.steam_login_completed = true;

        // Fire stored Auth notifications now
        EOS_LOG_INFO("    Firing auth notifications...");
        auth_fire_login_notifications(EOS_LS_NotLoggedIn, EOS_LS_LoggedIn);
        EOS_LOG_INFO("    Auth notifications fired");
    }

    // Defer completion to EOS_Platform_Tick. EOS never fires completion callbacks
    // synchronously inside the API call — Palworld's EOS subsystem registers the op
    // and waits for the callback on a later tick, so a synchronous call is dropped
    // and the game reports "EOS login Timeout". Mirror the sessions/lobby pattern.
    if (CompletionDelegate && state->platform && state->platform->callbacks) {
        callback_queue_push(state->platform->callbacks, (void*)CompletionDelegate, &info, sizeof(info));
    } else if (CompletionDelegate) {
        CompletionDelegate(&info);  // no queue available: last resort
    }
}

EOS_DECLARE_FUNC(void) EOS_Connect_Logout(
    EOS_HConnect Handle,
    const EOS_Connect_LogoutOptions* Options,
    void* ClientData,
    const EOS_Connect_OnLogoutCallback CompletionDelegate
) {
    EOS_LOG_API_ENTER();

    ConnectState* state = (ConnectState*)Handle;
    if (!state || state->magic != CONNECT_MAGIC) {
        EOS_LOG_ERROR("Invalid Connect handle");
        return;
    }

    if (!Options || Options->ApiVersion != EOS_CONNECT_LOGOUT_API_LATEST) {
        EOS_LOG_ERROR("Invalid LogoutOptions");
        if (CompletionDelegate) {
            EOS_Connect_LogoutCallbackInfo info = {0};
            info.ResultCode = EOS_InvalidParameters;
            info.ClientData = ClientData;
            info.LocalUserId = NULL;
            CompletionDelegate(&info);
        }
        return;
    }

    EOS_Connect_LogoutCallbackInfo info = {0};
    info.ClientData = ClientData;
    info.LocalUserId = Options->LocalUserId;

    LocalUser* user = connect_find_user_by_id(state, Options->LocalUserId);
    if (!user) {
        EOS_LOG_WARN("Logout failed: user not found");
        info.ResultCode = EOS_InvalidUser;
    } else {
        EOS_ELoginStatus prev_status = user->status;

        // Mark user as logged out
        user->status = EOS_LS_NotLoggedIn;
        user->in_use = false;
        state->user_count--;

        info.ResultCode = EOS_Success;

        EOS_LOG_INFO("User logged out successfully (total users: %d)", state->user_count);

        // Fire login status notifications
        connect_fire_login_status_notifications(state, Options->LocalUserId,
                                               prev_status, EOS_LS_NotLoggedIn);
    }

    // Queue/call completion callback
    if (CompletionDelegate) {
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_Connect_CreateUser(
    EOS_HConnect Handle,
    const EOS_Connect_CreateUserOptions* Options,
    void* ClientData,
    const EOS_Connect_OnCreateUserCallback CompletionDelegate
) {
    EOS_LOG_API_ENTER();

    // For LAN emulator, CreateUser is the same as Login
    // We ignore the continuance token and just create a new user

    ConnectState* state = (ConnectState*)Handle;
    if (!state || state->magic != CONNECT_MAGIC) {
        EOS_LOG_ERROR("Invalid Connect handle");
        return;
    }

    if (!Options || Options->ApiVersion != EOS_CONNECT_CREATEUSER_API_LATEST) {
        EOS_LOG_ERROR("Invalid CreateUserOptions");
        if (CompletionDelegate) {
            EOS_Connect_CreateUserCallbackInfo info = {0};
            info.ResultCode = EOS_InvalidParameters;
            info.ClientData = ClientData;
            info.LocalUserId = NULL;
            CompletionDelegate(&info);
        }
        return;
    }

    // Find free user slot
    int slot = -1;
    for (int i = 0; i < MAX_LOCAL_USERS; i++) {
        if (!state->users[i].in_use) {
            slot = i;
            break;
        }
    }

    EOS_Connect_CreateUserCallbackInfo info = {0};
    info.ClientData = ClientData;

    if (slot < 0) {
        EOS_LOG_WARN("CreateUser failed: maximum local users reached");
        info.ResultCode = EOS_LimitExceeded;
        info.LocalUserId = NULL;
    } else {
        // Generate user ID
        EOS_ProductUserId user_id = connect_generate_user_id(state, slot);
        if (!user_id) {
            EOS_LOG_ERROR("Failed to generate user ID");
            info.ResultCode = EOS_UnexpectedError;
            info.LocalUserId = NULL;
        } else {
            // Store user state
            state->users[slot].user_id = *(EOS_ProductUserIdDetails*)user_id;
            state->users[slot].status = EOS_LS_LoggedIn;
            state->users[slot].in_use = true;
            state->user_count++;

            // Free the temporary allocation
            free(user_id);

            info.ResultCode = EOS_Success;
            info.LocalUserId = (EOS_ProductUserId)&state->users[slot].user_id;

            EOS_LOG_INFO("User created successfully (slot %d)", slot);

            // Fire login status notifications
            connect_fire_login_status_notifications(state, info.LocalUserId,
                                                   EOS_LS_NotLoggedIn, EOS_LS_LoggedIn);
        }
    }

    // Queue/call completion callback
    if (CompletionDelegate) {
        CompletionDelegate(&info);
    }
}

// ============================================================================
// User Query Functions
// ============================================================================

EOS_DECLARE_FUNC(int32_t) EOS_Connect_GetLoggedInUsersCount(EOS_HConnect Handle) {
    ConnectState* state = (ConnectState*)Handle;
    if (!state || state->magic != CONNECT_MAGIC) {
        EOS_LOG_ERROR("Connect_GetLoggedInUsersCount: Invalid Connect handle");
        return 0;
    }

    EOS_LOG_INFO("Connect_GetLoggedInUsersCount called - returning %d", state->user_count);
    return state->user_count;
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_Connect_GetLoggedInUserByIndex(EOS_HConnect Handle, int32_t Index) {
    EOS_LOG_API_ENTER();

    ConnectState* state = (ConnectState*)Handle;
    if (!state || state->magic != CONNECT_MAGIC) {
        EOS_LOG_ERROR("Invalid Connect handle");
        return NULL;
    }

    if (Index < 0 || Index >= state->user_count) {
        EOS_LOG_WARN("GetLoggedInUserByIndex: index %d out of range (count: %d)", Index, state->user_count);
        return NULL;
    }

    // Find the Nth logged-in user
    int count = 0;
    for (int i = 0; i < MAX_LOCAL_USERS; i++) {
        if (state->users[i].in_use && state->users[i].status == EOS_LS_LoggedIn) {
            if (count == Index) {
                return (EOS_ProductUserId)&state->users[i].user_id;
            }
            count++;
        }
    }

    return NULL;
}

EOS_DECLARE_FUNC(EOS_ELoginStatus) EOS_Connect_GetLoginStatus(EOS_HConnect Handle, EOS_ProductUserId LocalUserId) {
    EOS_LOG_API_ENTER();

    ConnectState* state = (ConnectState*)Handle;
    if (!state || state->magic != CONNECT_MAGIC) {
        EOS_LOG_ERROR("Invalid Connect handle");
        return EOS_LS_NotLoggedIn;
    }

    LocalUser* user = connect_find_user_by_id(state, LocalUserId);
    if (!user) {
        return EOS_LS_NotLoggedIn;
    }

    return user->status;
}

// ============================================================================
// Product User ID Functions
// ============================================================================

EOS_DECLARE_FUNC(EOS_Bool) EOS_ProductUserId_IsValid(EOS_ProductUserId AccountId) {
    if (!AccountId) return EOS_FALSE;

    // Games hand IsValid stale/garbage ids; a validity *probe* must never crash.
    // Guard the dereference with SEH so a bad pointer returns EOS_FALSE instead
    // of faulting (the joiner crashed here reading 0x...ffffffff).
    EOS_Bool result = EOS_FALSE;
#ifdef _WIN32
    __try {
#endif
        EOS_ProductUserIdDetails* id = (EOS_ProductUserIdDetails*)AccountId;
        if (id->magic == PUID_MAGIC && strlen(id->id_string) == PRODUCT_USER_ID_LENGTH) {
            result = EOS_TRUE;
            for (size_t i = 0; i < PRODUCT_USER_ID_LENGTH; i++) {
                char c = id->id_string[i];
                if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
                    result = EOS_FALSE;
                    break;
                }
            }
        }
#ifdef _WIN32
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        EOS_LOG_WARN("EOS_ProductUserId_IsValid: bad pointer %p -> false", (void*)AccountId);
        result = EOS_FALSE;
    }
#endif
    return result;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_ProductUserId_ToString(
    EOS_ProductUserId AccountId,
    char* OutBuffer,
    int32_t* InOutBufferLength
) {
    if (!OutBuffer || !InOutBufferLength) {
        return EOS_InvalidParameters;
    }

    if (!EOS_ProductUserId_IsValid(AccountId)) {
        return EOS_InvalidUser;
    }

    EOS_ProductUserIdDetails* id = (EOS_ProductUserIdDetails*)AccountId;
    int32_t required = PRODUCT_USER_ID_LENGTH + 1;  // +1 for null

    if (*InOutBufferLength < required) {
        *InOutBufferLength = required;
        return EOS_LimitExceeded;
    }

    strcpy(OutBuffer, id->id_string);
    *InOutBufferLength = required;
    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_ProductUserId_FromString(const char* ProductUserIdString) {
    if (!ProductUserIdString) return NULL;

    size_t len = strlen(ProductUserIdString);
    if (len != PRODUCT_USER_ID_LENGTH) return NULL;

    // Validate hex string
    for (size_t i = 0; i < PRODUCT_USER_ID_LENGTH; i++) {
        char c = ProductUserIdString[i];
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
            return NULL;
        }
    }

    // Normalize (uppercase) into a local buffer first so interning compares the
    // canonical form.
    char norm[PRODUCT_USER_ID_LENGTH + 1];
    for (size_t i = 0; i < PRODUCT_USER_ID_LENGTH; i++) {
        char c = ProductUserIdString[i];
        norm[i] = (c >= 'a' && c <= 'f') ? (char)(c - 32) : c;
    }
    norm[PRODUCT_USER_ID_LENGTH] = '\0';

    // INTERN: the same puid string ALWAYS returns the same handle. Real EOS interns
    // ProductUserIds and UE's account-id registry keys on the handle identity, so a
    // fresh allocation per call meant a remote (host) puid — handed in via the
    // session owner AND via GetExternalAccountMapping — appeared as several
    // different handles, none of which UE could register. The host user then
    // couldn't be formed during an Epic join (UserKeyHandle:-1 / E011). Static
    // storage is safe: EOS exposes no puid-release API; interned ids live forever.
    static EOS_ProductUserIdDetails g_interned_puids[256];
    static int g_interned_puid_count = 0;
    for (int i = 0; i < g_interned_puid_count; i++) {
        if (strcmp(g_interned_puids[i].id_string, norm) == 0) {
            return (EOS_ProductUserId)&g_interned_puids[i];
        }
    }
    if (g_interned_puid_count >= (int)(sizeof(g_interned_puids) / sizeof(g_interned_puids[0]))) {
        EOS_LOG_WARN("ProductUserId_FromString: intern table full, returning NULL");
        return NULL;
    }
    EOS_ProductUserIdDetails* id = &g_interned_puids[g_interned_puid_count++];
    id->magic = PUID_MAGIC;
    memcpy(id->id_string, norm, PRODUCT_USER_ID_LENGTH + 1);
    return (EOS_ProductUserId)id;
}

// ============================================================================
// Notification Functions
// ============================================================================

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Connect_AddNotifyLoginStatusChanged(
    EOS_HConnect Handle,
    const EOS_Connect_AddNotifyLoginStatusChangedOptions* Options,
    void* ClientData,
    const EOS_Connect_OnLoginStatusChangedCallback NotificationFn
) {
    EOS_LOG_API_ENTER();

    ConnectState* state = (ConnectState*)Handle;
    if (!state || state->magic != CONNECT_MAGIC) {
        EOS_LOG_ERROR("Invalid Connect handle");
        return EOS_INVALID_NOTIFICATIONID;
    }

    if (!Options || Options->ApiVersion != EOS_CONNECT_ADDNOTIFYLOGINSTATUSCHANGED_API_LATEST) {
        EOS_LOG_ERROR("Invalid AddNotifyLoginStatusChanged options");
        return EOS_INVALID_NOTIFICATIONID;
    }

    if (!NotificationFn) {
        EOS_LOG_ERROR("NULL notification callback");
        return EOS_INVALID_NOTIFICATIONID;
    }

    // Find free notification slot
    int slot = -1;
    for (int i = 0; i < MAX_LOGIN_NOTIFICATIONS; i++) {
        if (!state->login_notifications[i].active) {
            slot = i;
            break;
        }
    }

    if (slot < 0) {
        EOS_LOG_ERROR("Maximum login status notifications reached");
        return EOS_INVALID_NOTIFICATIONID;
    }

    EOS_NotificationId notif_id = state->next_notification_id++;
    state->login_notifications[slot].callback = NotificationFn;
    state->login_notifications[slot].client_data = ClientData;
    state->login_notifications[slot].id = notif_id;
    state->login_notifications[slot].active = true;
    state->login_notification_count++;

    EOS_LOG_DEBUG("Added login status notification (ID: %llu)", (unsigned long long)notif_id);
    return notif_id;
}

EOS_DECLARE_FUNC(void) EOS_Connect_RemoveNotifyLoginStatusChanged(
    EOS_HConnect Handle,
    EOS_NotificationId InId
) {
    EOS_LOG_API_ENTER();

    ConnectState* state = (ConnectState*)Handle;
    if (!state || state->magic != CONNECT_MAGIC) {
        EOS_LOG_ERROR("Invalid Connect handle");
        return;
    }

    if (InId == EOS_INVALID_NOTIFICATIONID) {
        EOS_LOG_WARN("Attempted to remove invalid notification ID");
        return;
    }

    // Find and remove notification
    for (int i = 0; i < MAX_LOGIN_NOTIFICATIONS; i++) {
        if (state->login_notifications[i].active && state->login_notifications[i].id == InId) {
            state->login_notifications[i].active = false;
            state->login_notification_count--;
            EOS_LOG_DEBUG("Removed login status notification (ID: %llu)", (unsigned long long)InId);
            return;
        }
    }

    EOS_LOG_WARN("Login status notification not found (ID: %llu)", (unsigned long long)InId);
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Connect_AddNotifyAuthExpiration(
    EOS_HConnect Handle,
    const EOS_Connect_AddNotifyAuthExpirationOptions* Options,
    void* ClientData,
    const EOS_Connect_OnAuthExpirationCallback NotificationFn
) {
    EOS_LOG_API_ENTER();

    ConnectState* state = (ConnectState*)Handle;
    if (!state || state->magic != CONNECT_MAGIC) {
        EOS_LOG_ERROR("Invalid Connect handle");
        return EOS_INVALID_NOTIFICATIONID;
    }

    if (!NotificationFn) {
        EOS_LOG_ERROR("NULL notification callback");
        return EOS_INVALID_NOTIFICATIONID;
    }

    // Find free notification slot
    int slot = -1;
    for (int i = 0; i < MAX_AUTH_EXPIRATION_NOTIFICATIONS; i++) {
        if (!state->auth_expiration_notifications[i].active) {
            slot = i;
            break;
        }
    }

    if (slot < 0) {
        EOS_LOG_ERROR("Maximum auth expiration notifications reached");
        return EOS_INVALID_NOTIFICATIONID;
    }

    EOS_NotificationId notif_id = state->next_notification_id++;
    state->auth_expiration_notifications[slot].callback = NotificationFn;
    state->auth_expiration_notifications[slot].client_data = ClientData;
    state->auth_expiration_notifications[slot].id = notif_id;
    state->auth_expiration_notifications[slot].active = true;
    state->auth_expiration_notification_count++;

    EOS_LOG_DEBUG("Added auth expiration notification (ID: %llu)", (unsigned long long)notif_id);

    // Note: For LAN emulator, auth never expires so this callback will never fire
    return notif_id;
}

EOS_DECLARE_FUNC(void) EOS_Connect_RemoveNotifyAuthExpiration(
    EOS_HConnect Handle,
    EOS_NotificationId InId
) {
    EOS_LOG_API_ENTER();

    ConnectState* state = (ConnectState*)Handle;
    if (!state || state->magic != CONNECT_MAGIC) {
        EOS_LOG_ERROR("Invalid Connect handle");
        return;
    }

    if (InId == EOS_INVALID_NOTIFICATIONID) {
        EOS_LOG_WARN("Attempted to remove invalid notification ID");
        return;
    }

    // Find and remove notification
    for (int i = 0; i < MAX_AUTH_EXPIRATION_NOTIFICATIONS; i++) {
        if (state->auth_expiration_notifications[i].active &&
            state->auth_expiration_notifications[i].id == InId) {
            state->auth_expiration_notifications[i].active = false;
            state->auth_expiration_notification_count--;
            EOS_LOG_DEBUG("Removed auth expiration notification (ID: %llu)", (unsigned long long)InId);
            return;
        }
    }

    EOS_LOG_WARN("Auth expiration notification not found (ID: %llu)", (unsigned long long)InId);
}

// ============================================================================
// Stub Functions (Not Supported in LAN Emulator)
// ============================================================================

EOS_DECLARE_FUNC(void) EOS_Connect_LinkAccount(
    EOS_HConnect Handle,
    const EOS_Connect_LinkAccountOptions* Options,
    void* ClientData,
    const EOS_Connect_OnLinkAccountCallback CompletionDelegate
) {
    EOS_LOG_API_ENTER();
    EOS_LOG_WARN("LinkAccount not supported in LAN emulator - returning success");

    if (CompletionDelegate) {
        EOS_Connect_LinkAccountCallbackInfo info = {0};
        info.ResultCode = EOS_Success;
        info.ClientData = ClientData;
        info.LocalUserId = Options ? Options->LocalUserId : NULL;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_Connect_UnlinkAccount(
    EOS_HConnect Handle,
    const EOS_Connect_UnlinkAccountOptions* Options,
    void* ClientData,
    const EOS_Connect_OnUnlinkAccountCallback CompletionDelegate
) {
    EOS_LOG_API_ENTER();
    EOS_LOG_WARN("UnlinkAccount not supported in LAN emulator - returning success");

    if (CompletionDelegate) {
        EOS_Connect_UnlinkAccountCallbackInfo info = {0};
        info.ResultCode = EOS_Success;
        info.ClientData = ClientData;
        info.LocalUserId = Options ? Options->LocalUserId : NULL;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_Connect_CreateDeviceId(
    EOS_HConnect Handle,
    const EOS_Connect_CreateDeviceIdOptions* Options,
    void* ClientData,
    const EOS_Connect_OnCreateDeviceIdCallback CompletionDelegate
) {
    EOS_LOG_API_ENTER();
    EOS_LOG_WARN("CreateDeviceId not supported in LAN emulator - returning success");

    if (CompletionDelegate) {
        EOS_Connect_CreateDeviceIdCallbackInfo info = {0};
        info.ResultCode = EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_Connect_DeleteDeviceId(
    EOS_HConnect Handle,
    const EOS_Connect_DeleteDeviceIdOptions* Options,
    void* ClientData,
    const EOS_Connect_OnDeleteDeviceIdCallback CompletionDelegate
) {
    EOS_LOG_API_ENTER();
    EOS_LOG_WARN("DeleteDeviceId not supported in LAN emulator - returning success");

    if (CompletionDelegate) {
        EOS_Connect_DeleteDeviceIdCallbackInfo info = {0};
        info.ResultCode = EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_Connect_TransferDeviceIdAccount(
    EOS_HConnect Handle,
    const EOS_Connect_TransferDeviceIdAccountOptions* Options,
    void* ClientData,
    const EOS_Connect_OnTransferDeviceIdAccountCallback CompletionDelegate
) {
    EOS_LOG_API_ENTER();
    EOS_LOG_WARN("TransferDeviceIdAccount not supported in LAN emulator - returning success");

    if (CompletionDelegate) {
        EOS_Connect_TransferDeviceIdAccountCallbackInfo info = {0};
        info.ResultCode = EOS_Success;
        info.ClientData = ClientData;
        info.LocalUserId = Options ? Options->PrimaryLocalUserId : NULL;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_Connect_QueryExternalAccountMappings(
    EOS_HConnect Handle,
    const EOS_Connect_QueryExternalAccountMappingsOptions* Options,
    void* ClientData,
    const EOS_Connect_OnQueryExternalAccountMappingsCallback CompletionDelegate
) {
    EOS_LOG_INFO(">>> QueryExternalAccountMappings called");

    if (CompletionDelegate) {
        EOS_Connect_QueryExternalAccountMappingsCallbackInfo info = {0};
        info.ResultCode = EOS_Success;  // Return success
        info.ClientData = ClientData;
        info.LocalUserId = Options ? Options->LocalUserId : NULL;
        EOS_LOG_INFO(">>> QueryExternalAccountMappings returning Success");
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_Connect_QueryProductUserIdMappings(
    EOS_HConnect Handle,
    const EOS_Connect_QueryProductUserIdMappingsOptions* Options,
    void* ClientData,
    const EOS_Connect_OnQueryProductUserIdMappingsCallback CompletionDelegate
) {
    EOS_LOG_INFO(">>> QueryProductUserIdMappings called");

    if (CompletionDelegate) {
        EOS_Connect_QueryProductUserIdMappingsCallbackInfo info = {0};
        info.ResultCode = EOS_Success;  // Return success so GetProductUserIdMapping works
        info.ClientData = ClientData;
        info.LocalUserId = Options ? Options->LocalUserId : NULL;
        EOS_LOG_INFO(">>> QueryProductUserIdMappings returning Success");
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_Connect_GetExternalAccountMapping(
    EOS_HConnect Handle,
    const EOS_Connect_GetExternalAccountMappingsOptions* Options
) {
    if (!Options || !Options->TargetExternalUserId) return NULL;
    EOS_LOG_INFO(">>> GetExternalAccountMapping: type=%d target='%s'",
                 (int)Options->AccountIdType, Options->TargetExternalUserId);

    // The game maps an external (Epic) account id -> ProductUserId to resolve a
    // joinable host's (and its own) identity. Epic ids and ProductUserIds are
    // DISTINCT, so we must return the user's REAL ProductUserId (matching the
    // session owner), NOT a reshaped Epic id. The social bridge knows both: the
    // local user's Connect id and each peer's puid (from its LAN beacon).
    ConnectState* state = (ConnectState*)Handle;
    if (state && state->platform) {
        EOS_ProductUserId puid;
        if (Options->AccountIdType == EOS_EAT_STEAM) {
            // Steam-friend-driven join UIs map a Steam friend's id -> PUID here.
            puid = social_bridge_resolve_puid_by_steam(state->platform, Options->TargetExternalUserId);
        } else {
            // Default/Epic path: target is an EpicAccountId string.
            puid = social_bridge_resolve_puid(state->platform, Options->TargetExternalUserId);
        }
        if (puid) {
            EOS_LOG_INFO(">>> GetExternalAccountMapping(type=%d, '%s') -> puid '%s'",
                         (int)Options->AccountIdType, Options->TargetExternalUserId,
                         ((EOS_ProductUserIdDetails*)puid)->id_string);
            return puid;
        }
    }
    EOS_LOG_WARN(">>> GetExternalAccountMapping -> no mapping found for '%s' (type=%d)",
                 Options->TargetExternalUserId, (int)Options->AccountIdType);
    return NULL;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Connect_GetProductUserExternalAccountCount(
    EOS_HConnect Handle,
    const EOS_Connect_GetProductUserExternalAccountCountOptions* Options
) {
    EOS_LOG_API_ENTER();
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Connect_CopyProductUserExternalAccountByIndex(
    EOS_HConnect Handle,
    const EOS_Connect_CopyProductUserExternalAccountByIndexOptions* Options,
    EOS_Connect_ExternalAccountInfo** OutExternalAccountInfo
) {
    EOS_LOG_API_ENTER();
    EOS_LOG_WARN("CopyProductUserExternalAccountByIndex not supported in LAN emulator");
    return EOS_NotFound;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Connect_CopyProductUserExternalAccountByAccountType(
    EOS_HConnect Handle,
    const EOS_Connect_CopyProductUserExternalAccountByAccountTypeOptions* Options,
    EOS_Connect_ExternalAccountInfo** OutExternalAccountInfo
) {
    EOS_LOG_API_ENTER();
    EOS_LOG_WARN("CopyProductUserExternalAccountByAccountType not supported in LAN emulator");
    return EOS_NotFound;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Connect_CopyProductUserExternalAccountByAccountId(
    EOS_HConnect Handle,
    const EOS_Connect_CopyProductUserExternalAccountByAccountIdOptions* Options,
    EOS_Connect_ExternalAccountInfo** OutExternalAccountInfo
) {
    EOS_LOG_API_ENTER();
    EOS_LOG_WARN("CopyProductUserExternalAccountByAccountId not supported in LAN emulator");
    return EOS_NotFound;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Connect_CopyProductUserInfo(
    EOS_HConnect Handle,
    const EOS_Connect_CopyProductUserInfoOptions* Options,
    EOS_Connect_ExternalAccountInfo** OutExternalAccountInfo
) {
    EOS_LOG_API_ENTER();
    EOS_LOG_WARN("CopyProductUserInfo not supported in LAN emulator");
    return EOS_NotFound;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Connect_CopyIdToken(
    EOS_HConnect Handle,
    const EOS_Connect_CopyIdTokenOptions* Options,
    EOS_Connect_IdToken** OutIdToken
) {
    EOS_LOG_API_ENTER();
    if (!Options || !OutIdToken || !Options->LocalUserId) {
        if (OutIdToken) *OutIdToken = NULL;
        return EOS_InvalidParameters;
    }

    // Mint a LAN ID token. On a real deployment this is an Epic-signed JWT; on
    // LAN both peers are this same emulator, so we use the local user's 32-char
    // PUID hex as the token payload. The receiving peer's VerifyIdToken just
    // reads the ProductUserId back out — enough for V Rising's connect/identity
    // handshake without real Epic signing. Freed by EOS_Connect_IdToken_Release.
    char hex[33];
    int32_t hlen = (int32_t)sizeof(hex);
    if (EOS_ProductUserId_ToString(Options->LocalUserId, hex, &hlen) != EOS_Success) {
        *OutIdToken = NULL;
        return EOS_InvalidParameters;
    }

    EOS_Connect_IdToken* tok = calloc(1, sizeof(*tok));
    char* jwt = malloc(sizeof(hex));
    if (!tok || !jwt) {
        free(tok); free(jwt);
        *OutIdToken = NULL;
        return EOS_UnexpectedError;
    }
    memcpy(jwt, hex, sizeof(hex));
    tok->ApiVersion = EOS_CONNECT_IDTOKEN_API_LATEST;
    tok->ProductUserId = Options->LocalUserId;
    tok->JsonWebToken = jwt;
    *OutIdToken = tok;
    EOS_LOG_INFO("CopyIdToken: issued LAN id token for %s", hex);
    return EOS_Success;
}

EOS_DECLARE_FUNC(void) EOS_Connect_VerifyIdToken(
    EOS_HConnect Handle,
    const EOS_Connect_VerifyIdTokenOptions* Options,
    void* ClientData,
    const EOS_Connect_OnVerifyIdTokenCallback CompletionDelegate
) {
    EOS_LOG_API_ENTER();

    if (CompletionDelegate) {
        EOS_Connect_VerifyIdTokenCallbackInfo info = {0};
        info.ResultCode = EOS_Success;
        info.ClientData = ClientData;
        // Our LAN tokens carry the issuer's PUID (see EOS_Connect_CopyIdToken);
        // echo it back so the verifying peer learns who connected. The caller
        // populates Options->IdToken.ProductUserId via FromString(JWT).
        if (Options && Options->IdToken) {
            info.ProductUserId = Options->IdToken->ProductUserId;
        }
        info.bIsAccountInfoPresent = EOS_FALSE;
        EOS_LOG_INFO("VerifyIdToken: accepted LAN id token (PUID %s)",
                     info.ProductUserId ? "present" : "null");
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_Connect_GetExternalAccountMappings(
    EOS_HConnect Handle,
    const EOS_Connect_GetExternalAccountMappingsOptions* Options
) {
    EOS_LOG_INFO(">>> GetExternalAccountMappings called");
    EOS_LOG_WARN("GetExternalAccountMappings not supported in LAN emulator");
    return NULL;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Connect_GetProductUserIdMapping(
    EOS_HConnect Handle,
    const EOS_Connect_GetProductUserIdMappingOptions* Options,
    char* OutBuffer,
    int32_t* InOutBufferLength
) {
    EOS_LOG_INFO(">>> GetProductUserIdMapping called - AccountIdType=%d",
                 Options ? (int)Options->AccountIdType : -1);

    ConnectState* state = (ConnectState*)Handle;
    if (!Options || Options->AccountIdType != EOS_EAT_EPIC || !OutBuffer || !InOutBufferLength) {
        return EOS_NotFound;
    }

    // Reverse mapping: given a ProductUserId, return ITS EpicAccountId. The game
    // calls this on the HOST's puid (the session owner) to rebuild the host's
    // FAccountId during a join. Returning the LOCAL user's epic for every puid
    // mapped the host to the wrong identity -> the host user couldn't be formed
    // -> EOS join died with UserKeyHandle:-1 / E011. Resolve the epic that
    // actually belongs to TargetProductUserId via the social bridge.
    char puid_str[33] = {0};
    int32_t plen = (int32_t)sizeof(puid_str);
    const char* epic = NULL;
    if (Options->TargetProductUserId && state && state->platform &&
        EOS_ProductUserId_ToString(Options->TargetProductUserId, puid_str, &plen) == EOS_Success) {
        epic = social_bridge_resolve_epic_by_puid(state->platform, puid_str);
    }
    // No target given (rare) but logged in: assume the local user.
    if (!epic && !Options->TargetProductUserId && g_auth_state.logged_in) {
        epic = g_auth_state.account_id.id_string;
    }

    if (epic) {
        int required = (int)strlen(epic) + 1;
        if (*InOutBufferLength >= required) {
            strcpy(OutBuffer, epic);
            *InOutBufferLength = required;
            EOS_LOG_INFO(">>> GetProductUserIdMapping: puid '%s' -> EpicAccountId=%s", puid_str, epic);
            return EOS_Success;
        }
        *InOutBufferLength = required;
        return EOS_LimitExceeded;
    }
    return EOS_NotFound;
}

// ============================================================================
// Release Functions
// ============================================================================

EOS_DECLARE_FUNC(void) EOS_ProductUserIdDetails_Release(EOS_ProductUserId UserId) {
    // No-op. ProductUserIds are now INTERNED (see EOS_ProductUserId_FromString)
    // and live for the process lifetime, matching real EOS (which exposes no
    // release for account ids). Freeing here would corrupt the static intern
    // table / ConnectState slots. This isn't a real EOS export and is unused.
    (void)UserId;
}

EOS_DECLARE_FUNC(void) EOS_Connect_ExternalAccountInfo_Release(EOS_Connect_ExternalAccountInfo* ExternalAccountInfo) {
    // No-op in LAN emulator as we never allocate these
    (void)ExternalAccountInfo;
}

EOS_DECLARE_FUNC(void) EOS_Connect_IdToken_Release(EOS_Connect_IdToken* IdToken) {
    if (!IdToken) return;
    if (IdToken->JsonWebToken) free((void*)IdToken->JsonWebToken);
    free(IdToken);
}
