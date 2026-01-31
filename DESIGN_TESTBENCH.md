# Test Bench Implementation Guide

## Scope
You are implementing the **Test Bench** - the fast feedback loop infrastructure.

**Your responsibility:**
- Mock game client that exercises EOS APIs
- Test scripts for single and dual instance testing
- Logging infrastructure for debugging
- Comparison harness against real SDK
- Build system (Makefile/CMake)

**NOT your responsibility:**
- Implementing EOS APIs (that's the other modules)
- Actual game logic

**This module enables rapid iteration and is CRITICAL for the project.**

---

## File Locations

**You will create:**
- `test-bench/mock-game/main.c` - Mock game client
- `test-bench/mock-game/Makefile` - Build the mock game
- `test-bench/scripts/test-single.sh` - Single instance test
- `test-bench/scripts/test-dual.sh` - Dual instance test
- `test-bench/scripts/test-compare.sh` - Compare against real SDK
- `src/logging.c` - Logging implementation
- `src/logging.h` - Logging API
- `Makefile` - Main build file
- `CMakeLists.txt` - Alternative CMake build

---

## Component 1: Mock Game Client

### Purpose
A minimal program that exercises the EOS API exactly like Palworld (or other games) would. This lets us test without needing to launch the actual game.

### main.c

```c
/**
 * EOS-LAN Mock Game Client
 *
 * Usage:
 *   ./mock-game --host --name "MySession" --max-players 4
 *   ./mock-game --join
 *   ./mock-game --test-auth
 *   ./mock-game --test-p2p <peer_address>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

#include "eos_sdk.h"
#include "eos_connect.h"
#include "eos_sessions.h"
#include "eos_p2p.h"

// Configuration
static struct {
    bool is_host;
    bool is_join;
    bool test_auth;
    bool test_p2p;
    char session_name[256];
    char player_name[64];
    char peer_address[64];
    int max_players;
    int verbose;
} g_config = {
    .is_host = false,
    .is_join = false,
    .test_auth = false,
    .test_p2p = false,
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
    if (Data->ResultCode == EOS_EResult_Success) {
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
    if (Data->ResultCode == EOS_EResult_Success) {
        g_session_created = true;
        LOG("Session created: %s (ID: %s)", Data->SessionName, Data->SessionId);
    } else {
        LOG_ERROR("Session creation failed: %s", EOS_EResult_ToString(Data->ResultCode));
    }
}

void OnSessionSearchComplete(const EOS_SessionSearch_FindCallbackInfo* Data) {
    if (Data->ResultCode == EOS_EResult_Success) {
        g_session_found = true;
        LOG("Session search complete");
    } else if (Data->ResultCode == EOS_EResult_NotFound) {
        LOG("No sessions found");
    } else {
        LOG_ERROR("Session search failed: %s", EOS_EResult_ToString(Data->ResultCode));
    }
}

void OnSessionJoined(const EOS_Sessions_JoinSessionCallbackInfo* Data) {
    if (Data->ResultCode == EOS_EResult_Success) {
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

    EOS_InitializeOptions init_opts = {
        .ApiVersion = EOS_INITIALIZE_API_LATEST,
        .ProductName = "MockGame",
        .ProductVersion = "1.0.0"
    };

    EOS_EResult result = EOS_Initialize(&init_opts);
    if (result != EOS_EResult_Success && result != EOS_EResult_AlreadyConfigured) {
        LOG_ERROR("EOS_Initialize failed: %s", EOS_EResult_ToString(result));
        return false;
    }

    EOS_Platform_Options platform_opts = {
        .ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST,
        .ProductId = "mock_product",
        .SandboxId = "mock_sandbox",
        .DeploymentId = "mock_deployment",
        .Flags = EOS_PF_DISABLE_OVERLAY
    };

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

    EOS_Connect_LoginOptions opts = {
        .ApiVersion = EOS_CONNECT_LOGIN_API_LATEST
    };

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
    EOS_Sessions_CreateSessionModificationOptions create_opts = {
        .ApiVersion = EOS_SESSIONS_CREATESESSIONMODIFICATION_API_LATEST,
        .SessionName = name,
        .BucketId = "MockGame:Default",
        .MaxPlayers = max_players,
        .LocalUserId = g_local_user,
        .bPresenceEnabled = EOS_TRUE
    };

    EOS_HSessionModification mod_handle;
    EOS_EResult result = EOS_Sessions_CreateSessionModification(sessions, &create_opts, &mod_handle);
    if (result != EOS_EResult_Success) {
        LOG_ERROR("CreateSessionModification failed: %s", EOS_EResult_ToString(result));
        return false;
    }

    // Set permissions
    EOS_SessionModification_SetPermissionLevelOptions perm_opts = {
        .ApiVersion = EOS_SESSIONMODIFICATION_SETPERMISSIONLEVEL_API_LATEST,
        .PermissionLevel = EOS_OSPF_PublicAdvertised
    };
    EOS_SessionModification_SetPermissionLevel(mod_handle, &perm_opts);

    // Set join in progress
    EOS_SessionModification_SetJoinInProgressAllowedOptions jip_opts = {
        .ApiVersion = EOS_SESSIONMODIFICATION_SETJOININPROGRESSALLOWED_API_LATEST,
        .bAllowJoinInProgress = EOS_TRUE
    };
    EOS_SessionModification_SetJoinInProgressAllowed(mod_handle, &jip_opts);

    // Commit
    EOS_Sessions_UpdateSessionOptions update_opts = {
        .ApiVersion = EOS_SESSIONS_UPDATESESSION_API_LATEST,
        .SessionModificationHandle = mod_handle
    };

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

    EOS_Sessions_CreateSessionSearchOptions search_create_opts = {
        .ApiVersion = EOS_SESSIONS_CREATESESSIONSEARCH_API_LATEST,
        .MaxSearchResults = 10
    };

    EOS_HSessionSearch search_handle;
    EOS_EResult result = EOS_Sessions_CreateSessionSearch(sessions, &search_create_opts, &search_handle);
    if (result != EOS_EResult_Success) {
        LOG_ERROR("CreateSessionSearch failed: %s", EOS_EResult_ToString(result));
        return false;
    }

    // Execute search
    EOS_SessionSearch_FindOptions find_opts = {
        .ApiVersion = EOS_SESSIONSEARCH_FIND_API_LATEST,
        .LocalUserId = g_local_user
    };

    EOS_SessionSearch_Find(search_handle, &find_opts, NULL, OnSessionSearchComplete);

    // Wait for callback
    int timeout = 50;
    while (!g_session_found && timeout-- > 0) {
        EOS_Platform_Tick(g_platform);
        usleep(100000);
    }

    if (g_session_found) {
        // Print results
        EOS_SessionSearch_GetSearchResultCountOptions count_opts = {
            .ApiVersion = EOS_SESSIONSEARCH_GETSEARCHRESULTCOUNT_API_LATEST
        };
        uint32_t count = EOS_SessionSearch_GetSearchResultCount(search_handle, &count_opts);
        LOG("Found %u sessions:", count);

        for (uint32_t i = 0; i < count; i++) {
            EOS_HSessionDetails details;
            EOS_SessionSearch_CopySearchResultByIndexOptions copy_opts = {
                .ApiVersion = EOS_SESSIONSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST,
                .SessionIndex = i
            };

            if (EOS_SessionSearch_CopySearchResultByIndex(search_handle, &copy_opts, &details) == EOS_EResult_Success) {
                EOS_SessionDetails_Info* info;
                EOS_SessionDetails_CopyInfoOptions info_opts = {
                    .ApiVersion = EOS_SESSIONDETAILS_COPYINFO_API_LATEST
                };

                if (EOS_SessionDetails_CopyInfo(details, &info_opts, &info) == EOS_EResult_Success) {
                    LOG("  [%u] %s (ID: %s, Players: %u/%u)",
                        i, info->SessionId, info->SessionId,
                        info->Settings->NumPublicConnections - info->NumOpenPublicConnections,
                        info->Settings->NumPublicConnections);
                    EOS_SessionDetails_Info_Release(info);
                }
                EOS_SessionDetails_Release(details);
            }
        }
    }

    EOS_SessionSearch_Release(search_handle);
    return g_session_found;
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
    while (g_running && !g_session_joined) {
        if (search_sessions()) {
            // Found sessions, try to join first one
            // (Implementation would go here)
            break;
        }

        LOG("No sessions found, retrying in 2 seconds...");
        for (int i = 0; i < 20 && g_running; i++) {
            EOS_Platform_Tick(g_platform);
            usleep(100000);
        }
    }

    // Main loop after joining
    while (g_running) {
        EOS_Platform_Tick(g_platform);
        usleep(16000);
    }
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
    } else if (g_config.is_host) {
        if (create_session(g_config.session_name, g_config.max_players)) {
            run_host_loop();
        }
    } else if (g_config.is_join) {
        run_join_loop();
    } else {
        LOG("No mode specified. Use --host, --join, or --test-auth");
        print_usage(argv[0]);
    }

    // Cleanup
    shutdown_eos();

    LOG("Mock game exited cleanly.");
    return 0;
}
```

---

## Component 2: Test Scripts

### test-single.sh

```bash
#!/bin/bash
# Test single instance - basic API validation

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"
BUILD_DIR="$PROJECT_DIR/build"
MOCK_GAME="$BUILD_DIR/mock-game"

echo "=== EOS-LAN Single Instance Test ==="
echo "Project: $PROJECT_DIR"
echo ""

# Build if needed
if [ ! -f "$MOCK_GAME" ]; then
    echo "Building mock game..."
    cd "$PROJECT_DIR" && make mock-game
fi

# Test 1: Auth only
echo "--- Test 1: Authentication ---"
$MOCK_GAME --test-auth --verbose 2
echo "PASS: Authentication works"
echo ""

# Test 2: Create session
echo "--- Test 2: Session Creation ---"
timeout 5 $MOCK_GAME --host --name "SingleTest" --verbose 2 &
HOST_PID=$!
sleep 2
kill $HOST_PID 2>/dev/null || true
wait $HOST_PID 2>/dev/null || true
echo "PASS: Session creation works"
echo ""

echo "=== All single instance tests passed ==="
```

### test-dual.sh

```bash
#!/bin/bash
# Test dual instance - LAN discovery and join

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"
BUILD_DIR="$PROJECT_DIR/build"
MOCK_GAME="$BUILD_DIR/mock-game"
LOG_DIR="$PROJECT_DIR/test-bench/logs"

mkdir -p "$LOG_DIR"

echo "=== EOS-LAN Dual Instance Test ==="
echo "Project: $PROJECT_DIR"
echo ""

# Build if needed
if [ ! -f "$MOCK_GAME" ]; then
    echo "Building mock game..."
    cd "$PROJECT_DIR" && make mock-game
fi

cleanup() {
    echo "Cleaning up..."
    kill $HOST_PID 2>/dev/null || true
    kill $CLIENT_PID 2>/dev/null || true
}
trap cleanup EXIT

# Start host
echo "--- Starting host ---"
$MOCK_GAME --host --name "DualTest" --verbose 2 > "$LOG_DIR/host.log" 2>&1 &
HOST_PID=$!
echo "Host PID: $HOST_PID"

# Wait for host to initialize
sleep 3

# Start client
echo "--- Starting client ---"
$MOCK_GAME --join --verbose 2 > "$LOG_DIR/client.log" 2>&1 &
CLIENT_PID=$!
echo "Client PID: $CLIENT_PID"

# Wait for client to find session
sleep 5

# Check results
echo ""
echo "--- Checking results ---"

if grep -q "Found 1 sessions" "$LOG_DIR/client.log"; then
    echo "PASS: Client found host's session"
else
    echo "FAIL: Client did not find session"
    echo "Host log:"
    cat "$LOG_DIR/host.log"
    echo ""
    echo "Client log:"
    cat "$LOG_DIR/client.log"
    exit 1
fi

echo ""
echo "=== Dual instance test passed ==="
echo "Logs available in: $LOG_DIR"
```

### test-compare.sh

```bash
#!/bin/bash
# Compare behavior against real EOS SDK

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"
LOG_DIR="$PROJECT_DIR/test-bench/logs"
REAL_SDK="$PROJECT_DIR/SDK/Bin/libEOSSDK-Linux-Shipping.so"
OUR_SDK="$PROJECT_DIR/build/libEOSSDK-Linux-Shipping.so"

echo "=== EOS-LAN vs Real SDK Comparison ==="

# Test with our SDK
echo "Testing with EOS-LAN..."
LD_PRELOAD="$OUR_SDK" $PROJECT_DIR/build/mock-game --test-auth --verbose 2 > "$LOG_DIR/our-sdk.log" 2>&1

# Note: Real SDK will fail without real credentials, but we can compare API call patterns
echo "Testing with real SDK (will fail auth, but useful for call pattern comparison)..."
LD_PRELOAD="$REAL_SDK" $PROJECT_DIR/build/mock-game --test-auth --verbose 2 > "$LOG_DIR/real-sdk.log" 2>&1 || true

echo ""
echo "Comparison complete. Check logs in: $LOG_DIR"
echo "  - our-sdk.log: Our implementation"
echo "  - real-sdk.log: Real EOS SDK"
```

---

## Component 3: Logging Infrastructure

### logging.h

```c
#ifndef EOS_LAN_LOGGING_H
#define EOS_LAN_LOGGING_H

#include <stdio.h>
#include <stdarg.h>

typedef enum {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
} LogLevel;

/**
 * Initialize logging system.
 * @param level Minimum level to log
 * @param file Optional file to write to (NULL = stderr only)
 */
void log_init(LogLevel level, const char* file);

/**
 * Shutdown logging system.
 */
void log_shutdown(void);

/**
 * Set log level at runtime.
 */
void log_set_level(LogLevel level);

/**
 * Log a message.
 */
void log_write(LogLevel level, const char* func, const char* fmt, ...);

// Convenience macros
#define EOS_LOG_ERROR(fmt, ...) log_write(LOG_LEVEL_ERROR, __func__, fmt, ##__VA_ARGS__)
#define EOS_LOG_WARN(fmt, ...)  log_write(LOG_LEVEL_WARN, __func__, fmt, ##__VA_ARGS__)
#define EOS_LOG_INFO(fmt, ...)  log_write(LOG_LEVEL_INFO, __func__, fmt, ##__VA_ARGS__)
#define EOS_LOG_DEBUG(fmt, ...) log_write(LOG_LEVEL_DEBUG, __func__, fmt, ##__VA_ARGS__)
#define EOS_LOG_TRACE(fmt, ...) log_write(LOG_LEVEL_TRACE, __func__, fmt, ##__VA_ARGS__)

// API call logging (logs function entry/exit with params)
#define EOS_LOG_API_ENTER() EOS_LOG_TRACE("ENTER")
#define EOS_LOG_API_RETURN(result) do { \
    EOS_LOG_TRACE("RETURN %s", EOS_EResult_ToString(result)); \
    return result; \
} while(0)

#endif // EOS_LAN_LOGGING_H
```

### logging.c

```c
#include "logging.h"
#include <time.h>
#include <string.h>
#include <pthread.h>

static LogLevel g_log_level = LOG_LEVEL_INFO;
static FILE* g_log_file = NULL;
static pthread_mutex_t g_log_mutex = PTHREAD_MUTEX_INITIALIZER;

static const char* level_names[] = {
    "NONE", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"
};

void log_init(LogLevel level, const char* file) {
    g_log_level = level;

    if (file && strlen(file) > 0) {
        g_log_file = fopen(file, "a");
    }
}

void log_shutdown(void) {
    if (g_log_file) {
        fclose(g_log_file);
        g_log_file = NULL;
    }
}

void log_set_level(LogLevel level) {
    g_log_level = level;
}

void log_write(LogLevel level, const char* func, const char* fmt, ...) {
    if (level > g_log_level) return;

    pthread_mutex_lock(&g_log_mutex);

    // Get timestamp
    time_t now = time(NULL);
    struct tm* tm = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm);

    // Format message
    char message[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    // Output format: [TIME] [LEVEL] [FUNC] message
    char output[2200];
    snprintf(output, sizeof(output), "[%s] [%-5s] [%s] %s\n",
             timestamp, level_names[level], func, message);

    // Write to stderr
    fputs(output, stderr);

    // Write to file if configured
    if (g_log_file) {
        fputs(output, g_log_file);
        fflush(g_log_file);
    }

    pthread_mutex_unlock(&g_log_mutex);
}
```

---

## Component 4: Build System

### Makefile

```makefile
# EOS-LAN Emulator Build

CC = gcc
CFLAGS = -Wall -Wextra -fPIC -I include -I src
LDFLAGS = -shared
LIBS = -lpthread

BUILD_DIR = build
SRC_DIR = src
TEST_DIR = test-bench

# Source files
SOURCES = \
    $(SRC_DIR)/platform.c \
    $(SRC_DIR)/callbacks.c \
    $(SRC_DIR)/connect.c \
    $(SRC_DIR)/sessions.c \
    $(SRC_DIR)/session_modification.c \
    $(SRC_DIR)/session_search.c \
    $(SRC_DIR)/session_details.c \
    $(SRC_DIR)/p2p.c \
    $(SRC_DIR)/lan_common.c \
    $(SRC_DIR)/lan_discovery.c \
    $(SRC_DIR)/lan_p2p.c \
    $(SRC_DIR)/logging.c \
    $(SRC_DIR)/stubs.c

OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Output library
LIB_NAME = libEOSSDK-Linux-Shipping.so
DLL_NAME = EOSSDK-Win64-Shipping.dll

.PHONY: all clean mock-game test test-single test-dual

all: $(BUILD_DIR)/$(LIB_NAME) mock-game

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(LIB_NAME): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

# Mock game
mock-game: $(BUILD_DIR)/mock-game

$(BUILD_DIR)/mock-game: $(TEST_DIR)/mock-game/main.c $(BUILD_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -L$(BUILD_DIR) -Wl,-rpath,'$$ORIGIN' \
		$< -o $@ -lEOSSDK-Linux-Shipping $(LIBS)

# Tests
test: test-single test-dual

test-single:
	$(TEST_DIR)/scripts/test-single.sh

test-dual:
	$(TEST_DIR)/scripts/test-dual.sh

# Windows cross-compile (requires mingw)
windows: CC = x86_64-w64-mingw32-gcc
windows: LDFLAGS = -shared -static-libgcc
windows: $(BUILD_DIR)/$(DLL_NAME)

$(BUILD_DIR)/$(DLL_NAME): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ -lws2_32

clean:
	rm -rf $(BUILD_DIR)

# Copy official headers
setup-headers:
	mkdir -p include/eos
	cp SDK/Include/*.h include/eos/
	cp SDK/Include/Linux/*.h include/eos/
```

---

## Testing Criteria

1. **Mock game builds and runs:**
   ```bash
   make mock-game
   ./build/mock-game --test-auth
   # Should output login success
   ```

2. **Single instance test passes:**
   ```bash
   ./test-bench/scripts/test-single.sh
   # Should show all tests passing
   ```

3. **Dual instance test passes:**
   ```bash
   ./test-bench/scripts/test-dual.sh
   # Should show client finding host's session
   ```

4. **Logging works:**
   ```bash
   EOS_LOG_LEVEL=5 ./build/mock-game --test-auth
   # Should show TRACE level logs
   ```

---

## Build Instructions

```bash
# Setup (one time)
make setup-headers

# Build everything
make

# Run tests
make test

# Clean
make clean
```

---

## Directory Structure After Setup

```
eos-lan-emu/
├── Makefile
├── include/
│   └── eos/              # Official EOS headers (copied)
│       ├── eos_sdk.h
│       ├── eos_types.h
│       └── ...
├── src/
│   ├── platform.c
│   ├── callbacks.c
│   ├── connect.c
│   ├── sessions.c
│   ├── p2p.c
│   ├── lan_common.c
│   ├── lan_discovery.c
│   ├── lan_p2p.c
│   ├── logging.c
│   └── stubs.c
├── test-bench/
│   ├── mock-game/
│   │   └── main.c
│   ├── scripts/
│   │   ├── test-single.sh
│   │   ├── test-dual.sh
│   │   └── test-compare.sh
│   └── logs/
└── build/
    ├── libEOSSDK-Linux-Shipping.so
    └── mock-game
```
