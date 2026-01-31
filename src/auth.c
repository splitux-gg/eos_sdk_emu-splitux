/**
 * EOS-LAN Emulator - Auth Implementation
 * Provides fake Epic Account authentication for LAN play
 */

#include "eos/eos_auth.h"
#include "internal/auth_internal.h"
#include "internal/platform_internal.h"
#include "internal/callbacks.h"
#include "internal/logging.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Helper to get the callback queue from the first platform
static CallbackQueue* get_callback_queue(void) {
    for (int i = 0; i < 8; i++) {
        if (g_platforms[i] && g_platforms[i]->callbacks) {
            return g_platforms[i]->callbacks;
        }
    }
    return NULL;
}

// Global auth state
AuthState g_auth_state = {0};

// DLL constructor - runs when DLL is loaded, BEFORE any EOS calls
#ifdef _WIN32
#include <windows.h>
static void auth_dll_init(void);
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        auth_dll_init();
    }
    return TRUE;
}
#else
__attribute__((constructor))
#endif
static void auth_dll_init(void) {
    // Initialize auth state with a valid EpicAccountId at DLL load time
    // This ensures the ID is available BEFORE any EOS functions are called
    g_auth_state.account_id.magic = EAID_MAGIC;

    // Generate deterministic ID based on process ID for consistency
    unsigned int seed = 12345;  // Fixed seed for now
    srand(seed);
    for (int i = 0; i < EPIC_ACCOUNT_ID_LENGTH; i++) {
        int r = rand() % 16;
        g_auth_state.account_id.id_string[i] = (r < 10) ? ('0' + r) : ('a' + r - 10);
    }
    g_auth_state.account_id.id_string[EPIC_ACCOUNT_ID_LENGTH] = '\0';
    g_auth_state.logged_in = false;  // Start NotLoggedIn like Nemirtingas
    strncpy(g_auth_state.display_name, "LAN_Player", sizeof(g_auth_state.display_name) - 1);

    // Log the initialization
    EOS_LOG_INFO("=== DLL INIT (auth_dll_init) ===");
    EOS_LOG_INFO("    account_id=%s (ptr=%p)", g_auth_state.account_id.id_string,
                 (void*)&g_auth_state.account_id);
    EOS_LOG_INFO("    magic=0x%08X, logged_in=%d, steam_login_completed=%d",
                 g_auth_state.account_id.magic, g_auth_state.logged_in,
                 g_auth_state.steam_login_completed);
}

void auth_init(void) {
    // Now a no-op since we init during DLL load
}

// Generate a fake Epic Account ID based on some seed
static void generate_epic_account_id(EOS_EpicAccountIdDetails* id) {
    id->magic = EAID_MAGIC;

    // Generate random-looking ID
    srand((unsigned int)time(NULL) ^ (unsigned int)(uintptr_t)id);
    for (int i = 0; i < EPIC_ACCOUNT_ID_LENGTH; i++) {
        int r = rand() % 16;
        id->id_string[i] = (r < 10) ? ('0' + r) : ('a' + r - 10);
    }
    id->id_string[EPIC_ACCOUNT_ID_LENGTH] = '\0';
}

// Auto-login for LAN play - called during platform creation
void auth_auto_login(void) {
    EOS_LOG_INFO(">>> auth_auto_login CALLED <<<");
    EOS_LOG_INFO("    Current state: logged_in=%d, account_id=%s",
                 g_auth_state.logged_in, g_auth_state.account_id.id_string);

    if (g_auth_state.logged_in) {
        EOS_LOG_INFO("    Already logged in - skipping");
        return;
    }

    generate_epic_account_id(&g_auth_state.account_id);
    g_auth_state.logged_in = true;
    strncpy(g_auth_state.display_name, "LAN_Player", sizeof(g_auth_state.display_name) - 1);

    EOS_LOG_INFO("    Auto-login complete: EpicAccountId=%s (ptr=%p)",
                 g_auth_state.account_id.id_string, (void*)&g_auth_state.account_id);
}

