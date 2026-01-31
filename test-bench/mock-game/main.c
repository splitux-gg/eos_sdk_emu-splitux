/**
 * EOS-LAN Mock Game Client
 *
 * Usage:
 *   ./mock-game --host --name "MySession" --max-players 4
 *   ./mock-game --join
 *   ./mock-game --test-auth
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#ifdef _WIN32
#include <windows.h>
#define usleep(x) Sleep((x) / 1000)
#else
#include <unistd.h>
#endif

#include "eos/eos_sdk.h"
#include "eos/eos_init.h"
#include "eos/eos_common.h"
#include "eos/eos_connect.h"
#include "eos/eos_sessions.h"
#include "eos/eos_p2p.h"

// Configuration
static struct {
    bool is_host;
    bool is_join;
    bool is_both;  // Host AND discover other sessions
    bool test_auth;
    char session_name[256];
    char player_name[64];
    int max_players;
    int verbose;
} g_config = {
    .is_host = false,
    .is_join = false,
    .is_both = false,
    .test_auth = false,
    .session_name = "TestSession",
    .player_name = "Player",
    .max_players = 4,
    .verbose = 1
};

// State
static EOS_HPlatform g_platform = NULL;
static EOS_ProductUserId g_local_user = NULL;
static bool g_running = true;
static bool g_logged_in = false;
static bool g_session_created = false;
static bool g_session_found = false;
static bool g_session_joined = false;
static EOS_HSessionSearch g_search_handle = NULL;

// ============================================================================
// Logging
// ============================================================================

#define LOG(fmt, ...) do { \
    if (g_config.verbose >= 1) \
        printf("[%s] " fmt "\n", __func__, ##__VA_ARGS__); \
} while(0)

#define LOG_DEBUG(fmt, ...) do { \
    if (g_config.verbose >= 2) \
        printf("[%s] DEBUG: " fmt "\n", __func__, ##__VA_ARGS__); \
} while(0)

#define LOG_ERROR(fmt, ...) \
    fprintf(stderr, "[%s] ERROR: " fmt "\n", __func__, ##__VA_ARGS__)

// ============================================================================
// Callbacks
// ============================================================================

void OnLoginComplete(const EOS_Connect_LoginCallbackInfo* Data) {
    if (Data->ResultCode == EOS_Success) {
        g_local_user = Data->LocalUserId;
        g_logged_in = true;

        char id_str[64];
        int32_t len = sizeof(id_str);
        EOS_ProductUserId_ToString(g_local_user, id_str, &len);
        LOG("Login successful! User ID: %s", id_str);
    } else {
        LOG_ERROR("Login failed: %s", EOS_EResult_ToString(Data->ResultCode));
    }
}

void OnSessionCreated(const EOS_Sessions_UpdateSessionCallbackInfo* Data) {
    if (Data->ResultCode == EOS_Success) {
        g_session_created = true;
        LOG("Session created: %s", Data->SessionName);
    } else {
        LOG_ERROR("Session creation failed: %s", EOS_EResult_ToString(Data->ResultCode));
    }
}

void OnSessionSearchComplete(const EOS_SessionSearch_FindCallbackInfo* Data) {
    if (Data->ResultCode == EOS_Success) {
        g_session_found = true;
        LOG("Session search complete");
    } else if (Data->ResultCode == EOS_NotFound) {
        LOG("No sessions found");
    } else {
        LOG_ERROR("Session search failed: %s", EOS_EResult_ToString(Data->ResultCode));
    }
}

void OnSessionJoined(const EOS_Sessions_JoinSessionCallbackInfo* Data) {
    if (Data->ResultCode == EOS_Success) {
        g_session_joined = true;
        LOG("Joined session successfully");
    } else {
        LOG_ERROR("Join session failed: %s", EOS_EResult_ToString(Data->ResultCode));
    }
}

// ============================================================================
// EOS Operations
// ============================================================================

bool init_eos() {
    LOG("Initializing EOS SDK...");

    EOS_InitializeOptions init_opts = {0};
    init_opts.ApiVersion = EOS_INITIALIZE_API_LATEST;
    init_opts.ProductName = "MockGame";
    init_opts.ProductVersion = "1.0.0";

    EOS_EResult result = EOS_Initialize(&init_opts);
    if (result != EOS_Success && result != EOS_AlreadyConfigured) {
        LOG_ERROR("EOS_Initialize failed: %s", EOS_EResult_ToString(result));
        return false;
    }

    EOS_Platform_Options platform_opts = {0};
    platform_opts.ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST;
    platform_opts.ProductId = "mock_product";
    platform_opts.SandboxId = "mock_sandbox";
    platform_opts.DeploymentId = "mock_deployment";
    platform_opts.Flags = EOS_PF_DISABLE_OVERLAY;

    g_platform = EOS_Platform_Create(&platform_opts);
    if (!g_platform) {
        LOG_ERROR("EOS_Platform_Create failed");
        return false;
    }

    LOG("EOS SDK initialized successfully");
    return true;
}

void shutdown_eos() {
    LOG("Shutting down EOS SDK...");

    if (g_search_handle) {
        EOS_SessionSearch_Release(g_search_handle);
        g_search_handle = NULL;
    }

    if (g_platform) {
        EOS_Platform_Release(g_platform);
        g_platform = NULL;
    }

    EOS_Shutdown();
    LOG("EOS SDK shutdown complete");
}

bool login() {
    LOG("Logging in as %s...", g_config.player_name);

    EOS_HConnect connect = EOS_Platform_GetConnectInterface(g_platform);

    EOS_Connect_LoginOptions opts = {0};
    opts.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;

    EOS_Connect_Login(connect, &opts, NULL, OnLoginComplete);

    // Wait for callback
    int timeout = 50;  // 5 seconds
    while (!g_logged_in && timeout-- > 0) {
        EOS_Platform_Tick(g_platform);
        usleep(100000);  // 100ms
    }

    return g_logged_in;
}

bool create_session(const char* name, int max_players) {
    LOG("Creating session '%s' with %d max players...", name, max_players);

    EOS_HSessions sessions = EOS_Platform_GetSessionsInterface(g_platform);

    // Create modification handle
    EOS_Sessions_CreateSessionModificationOptions create_opts = {0};
    create_opts.ApiVersion = EOS_SESSIONS_CREATESESSIONMODIFICATION_API_LATEST;
    create_opts.SessionName = name;
    create_opts.BucketId = "MockGame:Default";
    create_opts.MaxPlayers = max_players;
    create_opts.LocalUserId = g_local_user;
    create_opts.bPresenceEnabled = EOS_TRUE;

    EOS_HSessionModification mod_handle = NULL;
    EOS_EResult result = EOS_Sessions_CreateSessionModification(sessions, &create_opts, &mod_handle);
    if (result != EOS_Success) {
        LOG_ERROR("CreateSessionModification failed: %s", EOS_EResult_ToString(result));
        return false;
    }

    // Set permissions
    EOS_SessionModification_SetPermissionLevelOptions perm_opts = {0};
    perm_opts.ApiVersion = EOS_SESSIONMODIFICATION_SETPERMISSIONLEVEL_API_LATEST;
    perm_opts.PermissionLevel = EOS_OSPF_PublicAdvertised;
    EOS_SessionModification_SetPermissionLevel(mod_handle, &perm_opts);

    // Set join in progress
    EOS_SessionModification_SetJoinInProgressAllowedOptions jip_opts = {0};
    jip_opts.ApiVersion = EOS_SESSIONMODIFICATION_SETJOININPROGRESSALLOWED_API_LATEST;
    jip_opts.bAllowJoinInProgress = EOS_TRUE;
    EOS_SessionModification_SetJoinInProgressAllowed(mod_handle, &jip_opts);

    // Commit
    EOS_Sessions_UpdateSessionOptions update_opts = {0};
    update_opts.ApiVersion = EOS_SESSIONS_UPDATESESSION_API_LATEST;
    update_opts.SessionModificationHandle = mod_handle;

    EOS_Sessions_UpdateSession(sessions, &update_opts, NULL, OnSessionCreated);

    EOS_SessionModification_Release(mod_handle);

    // Wait for callback
    int timeout = 50;
    while (!g_session_created && timeout-- > 0) {
        EOS_Platform_Tick(g_platform);
        usleep(100000);
    }

    return g_session_created;
}

bool search_sessions() {
    LOG("Searching for sessions...");

    EOS_HSessions sessions = EOS_Platform_GetSessionsInterface(g_platform);

    EOS_Sessions_CreateSessionSearchOptions search_create_opts = {0};
    search_create_opts.ApiVersion = EOS_SESSIONS_CREATESESSIONSEARCH_API_LATEST;
    search_create_opts.MaxSearchResults = 10;

    EOS_EResult result = EOS_Sessions_CreateSessionSearch(sessions, &search_create_opts, &g_search_handle);
    if (result != EOS_Success) {
        LOG_ERROR("CreateSessionSearch failed: %s", EOS_EResult_ToString(result));
        return false;
    }

    // Execute search
    EOS_SessionSearch_FindOptions find_opts = {0};
    find_opts.ApiVersion = EOS_SESSIONSEARCH_FIND_API_LATEST;
    find_opts.LocalUserId = g_local_user;

    EOS_SessionSearch_Find(g_search_handle, &find_opts, NULL, OnSessionSearchComplete);

    // Wait for callback
    int timeout = 50;
    while (!g_session_found && timeout-- > 0) {
        EOS_Platform_Tick(g_platform);
        usleep(100000);
    }

    if (g_session_found) {
        // Print results
        EOS_SessionSearch_GetSearchResultCountOptions count_opts = {0};
        count_opts.ApiVersion = EOS_SESSIONSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;
        uint32_t count = EOS_SessionSearch_GetSearchResultCount(g_search_handle, &count_opts);
        LOG("Found %u sessions", count);

        for (uint32_t i = 0; i < count; i++) {
            EOS_HSessionDetails details = NULL;
            EOS_SessionSearch_CopySearchResultByIndexOptions copy_opts = {0};
            copy_opts.ApiVersion = EOS_SESSIONSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
            copy_opts.SessionIndex = i;

            if (EOS_SessionSearch_CopySearchResultByIndex(g_search_handle, &copy_opts, &details) == EOS_Success) {
                EOS_SessionDetails_Info* info = NULL;
                EOS_SessionDetails_CopyInfoOptions info_opts = {0};
                info_opts.ApiVersion = EOS_SESSIONDETAILS_COPYINFO_API_LATEST;

                if (EOS_SessionDetails_CopyInfo(details, &info_opts, &info) == EOS_Success) {
                    LOG("  [%u] Session: %s (Players: %u/%u)",
                        i, info->SessionId ? info->SessionId : "unknown",
                        info->Settings ? (info->Settings->NumPublicConnections - info->NumOpenPublicConnections) : 0,
                        info->Settings ? info->Settings->NumPublicConnections : 0);
                    EOS_SessionDetails_Info_Release(info);
                }
                EOS_SessionDetails_Release(details);
            }
        }
    }

    return g_session_found;
}

bool join_first_session() {
    if (!g_search_handle) {
        LOG_ERROR("No search handle available");
        return false;
    }

    EOS_SessionSearch_GetSearchResultCountOptions count_opts = {0};
    count_opts.ApiVersion = EOS_SESSIONSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;
    uint32_t count = EOS_SessionSearch_GetSearchResultCount(g_search_handle, &count_opts);

    if (count == 0) {
        LOG("No sessions to join");
        return false;
    }

    // Get first session details
    EOS_HSessionDetails details = NULL;
    EOS_SessionSearch_CopySearchResultByIndexOptions copy_opts = {0};
    copy_opts.ApiVersion = EOS_SESSIONSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
    copy_opts.SessionIndex = 0;

    if (EOS_SessionSearch_CopySearchResultByIndex(g_search_handle, &copy_opts, &details) != EOS_Success) {
        LOG_ERROR("Failed to get session details");
        return false;
    }

    LOG("Joining first session...");

    EOS_HSessions sessions = EOS_Platform_GetSessionsInterface(g_platform);
    EOS_Sessions_JoinSessionOptions join_opts = {0};
    join_opts.ApiVersion = EOS_SESSIONS_JOINSESSION_API_LATEST;
    join_opts.SessionHandle = details;
    join_opts.LocalUserId = g_local_user;
    join_opts.SessionName = g_config.session_name;
    join_opts.bPresenceEnabled = EOS_TRUE;

    EOS_Sessions_JoinSession(sessions, &join_opts, NULL, OnSessionJoined);

    // Wait for callback
    int timeout = 50;
    while (!g_session_joined && timeout-- > 0) {
        EOS_Platform_Tick(g_platform);
        usleep(100000);
    }

    EOS_SessionDetails_Release(details);
    return g_session_joined;
}

void run_host_loop() {
    LOG("Running as HOST. Press Ctrl+C to stop.");
    LOG("Session '%s' is now advertised on LAN.", g_config.session_name);

    while (g_running) {
        EOS_Platform_Tick(g_platform);
        usleep(16000);  // ~60 ticks/second
    }
}

void run_join_loop() {
    LOG("Running as CLIENT. Searching for sessions...");

    // Search loop
    int retries = 0;
    while (g_running && !g_session_joined && retries < 5) {
        if (search_sessions() && join_first_session()) {
            break;
        }

        retries++;
        LOG("No sessions found or join failed, retrying in 2 seconds... (attempt %d/5)", retries);
        for (int i = 0; i < 20 && g_running; i++) {
            EOS_Platform_Tick(g_platform);
            usleep(100000);
        }
    }

    if (!g_session_joined) {
        LOG_ERROR("Failed to join any session after 5 attempts");
        g_running = false;
        return;
    }

    // Main loop after joining
    LOG("Successfully joined session, continuing...");
    while (g_running) {
        EOS_Platform_Tick(g_platform);
        usleep(16000);
    }
}

void run_both_loop() {
    LOG("Running as HOST+DISCOVER. Session '%s' advertised, also discovering others...", g_config.session_name);

    int tick_count = 0;
    int search_interval = 100;  // Search every ~1.6 seconds (100 ticks * 16ms)
    int discovered_count = 0;
    int other_sessions = 0;

    while (g_running) {
        EOS_Platform_Tick(g_platform);

        // Periodically search for other sessions
        if (tick_count % search_interval == 0) {
            // Release old search handle
            if (g_search_handle) {
                EOS_SessionSearch_Release(g_search_handle);
                g_search_handle = NULL;
            }
            g_session_found = false;

            EOS_HSessions sessions = EOS_Platform_GetSessionsInterface(g_platform);
            EOS_Sessions_CreateSessionSearchOptions search_create_opts = {0};
            search_create_opts.ApiVersion = EOS_SESSIONS_CREATESESSIONSEARCH_API_LATEST;
            search_create_opts.MaxSearchResults = 10;

            if (EOS_Sessions_CreateSessionSearch(sessions, &search_create_opts, &g_search_handle) == EOS_Success) {
                EOS_SessionSearch_FindOptions find_opts = {0};
                find_opts.ApiVersion = EOS_SESSIONSEARCH_FIND_API_LATEST;
                find_opts.LocalUserId = g_local_user;
                EOS_SessionSearch_Find(g_search_handle, &find_opts, NULL, OnSessionSearchComplete);
            }
        }

        // Check search results
        if (g_session_found && g_search_handle) {
            EOS_SessionSearch_GetSearchResultCountOptions count_opts = {0};
            count_opts.ApiVersion = EOS_SESSIONSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;
            uint32_t count = EOS_SessionSearch_GetSearchResultCount(g_search_handle, &count_opts);

            // Count sessions that aren't ours
            other_sessions = 0;
            for (uint32_t i = 0; i < count; i++) {
                EOS_HSessionDetails details = NULL;
                EOS_SessionSearch_CopySearchResultByIndexOptions copy_opts = {0};
                copy_opts.ApiVersion = EOS_SESSIONSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
                copy_opts.SessionIndex = i;

                if (EOS_SessionSearch_CopySearchResultByIndex(g_search_handle, &copy_opts, &details) == EOS_Success) {
                    EOS_SessionDetails_Info* info = NULL;
                    EOS_SessionDetails_CopyInfoOptions info_opts = {0};
                    info_opts.ApiVersion = EOS_SESSIONDETAILS_COPYINFO_API_LATEST;

                    if (EOS_SessionDetails_CopyInfo(details, &info_opts, &info) == EOS_Success) {
                        // Check if this is a different session (not ours)
                        if (info->Settings && info->Settings->BucketId) {
                            // All sessions from other instances count
                            other_sessions++;
                            if (other_sessions > discovered_count) {
                                LOG("DISCOVERED other session: %s", info->SessionId ? info->SessionId : "unknown");
                                discovered_count = other_sessions;
                            }
                        }
                        EOS_SessionDetails_Info_Release(info);
                    }
                    EOS_SessionDetails_Release(details);
                }
            }

            if (other_sessions > 0 && tick_count % (search_interval * 3) == 0) {
                LOG("Status: Hosting '%s', discovered %d other session(s)", g_config.session_name, other_sessions);
            }

            g_session_found = false;  // Reset for next search
        }

        tick_count++;
        usleep(16000);  // ~60 ticks/second
    }

    LOG("Final: Discovered %d other sessions while hosting '%s'", discovered_count, g_config.session_name);
}

// ============================================================================
// Signal handling
// ============================================================================

void signal_handler(int sig) {
    LOG("\nReceived signal %d, shutting down...", sig);
    g_running = false;
}

// ============================================================================
// Main
// ============================================================================

void print_usage(const char* prog) {
    printf("Usage: %s [options]\n", prog);
    printf("Options:\n");
    printf("  --host              Run as session host\n");
    printf("  --join              Run as session client (search and join)\n");
    printf("  --both              Host a session AND discover others (local co-op sim)\n");
    printf("  --test-auth         Test authentication only\n");
    printf("  --name <name>       Session name (default: TestSession)\n");
    printf("  --player <name>     Player name (default: Player)\n");
    printf("  --max-players <n>   Max players (default: 4)\n");
    printf("  --verbose <level>   Verbosity 0-2 (default: 1)\n");
    printf("  --help              Show this help\n");
}

int main(int argc, char** argv) {
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--host") == 0) {
            g_config.is_host = true;
        } else if (strcmp(argv[i], "--join") == 0) {
            g_config.is_join = true;
        } else if (strcmp(argv[i], "--both") == 0) {
            g_config.is_both = true;
        } else if (strcmp(argv[i], "--test-auth") == 0) {
            g_config.test_auth = true;
        } else if (strcmp(argv[i], "--name") == 0 && i + 1 < argc) {
            strncpy(g_config.session_name, argv[++i], sizeof(g_config.session_name) - 1);
        } else if (strcmp(argv[i], "--player") == 0 && i + 1 < argc) {
            strncpy(g_config.player_name, argv[++i], sizeof(g_config.player_name) - 1);
        } else if (strcmp(argv[i], "--max-players") == 0 && i + 1 < argc) {
            g_config.max_players = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--verbose") == 0 && i + 1 < argc) {
            g_config.verbose = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Initialize
    if (!init_eos()) {
        return 1;
    }

    // Login
    if (!login()) {
        shutdown_eos();
        return 1;
    }

    // Run based on mode
    if (g_config.test_auth) {
        LOG("Auth test complete. User logged in successfully.");
    } else if (g_config.is_both) {
        if (create_session(g_config.session_name, g_config.max_players)) {
            run_both_loop();
        }
    } else if (g_config.is_host) {
        if (create_session(g_config.session_name, g_config.max_players)) {
            run_host_loop();
        }
    } else if (g_config.is_join) {
        run_join_loop();
    } else {
        LOG("No mode specified. Use --host, --join, --both, or --test-auth");
        print_usage(argv[0]);
    }

    // Cleanup
    shutdown_eos();

    LOG("Mock game exited cleanly.");
    return 0;
}
