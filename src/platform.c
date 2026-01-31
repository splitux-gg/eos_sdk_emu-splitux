#include "eos/eos_sdk.h"
#include "eos/eos_init.h"
#include "eos/eos_connect.h"
#include "internal/platform_internal.h"
#include "internal/connect_internal.h"
#include "internal/sessions_internal.h"
#include "internal/p2p_internal.h"
#include "internal/callbacks.h"
#include "internal/logging.h"
#include <stdlib.h>
#include <string.h>

bool g_sdk_initialized = false;
PlatformState* g_platforms[8] = {NULL};

EOS_DECLARE_FUNC(EOS_EResult) EOS_Initialize(const EOS_InitializeOptions* Options) {
    if (!Options || Options->ApiVersion != EOS_INITIALIZE_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (g_sdk_initialized) {
        return EOS_AlreadyConfigured;
    }

    // Initialize logging - use EOSLAN_LOG_PATH env var if set
    const char* log_path = getenv("EOSLAN_LOG_PATH");
    if (!log_path || strlen(log_path) == 0) {
        log_path = "eos-lan.log";
    }
    log_init(LOG_LEVEL_TRACE, log_path);

    g_sdk_initialized = true;
    EOS_LOG_INFO("EOS SDK Initialized: %s v%s",
                 Options->ProductName, Options->ProductVersion);

    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Shutdown() {
    if (!g_sdk_initialized) {
        return EOS_NotConfigured;
    }

    // Clean up all platforms
    for (int i = 0; i < 8; i++) {
        if (g_platforms[i]) {
            EOS_Platform_Release((EOS_HPlatform)g_platforms[i]);
        }
    }

    log_shutdown();
    g_sdk_initialized = false;
    EOS_LOG_INFO("EOS SDK Shutdown");

    return EOS_Success;
}

/**
 * Parse LAN networking configuration from environment variables
 */
static void parse_lan_env_vars(PlatformState* platform) {
    char* env_val;

    // EOSLAN_DISCOVERY_PORT
    if ((env_val = getenv("EOSLAN_DISCOVERY_PORT")) != NULL) {
        int port = atoi(env_val);
        if (port >= 1024 && port <= 65535) {
            platform->lan_config.discovery_port = port;
            EOS_LOG_INFO("Using discovery port from env: %d", port);
        } else {
            EOS_LOG_ERROR("Invalid EOSLAN_DISCOVERY_PORT: %s (must be 1024-65535)", env_val);
        }
    }

    // EOSLAN_BROADCAST_ADDR
    if ((env_val = getenv("EOSLAN_BROADCAST_ADDR")) != NULL) {
        if (strlen(env_val) < sizeof(platform->lan_config.broadcast_address)) {
            strncpy(platform->lan_config.broadcast_address, env_val,
                    sizeof(platform->lan_config.broadcast_address) - 1);
            platform->lan_config.broadcast_address[sizeof(platform->lan_config.broadcast_address) - 1] = '\0';
            EOS_LOG_INFO("Using broadcast address from env: %s", env_val);
        } else {
            EOS_LOG_ERROR("Invalid EOSLAN_BROADCAST_ADDR: too long");
        }
    }

    // EOSLAN_ANNOUNCE_INTERVAL
    if ((env_val = getenv("EOSLAN_ANNOUNCE_INTERVAL")) != NULL) {
        int interval = atoi(env_val);
        if (interval >= 500 && interval <= 10000) {
            platform->lan_config.announcement_interval_ms = interval;
            EOS_LOG_INFO("Using announcement interval from env: %dms", interval);
        } else {
            EOS_LOG_ERROR("Invalid EOSLAN_ANNOUNCE_INTERVAL: %s (must be 500-10000)", env_val);
        }
    }

    // EOSLAN_PREFERRED_IP
    if ((env_val = getenv("EOSLAN_PREFERRED_IP")) != NULL) {
        if (strlen(env_val) < sizeof(platform->lan_config.preferred_local_address)) {
            strncpy(platform->lan_config.preferred_local_address, env_val,
                    sizeof(platform->lan_config.preferred_local_address) - 1);
            platform->lan_config.preferred_local_address[sizeof(platform->lan_config.preferred_local_address) - 1] = '\0';
            EOS_LOG_INFO("Using preferred IP from env: %s", env_val);
        }
    }

    // EOSLAN_DEBUG
    if ((env_val = getenv("EOSLAN_DEBUG")) != NULL) {
        platform->lan_config.enable_debug_logs = (atoi(env_val) != 0);
        if (platform->lan_config.enable_debug_logs) {
            EOS_LOG_INFO("LAN debug logging enabled via env");
        }
    }
}

EOS_DECLARE_FUNC(EOS_HPlatform) EOS_Platform_Create(const EOS_Platform_Options* Options) {
    if (!g_sdk_initialized || !Options) {
        return NULL;
    }

    PlatformState* platform = calloc(1, sizeof(PlatformState));
    if (!platform) {
        return NULL;
    }

    platform->initialized = true;
    platform->network_status = EOS_NS_Online;
    platform->app_status = EOS_AS_Foreground;

    // Initialize LAN config with defaults
    platform->lan_config.discovery_port = 23456;
    strcpy(platform->lan_config.broadcast_address, "255.255.255.255");
    platform->lan_config.announcement_interval_ms = 2000;
    platform->lan_config.preferred_local_address[0] = '\0';
    platform->lan_config.enable_debug_logs = false;

    // Override with environment variables
    parse_lan_env_vars(platform);

    // Initialize callback queue
    platform->callbacks = callback_queue_create();
    if (!platform->callbacks) {
        EOS_LOG_ERROR("Failed to create callback queue");
        free(platform);
        return NULL;
    }

    // Initialize subsystems
    platform->connect = connect_create(platform);
    if (!platform->connect) {
        EOS_LOG_ERROR("Failed to create Connect subsystem");
        free(platform);
        return NULL;
    }

    platform->sessions = sessions_create(platform);
    if (!platform->sessions) {
        EOS_LOG_ERROR("Failed to create Sessions subsystem");
        connect_destroy(platform->connect);
        free(platform);
        return NULL;
    }

    platform->p2p = p2p_create(platform);
    if (!platform->p2p) {
        EOS_LOG_ERROR("Failed to create P2P subsystem");
        sessions_destroy(platform->sessions);
        connect_destroy(platform->connect);
        free(platform);
        return NULL;
    }

    // Store in global array
    for (int i = 0; i < 8; i++) {
        if (!g_platforms[i]) {
            g_platforms[i] = platform;
            break;
        }
    }

    // Auto-login Connect user (but NOT Auth - let Connect_Login trigger Auth login)
    EOS_Connect_LoginOptions login_opts = {0};
    login_opts.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
    EOS_Connect_Login((EOS_HConnect)platform->connect, &login_opts, NULL, NULL);

    EOS_LOG_INFO("Platform created (with auto-login)");
    return (EOS_HPlatform)platform;
}

EOS_DECLARE_FUNC(void) EOS_Platform_Release(EOS_HPlatform Handle) {
    PlatformState* platform = (PlatformState*)Handle;
    if (!platform) return;

    // Destroy subsystems
    if (platform->p2p) {
        p2p_destroy(platform->p2p);
        platform->p2p = NULL;
    }

    if (platform->sessions) {
        sessions_destroy(platform->sessions);
        platform->sessions = NULL;
    }

    if (platform->connect) {
        connect_destroy(platform->connect);
        platform->connect = NULL;
    }

    if (platform->callbacks) {
        callback_queue_destroy(platform->callbacks);
        platform->callbacks = NULL;
    }

    // Remove from global array
    for (int i = 0; i < 8; i++) {
        if (g_platforms[i] == platform) {
            g_platforms[i] = NULL;
            break;
        }
    }

    free(platform);
    EOS_LOG_INFO("Platform released");
}

EOS_DECLARE_FUNC(void) EOS_Platform_Tick(EOS_HPlatform Handle) {
    PlatformState* platform = (PlatformState*)Handle;
    if (!platform || !platform->callbacks) {
        if (!platform) EOS_LOG_ERROR("Platform_Tick: platform is NULL!");
        if (platform && !platform->callbacks) EOS_LOG_ERROR("Platform_Tick: callbacks is NULL!");
        return;
    }

    // Drive LAN discovery (broadcasts sessions, polls for announcements)
    if (platform->sessions) {
        sessions_tick(platform->sessions);
    }

    // Process queued callbacks
    callback_queue_process(platform->callbacks);
}

// Platform interface getters
EOS_DECLARE_FUNC(EOS_HConnect) EOS_Platform_GetConnectInterface(EOS_HPlatform Handle) {
    EOS_LOG_API_ENTER();
    PlatformState* platform = (PlatformState*)Handle;
    if (!platform) {
        EOS_LOG_ERROR("Invalid platform handle");
        return NULL;
    }
    return (EOS_HConnect)platform->connect;
}

EOS_DECLARE_FUNC(EOS_HSessions) EOS_Platform_GetSessionsInterface(EOS_HPlatform Handle) {
    EOS_LOG_API_ENTER();
    PlatformState* platform = (PlatformState*)Handle;
    if (!platform) {
        EOS_LOG_ERROR("Invalid platform handle");
        return NULL;
    }
    return (EOS_HSessions)platform->sessions;
}

EOS_DECLARE_FUNC(EOS_HP2P) EOS_Platform_GetP2PInterface(EOS_HPlatform Handle) {
    EOS_LOG_API_ENTER();
    PlatformState* platform = (PlatformState*)Handle;
    if (!platform) {
        EOS_LOG_ERROR("Invalid platform handle");
        return NULL;
    }
    return (EOS_HP2P)platform->p2p;
}

EOS_DECLARE_FUNC(EOS_HAuth) EOS_Platform_GetAuthInterface(EOS_HPlatform Handle) {
    EOS_LOG_API_ENTER();
    return (EOS_HAuth)Handle;
}

// Status functions
EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_SetNetworkStatus(EOS_HPlatform Handle, EOS_ENetworkStatus Status) {
    PlatformState* platform = (PlatformState*)Handle;
    if (!platform) return EOS_InvalidParameters;

    platform->network_status = Status;
    EOS_LOG_INFO("Network status set to %d", Status);
    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_ENetworkStatus) EOS_Platform_GetNetworkStatus(EOS_HPlatform Handle) {
    PlatformState* platform = (PlatformState*)Handle;
    if (!platform) return EOS_NS_Disabled;

    return platform->network_status;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_SetApplicationStatus(EOS_HPlatform Handle, EOS_EApplicationStatus Status) {
    PlatformState* platform = (PlatformState*)Handle;
    if (!platform) return EOS_InvalidParameters;

    platform->app_status = Status;
    EOS_LOG_INFO("Application status set to %d", Status);
    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EApplicationStatus) EOS_Platform_GetApplicationStatus(EOS_HPlatform Handle) {
    PlatformState* platform = (PlatformState*)Handle;
    if (!platform) return EOS_AS_BackgroundSuspended;

    return platform->app_status;
}

// Utility function to convert EOS_EResult to string
EOS_DECLARE_FUNC(const char*) EOS_EResult_ToString(EOS_EResult Result) {
    switch (Result) {
        case EOS_Success: return "EOS_Success";
        case EOS_NoConnection: return "EOS_NoConnection";
        case EOS_InvalidCredentials: return "EOS_InvalidCredentials";
        case EOS_InvalidUser: return "EOS_InvalidUser";
        case EOS_InvalidAuth: return "EOS_InvalidAuth";
        case EOS_AccessDenied: return "EOS_AccessDenied";
        case EOS_MissingPermissions: return "EOS_MissingPermissions";
        case EOS_Token_Not_Account: return "EOS_Token_Not_Account";
        case EOS_TooManyRequests: return "EOS_TooManyRequests";
        case EOS_AlreadyPending: return "EOS_AlreadyPending";
        case EOS_InvalidParameters: return "EOS_InvalidParameters";
        case EOS_InvalidRequest: return "EOS_InvalidRequest";
        case EOS_UnrecognizedResponse: return "EOS_UnrecognizedResponse";
        case EOS_IncompatibleVersion: return "EOS_IncompatibleVersion";
        case EOS_NotConfigured: return "EOS_NotConfigured";
        case EOS_AlreadyConfigured: return "EOS_AlreadyConfigured";
        case EOS_NotImplemented: return "EOS_NotImplemented";
        case EOS_Canceled: return "EOS_Canceled";
        case EOS_NotFound: return "EOS_NotFound";
        case EOS_OperationWillRetry: return "EOS_OperationWillRetry";
        case EOS_NoChange: return "EOS_NoChange";
        case EOS_VersionMismatch: return "EOS_VersionMismatch";
        case EOS_LimitExceeded: return "EOS_LimitExceeded";
        case EOS_Disabled: return "EOS_Disabled";
        default: return "EOS_UnknownError";
    }
}