EOS_DECLARE_FUNC(void) EOS_Auth_Login(EOS_HAuth Handle, const EOS_Auth_LoginOptions* Options, void* ClientData, const EOS_Auth_OnLoginCallback CompletionDelegate) {
    EOS_LOG_INFO(">>> EOS_Auth_Login CALLED <<<");
    EOS_LOG_INFO("    Handle=%p, Options=%p, ClientData=%p, Callback=%p",
                 (void*)Handle, (void*)Options, ClientData, (void*)CompletionDelegate);

    // Log Options details if available
    if (Options) {
        EOS_LOG_INFO("    Options->ApiVersion=%d", Options->ApiVersion);
        if (Options->Credentials) {
            EOS_LOG_INFO("    Credentials->Type=%d, Token=%s, ExternalType=%d",
                         (int)Options->Credentials->Type,
                         Options->Credentials->Token ? "(present)" : "(null)",
                         (int)Options->Credentials->ExternalType);
        } else {
            EOS_LOG_INFO("    Credentials=NULL");
        }
    }

    // Generate fake Epic account
    generate_epic_account_id(&g_auth_state.account_id);
    g_auth_state.logged_in = true;
    strncpy(g_auth_state.display_name, "LAN_Player", sizeof(g_auth_state.display_name) - 1);

    EOS_LOG_INFO("    Auth_Login SUCCESS - EpicAccountId=%s (ptr=%p)",
                 g_auth_state.account_id.id_string, (void*)&g_auth_state.account_id);

    if (CompletionDelegate) {
        EOS_Auth_LoginCallbackInfo info = {0};
        info.ResultCode = EOS_Success;
        info.ClientData = ClientData;
        info.LocalUserId = (EOS_EpicAccountId)&g_auth_state.account_id;
        EOS_LOG_INFO("    Firing Auth_Login callback with LocalUserId=%p", (void*)info.LocalUserId);
        CompletionDelegate(&info);
        EOS_LOG_INFO("    Auth_Login callback returned");
    } else {
        EOS_LOG_INFO("    No callback provided - not firing");
    }
}

