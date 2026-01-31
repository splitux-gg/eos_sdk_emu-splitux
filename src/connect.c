#include "internal/connect_internal.h"
#include "internal/auth_internal.h"
#include "internal/logging.h"
#include "eos/eos_connect.h"
#include "eos/eos_connect_types.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Magic number for validation
#define CONNECT_MAGIC 0x434F4E4E  // "CONN"
#define PUID_MAGIC 0x50554944     // "PUID"

// ============================================================================
// Internal Helper Functions
// ============================================================================

static void connect_init_instance_id(ConnectState* state) {
    // For now, generate a random instance ID
    // In a full implementation, this would read from config
    srand((unsigned int)time(NULL));

    for (int i = 0; i < 16; i++) {
        state->instance_id[i] = "0123456789ABCDEF"[rand() % 16];
    }
    state->instance_id[16] = '\0';

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

    // Format: {instance_id:16hex}{user_index:4hex}{random:4hex}{padding:8hex}
    uint16_t random_part = (uint16_t)(rand() & 0xFFFF);

    snprintf(id->id_string, sizeof(id->id_string),
             "%s%04X%04X00000000",
             state->instance_id,
             (uint16_t)user_index,
             random_part);

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

    // Queue/call completion callback
    if (CompletionDelegate) {
        CompletionDelegate(&info);
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

    EOS_ProductUserIdDetails* id = (EOS_ProductUserIdDetails*)AccountId;
    if (id->magic != PUID_MAGIC) return EOS_FALSE;

    // Check string is valid hex, correct length
    size_t len = strlen(id->id_string);
    if (len != PRODUCT_USER_ID_LENGTH) return EOS_FALSE;

    for (size_t i = 0; i < PRODUCT_USER_ID_LENGTH; i++) {
        char c = id->id_string[i];
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
            return EOS_FALSE;
        }
    }

    return EOS_TRUE;
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

    EOS_ProductUserIdDetails* id = calloc(1, sizeof(EOS_ProductUserIdDetails));
    if (!id) return NULL;

    id->magic = PUID_MAGIC;
    strncpy(id->id_string, ProductUserIdString, PRODUCT_USER_ID_LENGTH);
    id->id_string[PRODUCT_USER_ID_LENGTH] = '\0';

    // Uppercase for consistency
    for (size_t i = 0; i < PRODUCT_USER_ID_LENGTH; i++) {
        if (id->id_string[i] >= 'a' && id->id_string[i] <= 'f') {
            id->id_string[i] -= 32;
        }
    }

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
    EOS_LOG_INFO(">>> GetExternalAccountMapping called - AccountIdType=%d",
                 Options ? (int)Options->AccountIdType : -1);
    EOS_LOG_WARN("GetExternalAccountMapping not supported in LAN emulator");
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
    EOS_LOG_WARN("CopyIdToken not supported in LAN emulator");
    return EOS_NotFound;
}

EOS_DECLARE_FUNC(void) EOS_Connect_VerifyIdToken(
    EOS_HConnect Handle,
    const EOS_Connect_VerifyIdTokenOptions* Options,
    void* ClientData,
    const EOS_Connect_OnVerifyIdTokenCallback CompletionDelegate
) {
    EOS_LOG_API_ENTER();
    EOS_LOG_WARN("VerifyIdToken not supported in LAN emulator - returning success");

    if (CompletionDelegate) {
        EOS_Connect_VerifyIdTokenCallbackInfo info = {0};
        info.ResultCode = EOS_Success;
        info.ClientData = ClientData;
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

    // For Epic account type, return our EpicAccountId
    if (Options && Options->AccountIdType == EOS_EAT_EPIC) {
        extern AuthState g_auth_state;
        if (g_auth_state.logged_in && OutBuffer && InOutBufferLength) {
            int required = (int)strlen(g_auth_state.account_id.id_string) + 1;
            if (*InOutBufferLength >= required) {
                strcpy(OutBuffer, g_auth_state.account_id.id_string);
                *InOutBufferLength = required;
                EOS_LOG_INFO(">>> GetProductUserIdMapping returning EpicAccountId=%s",
                             g_auth_state.account_id.id_string);
                return EOS_Success;
            }
            *InOutBufferLength = required;
            return EOS_LimitExceeded;
        }
    }
    return EOS_NotFound;
}

// ============================================================================
// Release Functions
// ============================================================================

EOS_DECLARE_FUNC(void) EOS_ProductUserIdDetails_Release(EOS_ProductUserId UserId) {
    // For user IDs created via FromString, we need to free them
    // For user IDs returned by our Login/CreateUser, they're owned by ConnectState
    // To be safe, we check if this is a standalone allocation
    if (!UserId) return;

    EOS_ProductUserIdDetails* details = (EOS_ProductUserIdDetails*)UserId;
    if (details->magic == PUID_MAGIC) {
        // This looks like a valid standalone ID, free it
        free(details);
    }
}

EOS_DECLARE_FUNC(void) EOS_Connect_ExternalAccountInfo_Release(EOS_Connect_ExternalAccountInfo* ExternalAccountInfo) {
    // No-op in LAN emulator as we never allocate these
    (void)ExternalAccountInfo;
}

EOS_DECLARE_FUNC(void) EOS_Connect_IdToken_Release(EOS_Connect_IdToken* IdToken) {
    // No-op in LAN emulator as we never allocate these
    (void)IdToken;
}