EOS_DECLARE_FUNC(void) EOS_Auth_Logout(EOS_HAuth Handle, const EOS_Auth_LogoutOptions* Options, void* ClientData, const EOS_Auth_OnLogoutCallback CompletionDelegate) {
    EOS_LOG_INFO("Auth_Logout called");
    g_auth_state.logged_in = false;

    if (CompletionDelegate) {
        EOS_Auth_LogoutCallbackInfo info = {0};
        info.ResultCode = EOS_Success;
        info.ClientData = ClientData;
        info.LocalUserId = (EOS_EpicAccountId)&g_auth_state.account_id;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_Auth_LinkAccount(EOS_HAuth Handle, const EOS_Auth_LinkAccountOptions* Options, void* ClientData, const EOS_Auth_OnLinkAccountCallback CompletionDelegate) {
    EOS_LOG_INFO(">>> EOS_Auth_LinkAccount CALLED");
    if (CompletionDelegate) {
        EOS_Auth_LinkAccountCallbackInfo info = {0};
        info.ResultCode = EOS_Success;
        info.ClientData = ClientData;
        info.LocalUserId = g_auth_state.logged_in ? (EOS_EpicAccountId)&g_auth_state.account_id : NULL;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_Auth_DeletePersistentAuth(EOS_HAuth Handle, const EOS_Auth_DeletePersistentAuthOptions* Options, void* ClientData, const EOS_Auth_OnDeletePersistentAuthCallback CompletionDelegate) {
    EOS_LOG_INFO(">>> EOS_Auth_DeletePersistentAuth CALLED");
    if (CompletionDelegate) {
        EOS_Auth_DeletePersistentAuthCallbackInfo info = {0};
        info.ResultCode = EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_Auth_VerifyUserAuth(EOS_HAuth Handle, const EOS_Auth_VerifyUserAuthOptions* Options, void* ClientData, const EOS_Auth_OnVerifyUserAuthCallback CompletionDelegate) {
    EOS_LOG_INFO(">>> EOS_Auth_VerifyUserAuth CALLED");
    if (CompletionDelegate) {
        EOS_Auth_VerifyUserAuthCallbackInfo info = {0};
        info.ResultCode = g_auth_state.logged_in ? EOS_Success : EOS_NotConfigured;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(int32_t) EOS_Auth_GetLoggedInAccountsCount(EOS_HAuth Handle) {
    int32_t count = g_auth_state.logged_in ? 1 : 0;
    EOS_LOG_INFO(">>> Auth_GetLoggedInAccountsCount CALLED - returning %d (logged_in=%d)",
                 count, g_auth_state.logged_in);
    return count;
}

EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_Auth_GetLoggedInAccountByIndex(EOS_HAuth Handle, int32_t Index) {
    EOS_LOG_INFO(">>> Auth_GetLoggedInAccountByIndex(%d) CALLED - returning account_id=%s",
                 Index, g_auth_state.account_id.id_string);
    if (Index != 0) {
        return NULL;
    }
    // Always return our account_id, even before "login"
    // Ensure the account is initialized
    if (g_auth_state.account_id.magic != EAID_MAGIC) {
        auth_auto_login();
    }
    return (EOS_EpicAccountId)&g_auth_state.account_id;
}

EOS_DECLARE_FUNC(EOS_ELoginStatus) EOS_Auth_GetLoginStatus(EOS_HAuth Handle, EOS_EpicAccountId LocalUserId) {
    EOS_ELoginStatus status = g_auth_state.logged_in ? EOS_LS_LoggedIn : EOS_LS_NotLoggedIn;
    EOS_LOG_INFO(">>> Auth_GetLoginStatus CALLED - LocalUserId=%p, logged_in=%d, returning %s",
                 (void*)LocalUserId, g_auth_state.logged_in,
                 status == EOS_LS_LoggedIn ? "LoggedIn" : "NotLoggedIn");
    return status;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Auth_CopyUserAuthToken(EOS_HAuth Handle, const EOS_Auth_CopyUserAuthTokenOptions* Options, EOS_EpicAccountId LocalUserId, EOS_Auth_Token** OutUserAuthToken) {
    EOS_LOG_INFO(">>> EOS_Auth_CopyUserAuthToken CALLED - logged_in=%d", g_auth_state.logged_in);

    if (!OutUserAuthToken) {
        return EOS_InvalidParameters;
    }

    if (!g_auth_state.logged_in) {
        *OutUserAuthToken = NULL;
        return EOS_NotFound;
    }

    // Allocate and populate a fake token with our EpicAccountId
    EOS_Auth_Token* token = calloc(1, sizeof(EOS_Auth_Token));
    if (!token) {
        *OutUserAuthToken = NULL;
        return EOS_UnexpectedError;
    }

    token->ApiVersion = EOS_AUTH_TOKEN_API_LATEST;
    token->App = "Pal";
    token->ClientId = "xyza7891muomRmynIIHaJB9COBKkwj6n";  // Palworld's client ID
    token->AccountId = (EOS_EpicAccountId)&g_auth_state.account_id;
    token->AccessToken = "LAN_FAKE_ACCESS_TOKEN";
    token->ExpiresIn = 7200.0;  // 2 hours
    token->ExpiresAt = "2099-12-31T23:59:59.999Z";
    token->AuthType = EOS_ATT_User;
    token->RefreshToken = "LAN_FAKE_REFRESH_TOKEN";
    token->RefreshExpiresIn = 86400.0;  // 24 hours
    token->RefreshExpiresAt = "2099-12-31T23:59:59.999Z";

    EOS_LOG_INFO(">>> Returning fake auth token with AccountId=%s", g_auth_state.account_id.id_string);
    *OutUserAuthToken = token;
    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Auth_CopyIdToken(EOS_HAuth Handle, const EOS_Auth_CopyIdTokenOptions* Options, EOS_Auth_IdToken** OutIdToken) {
    EOS_LOG_INFO("Auth_CopyIdToken called");
    if (OutIdToken) *OutIdToken = NULL;
    return EOS_NotFound;
}

EOS_DECLARE_FUNC(void) EOS_Auth_Token_Release(EOS_Auth_Token* AuthToken) {
    EOS_LOG_INFO(">>> EOS_Auth_Token_Release CALLED");
    if (AuthToken) {
        free(AuthToken);
    }
}

EOS_DECLARE_FUNC(void) EOS_Auth_IdToken_Release(EOS_Auth_IdToken* IdToken) {
    EOS_LOG_INFO(">>> EOS_Auth_IdToken_Release CALLED");
    if (IdToken) {
        free(IdToken);
    }
}

EOS_DECLARE_FUNC(void) EOS_Auth_QueryIdToken(EOS_HAuth Handle, const EOS_Auth_QueryIdTokenOptions* Options, void* ClientData, const EOS_Auth_OnQueryIdTokenCallback CompletionDelegate) {
    EOS_LOG_INFO(">>> EOS_Auth_QueryIdToken CALLED");
    if (CompletionDelegate) {
        EOS_Auth_QueryIdTokenCallbackInfo info = {0};
        info.ResultCode = EOS_Success;
        info.ClientData = ClientData;
        info.LocalUserId = g_auth_state.logged_in ? (EOS_EpicAccountId)&g_auth_state.account_id : NULL;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_Auth_VerifyIdToken(EOS_HAuth Handle, const EOS_Auth_VerifyIdTokenOptions* Options, void* ClientData, const EOS_Auth_OnVerifyIdTokenCallback CompletionDelegate) {
    EOS_LOG_INFO(">>> EOS_Auth_VerifyIdToken CALLED");
    if (CompletionDelegate) {
        EOS_Auth_VerifyIdTokenCallbackInfo info = {0};
        info.ResultCode = EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Auth_GetSelectedAccountId(EOS_HAuth Handle, const EOS_EpicAccountId LocalUserId, EOS_EpicAccountId* OutSelectedAccountId) {
    EOS_LOG_INFO(">>> EOS_Auth_GetSelectedAccountId CALLED - returning account");
    if (OutSelectedAccountId) {
        *OutSelectedAccountId = g_auth_state.logged_in ? (EOS_EpicAccountId)&g_auth_state.account_id : NULL;
    }
    return g_auth_state.logged_in ? EOS_Success : EOS_NotFound;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Auth_GetMergedAccountsCount(EOS_HAuth Handle, const EOS_EpicAccountId LocalUserId) {
    EOS_LOG_INFO(">>> EOS_Auth_GetMergedAccountsCount CALLED");
    return 0;
}

EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_Auth_GetMergedAccountByIndex(EOS_HAuth Handle, const EOS_EpicAccountId LocalUserId, const uint32_t Index) {
    EOS_LOG_INFO(">>> EOS_Auth_GetMergedAccountByIndex CALLED");
    return NULL;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Auth_AddNotifyLoginStatusChanged(EOS_HAuth Handle, const EOS_Auth_AddNotifyLoginStatusChangedOptions* Options, void* ClientData, const EOS_Auth_OnLoginStatusChangedCallback Notification) {
    EOS_LOG_INFO(">>> EOS_Auth_AddNotifyLoginStatusChanged CALLED <<<");
    EOS_LOG_INFO("    Handle=%p, Options=%p, ClientData=%p, Callback=%p",
                 (void*)Handle, (void*)Options, ClientData, (void*)Notification);
    EOS_LOG_INFO("    Current state: logged_in=%d, steam_login_completed=%d",
                 g_auth_state.logged_in, g_auth_state.steam_login_completed);
    EOS_LOG_INFO("    Our account_id=%s (ptr=%p)",
                 g_auth_state.account_id.id_string, (void*)&g_auth_state.account_id);

    static EOS_NotificationId next_id = 1;
    EOS_NotificationId id = next_id++;

    // Store the callback
    if (Notification) {
        bool stored = false;
        for (int i = 0; i < MAX_AUTH_NOTIFICATIONS; i++) {
            if (!g_auth_state.login_status_notifications[i].in_use) {
                g_auth_state.login_status_notifications[i].id = id;
                g_auth_state.login_status_notifications[i].client_data = ClientData;
                g_auth_state.login_status_notifications[i].callback = Notification;
                g_auth_state.login_status_notifications[i].in_use = true;
                EOS_LOG_INFO("    Stored callback in slot %d (notification_id=%llu)",
                             i, (unsigned long long)id);
                stored = true;
                break;
            }
        }
        if (!stored) {
            EOS_LOG_ERROR("    FAILED to store callback - all slots full!");
        }

        // If Steam login already completed, fire callback SYNCHRONOUSLY
        // The game seems to need this before continuing
        if (g_auth_state.steam_login_completed && g_auth_state.logged_in) {
            EOS_Auth_LoginStatusChangedCallbackInfo info = {0};
            info.ClientData = ClientData;
            info.LocalUserId = (EOS_EpicAccountId)&g_auth_state.account_id;
            info.PrevStatus = EOS_LS_NotLoggedIn;
            info.CurrentStatus = EOS_LS_LoggedIn;

            // Fire synchronously - don't queue
            EOS_LOG_INFO("    >>> FIRING CALLBACK SYNCHRONOUSLY <<<");
            EOS_LOG_INFO("    LocalUserId=%p (%s), PrevStatus=NotLoggedIn, CurrentStatus=LoggedIn",
                         (void*)info.LocalUserId, g_auth_state.account_id.id_string);
            Notification(&info);
            EOS_LOG_INFO("    >>> Callback returned <<<");
        } else {
            EOS_LOG_INFO("    NOT firing - steam_login_completed=%d, logged_in=%d",
                         g_auth_state.steam_login_completed, g_auth_state.logged_in);
        }
    } else {
        EOS_LOG_WARN("    Notification callback is NULL!");
    }

    EOS_LOG_INFO("    Returning notification_id=%llu", (unsigned long long)id);
    return id;
}

// Queue all stored login status notifications for Platform_Tick
void auth_fire_login_notifications(EOS_ELoginStatus prev, EOS_ELoginStatus current) {
    EOS_LOG_INFO(">>> auth_fire_login_notifications CALLED <<<");
    EOS_LOG_INFO("    prev=%d, current=%d", (int)prev, (int)current);

    CallbackQueue* q = get_callback_queue();
    int queued = 0;
    for (int i = 0; i < MAX_AUTH_NOTIFICATIONS; i++) {
        if (g_auth_state.login_status_notifications[i].in_use &&
            g_auth_state.login_status_notifications[i].callback) {

            EOS_Auth_LoginStatusChangedCallbackInfo info = {0};
            info.ClientData = g_auth_state.login_status_notifications[i].client_data;
            info.LocalUserId = (EOS_EpicAccountId)&g_auth_state.account_id;
            info.PrevStatus = prev;
            info.CurrentStatus = current;

            if (q) {
                EOS_LOG_INFO("    Queuing callback in slot %d: LocalUserId=%p (%s)",
                             i, (void*)info.LocalUserId, g_auth_state.account_id.id_string);
                callback_queue_push(q, (void*)g_auth_state.login_status_notifications[i].callback,
                                    &info, sizeof(info));
                queued++;
            } else {
                EOS_LOG_INFO("    Firing callback in slot %d (no queue): LocalUserId=%p (%s)",
                             i, (void*)info.LocalUserId, g_auth_state.account_id.id_string);
                g_auth_state.login_status_notifications[i].callback(&info);
            }
        }
    }
    EOS_LOG_INFO("    Total callbacks queued: %d", queued);
}

EOS_DECLARE_FUNC(void) EOS_Auth_RemoveNotifyLoginStatusChanged(EOS_HAuth Handle, EOS_NotificationId InId) {
    EOS_LOG_INFO(">>> EOS_Auth_RemoveNotifyLoginStatusChanged CALLED");
}
