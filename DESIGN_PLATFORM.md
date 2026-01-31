# Platform Layer Implementation Guide

## Scope
You are implementing the **Platform Layer** of an EOS SDK emulator for LAN multiplayer.

**Your responsibility:**
- SDK initialization and shutdown
- Platform instance lifecycle
- Interface handle management
- Tick loop (drives callback processing)
- Network/application status tracking

**NOT your responsibility:**
- Callback queue implementation (provided by callbacks module)
- Actual subsystem logic (Connect, Sessions, P2P)
- Network I/O

---

## File Locations

**You will create:**
- `src/platform.c` - Implementation
- `src/platform_internal.h` - Internal structures (shared with other modules)

**You will use:**
- `include/eos/*.h` - Official EOS SDK headers (copy from SDK/Include/)
- `src/callbacks.h` - Callback queue interface (from callbacks module)

---

## API Functions to Implement

### Core Lifecycle

```c
/**
 * Initialize the EOS SDK globally. Must be called once before any other EOS function.
 *
 * Implementation:
 * - Validate Options is not NULL
 * - Validate ProductName and ProductVersion are not NULL/empty
 * - Set g_sdk_initialized = true
 * - Store product name/version for later use
 * - Return EOS_Success
 *
 * Error cases:
 * - EOS_InvalidParameters if Options is NULL or required fields missing
 * - EOS_AlreadyConfigured if already initialized
 */
EOS_EResult EOS_Initialize(const EOS_InitializeOptions* Options);

/**
 * Shutdown the EOS SDK. No EOS calls allowed after this.
 *
 * Implementation:
 * - Check g_sdk_initialized is true
 * - Release all platform instances
 * - Set g_sdk_initialized = false
 * - Return EOS_Success
 *
 * Error cases:
 * - EOS_NotConfigured if not initialized
 * - EOS_UnexpectedError if already shut down
 */
EOS_EResult EOS_Shutdown();

/**
 * Create a platform instance. This is the main handle used for all EOS operations.
 *
 * Implementation:
 * - Validate Options is not NULL
 * - Allocate PlatformState structure
 * - Copy relevant options (ProductId, SandboxId, DeploymentId, bIsServer, Flags)
 * - Initialize subsystem handles to NULL (they're created lazily)
 * - Initialize callback queue
 * - Set network_status to EOS_NS_Online (LAN always online)
 * - Set app_status to EOS_AS_Foreground
 * - Add to global platform list
 * - Return opaque handle (cast PlatformState* to EOS_HPlatform)
 *
 * Error cases:
 * - Return NULL if Options is NULL
 * - Return NULL if allocation fails
 */
EOS_HPlatform EOS_Platform_Create(const EOS_Platform_Options* Options);

/**
 * Release a platform instance.
 *
 * Implementation:
 * - Validate Handle is not NULL
 * - Shutdown all subsystems (if initialized)
 * - Free callback queue
 * - Remove from global platform list
 * - Free PlatformState
 */
void EOS_Platform_Release(EOS_HPlatform Handle);

/**
 * Process pending operations. Must be called regularly (every frame).
 *
 * Implementation:
 * - Validate Handle
 * - Call callback_queue_process() to fire pending callbacks
 * - Poll network subsystems if initialized (discovery, p2p)
 * - Respect TickBudgetInMilliseconds if set (optional for v1)
 */
void EOS_Platform_Tick(EOS_HPlatform Handle);
```

### Interface Getters

```c
/**
 * Get handle to Connect interface.
 *
 * Implementation:
 * - If platform->connect is NULL, create it (lazy init)
 * - Return platform->connect cast to EOS_HConnect
 */
EOS_HConnect EOS_Platform_GetConnectInterface(EOS_HPlatform Handle);

/**
 * Get handle to Sessions interface.
 */
EOS_HSessions EOS_Platform_GetSessionsInterface(EOS_HPlatform Handle);

/**
 * Get handle to P2P interface.
 */
EOS_HP2P EOS_Platform_GetP2PInterface(EOS_HPlatform Handle);

/**
 * Get handle to Auth interface.
 * Note: For LAN emulator, this returns a dummy handle. Auth is not fully implemented.
 */
EOS_HAuth EOS_Platform_GetAuthInterface(EOS_HPlatform Handle);

// The following return dummy handles (not implemented for LAN):
EOS_HMetrics EOS_Platform_GetMetricsInterface(EOS_HPlatform Handle);
EOS_HEcom EOS_Platform_GetEcomInterface(EOS_HPlatform Handle);
EOS_HUI EOS_Platform_GetUIInterface(EOS_HPlatform Handle);
EOS_HFriends EOS_Platform_GetFriendsInterface(EOS_HPlatform Handle);
EOS_HPresence EOS_Platform_GetPresenceInterface(EOS_HPlatform Handle);
EOS_HUserInfo EOS_Platform_GetUserInfoInterface(EOS_HPlatform Handle);
EOS_HRTC EOS_Platform_GetRTCInterface(EOS_HPlatform Handle);
EOS_HRTCAdmin EOS_Platform_GetRTCAdminInterface(EOS_HPlatform Handle);
EOS_HPlayerDataStorage EOS_Platform_GetPlayerDataStorageInterface(EOS_HPlatform Handle);
EOS_HTitleStorage EOS_Platform_GetTitleStorageInterface(EOS_HPlatform Handle);
EOS_HAchievements EOS_Platform_GetAchievementsInterface(EOS_HPlatform Handle);
EOS_HStats EOS_Platform_GetStatsInterface(EOS_HPlatform Handle);
EOS_HLeaderboards EOS_Platform_GetLeaderboardsInterface(EOS_HPlatform Handle);
EOS_HMods EOS_Platform_GetModsInterface(EOS_HPlatform Handle);
EOS_HAntiCheatClient EOS_Platform_GetAntiCheatClientInterface(EOS_HPlatform Handle);
EOS_HAntiCheatServer EOS_Platform_GetAntiCheatServerInterface(EOS_HPlatform Handle);
EOS_HProgressionSnapshot EOS_Platform_GetProgressionSnapshotInterface(EOS_HPlatform Handle);
EOS_HReports EOS_Platform_GetReportsInterface(EOS_HPlatform Handle);
EOS_HSanctions EOS_Platform_GetSanctionsInterface(EOS_HPlatform Handle);
EOS_HKWS EOS_Platform_GetKWSInterface(EOS_HPlatform Handle);
EOS_HCustomInvites EOS_Platform_GetCustomInvitesInterface(EOS_HPlatform Handle);
EOS_HLobby EOS_Platform_GetLobbyInterface(EOS_HPlatform Handle);
EOS_HIntegratedPlatform EOS_Platform_GetIntegratedPlatformInterface(EOS_HPlatform Handle);
```

### Status Functions

```c
/**
 * Set network status.
 * For LAN emulator, this is stored but doesn't affect behavior (always online).
 */
EOS_EResult EOS_Platform_SetNetworkStatus(EOS_HPlatform Handle, EOS_ENetworkStatus NewStatus);
EOS_ENetworkStatus EOS_Platform_GetNetworkStatus(EOS_HPlatform Handle);

EOS_EResult EOS_Platform_SetApplicationStatus(EOS_HPlatform Handle, EOS_EApplicationStatus NewStatus);
EOS_EApplicationStatus EOS_Platform_GetApplicationStatus(EOS_HPlatform Handle);
```

### Utility Functions

```c
/**
 * Convert result code to string.
 */
const char* EOS_EResult_ToString(EOS_EResult Result);

/**
 * Check if result indicates operation is complete.
 */
EOS_Bool EOS_EResult_IsOperationComplete(EOS_EResult Result);

/**
 * These always return EOS_NoChange for LAN emulator (no Epic launcher).
 */
EOS_EResult EOS_Platform_CheckForLauncherAndRestart(EOS_HPlatform Handle);
EOS_EResult EOS_Platform_GetDesktopCrossplayStatus(EOS_HPlatform Handle, ...);

/**
 * Country/locale - return stored values or EOS_NotFound.
 */
EOS_EResult EOS_Platform_GetActiveCountryCode(EOS_HPlatform Handle, ...);
EOS_EResult EOS_Platform_GetActiveLocaleCode(EOS_HPlatform Handle, ...);
EOS_EResult EOS_Platform_GetOverrideCountryCode(EOS_HPlatform Handle, ...);
EOS_EResult EOS_Platform_GetOverrideLocaleCode(EOS_HPlatform Handle, ...);
EOS_EResult EOS_Platform_SetOverrideCountryCode(EOS_HPlatform Handle, const char* Code);
EOS_EResult EOS_Platform_SetOverrideLocaleCode(EOS_HPlatform Handle, const char* Code);
```

---

## Data Structures

### platform_internal.h

```c
#ifndef EOS_LAN_PLATFORM_INTERNAL_H
#define EOS_LAN_PLATFORM_INTERNAL_H

#include <stdbool.h>
#include <stdint.h>

// Forward declarations for subsystem states
typedef struct ConnectState ConnectState;
typedef struct SessionsState SessionsState;
typedef struct P2PState P2PState;
typedef struct CallbackQueue CallbackQueue;
typedef struct DiscoveryService DiscoveryService;

#define MAX_PLATFORMS 4
#define EOS_LAN_VERSION "0.1.0"

typedef struct PlatformState {
    // Identification
    uint32_t magic;  // 0x454F534C = "EOSL"
    int index;       // Index in g_platforms array

    // Configuration (copied from EOS_Platform_Options)
    char product_id[65];
    char sandbox_id[65];
    char deployment_id[65];
    bool is_server;
    uint64_t flags;
    uint32_t tick_budget_ms;

    // Locale overrides
    char override_country_code[5];
    char override_locale_code[10];

    // Status
    EOS_ENetworkStatus network_status;
    EOS_EApplicationStatus app_status;

    // Subsystem handles (lazy initialized)
    ConnectState* connect;
    SessionsState* sessions;
    P2PState* p2p;

    // Shared network services
    DiscoveryService* discovery;

    // Callback processing
    CallbackQueue* callbacks;

} PlatformState;

// Global state
extern bool g_sdk_initialized;
extern char g_product_name[65];
extern char g_product_version[65];
extern PlatformState* g_platforms[MAX_PLATFORMS];
extern int g_platform_count;

// Validation helpers
static inline bool platform_is_valid(EOS_HPlatform handle) {
    PlatformState* p = (PlatformState*)handle;
    return p != NULL && p->magic == 0x454F534C;
}

#endif // EOS_LAN_PLATFORM_INTERNAL_H
```

---

## Implementation Notes

### Handle Validation
All functions receiving `EOS_HPlatform` should validate:
```c
if (!platform_is_valid(Handle)) {
    return EOS_InvalidParameters;  // or appropriate error
}
PlatformState* platform = (PlatformState*)Handle;
```

### Lazy Initialization of Subsystems
When `EOS_Platform_GetConnectInterface` is called:
```c
EOS_HConnect EOS_Platform_GetConnectInterface(EOS_HPlatform Handle) {
    if (!platform_is_valid(Handle)) return NULL;
    PlatformState* platform = (PlatformState*)Handle;

    if (platform->connect == NULL) {
        platform->connect = connect_create(platform);
    }
    return (EOS_HConnect)platform->connect;
}
```

### Tick Loop
```c
void EOS_Platform_Tick(EOS_HPlatform Handle) {
    if (!platform_is_valid(Handle)) return;
    PlatformState* platform = (PlatformState*)Handle;

    // Process pending callbacks
    if (platform->callbacks) {
        callback_queue_process(platform->callbacks);
    }

    // Poll network services
    if (platform->discovery) {
        discovery_poll(platform->discovery);
    }

    // Let subsystems do their tick work
    if (platform->sessions) {
        sessions_tick(platform->sessions);
    }
    if (platform->p2p) {
        p2p_tick(platform->p2p);
    }
}
```

### Thread Safety
For v1, assume single-threaded. All EOS calls from same thread.
Document this limitation.

---

## Integration Points

### What You Provide to Other Modules
```c
// Other modules can access platform state
PlatformState* platform_from_handle(EOS_HPlatform handle);

// Other modules use this to queue callbacks
CallbackQueue* platform_get_callback_queue(PlatformState* platform);

// Other modules access shared network services
DiscoveryService* platform_get_discovery(PlatformState* platform);
```

### What You Need from Other Modules

**From callbacks module:**
```c
CallbackQueue* callback_queue_create(void);
void callback_queue_destroy(CallbackQueue* q);
void callback_queue_process(CallbackQueue* q);
```

**From connect module:**
```c
ConnectState* connect_create(PlatformState* platform);
void connect_destroy(ConnectState* state);
```

**From sessions module:**
```c
SessionsState* sessions_create(PlatformState* platform);
void sessions_destroy(SessionsState* state);
void sessions_tick(SessionsState* state);
```

**From p2p module:**
```c
P2PState* p2p_create(PlatformState* platform);
void p2p_destroy(P2PState* state);
void p2p_tick(P2PState* state);
```

**From network module:**
```c
DiscoveryService* discovery_create(uint16_t port);
void discovery_destroy(DiscoveryService* ds);
void discovery_poll(DiscoveryService* ds);
```

---

## Testing Criteria

Your implementation is complete when:

1. **Initialization test:**
   ```c
   EOS_InitializeOptions opts = { .ApiVersion = EOS_INITIALIZE_API_LATEST };
   opts.ProductName = "TestGame";
   opts.ProductVersion = "1.0.0";
   assert(EOS_Initialize(&opts) == EOS_Success);
   assert(EOS_Initialize(&opts) == EOS_AlreadyConfigured);
   ```

2. **Platform creation test:**
   ```c
   EOS_Platform_Options opts = { .ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST };
   opts.ProductId = "test123";
   opts.SandboxId = "sandbox123";
   opts.DeploymentId = "deploy123";
   EOS_HPlatform platform = EOS_Platform_Create(&opts);
   assert(platform != NULL);
   ```

3. **Interface getter test:**
   ```c
   EOS_HConnect connect = EOS_Platform_GetConnectInterface(platform);
   assert(connect != NULL);
   EOS_HConnect connect2 = EOS_Platform_GetConnectInterface(platform);
   assert(connect == connect2);  // Same instance
   ```

4. **Tick test:**
   ```c
   // Should not crash, should process callbacks
   for (int i = 0; i < 100; i++) {
       EOS_Platform_Tick(platform);
   }
   ```

5. **Shutdown test:**
   ```c
   EOS_Platform_Release(platform);
   assert(EOS_Shutdown() == EOS_Success);
   assert(EOS_Shutdown() == EOS_UnexpectedError);
   ```

---

## Build Instructions

```makefile
# Compile platform.c
$(BUILD_DIR)/platform.o: src/platform.c src/platform_internal.h
	$(CC) $(CFLAGS) -I include -c $< -o $@
```

Dependencies:
- Standard C library
- EOS SDK headers (include/eos/)
- callbacks.h (from callbacks module)

---

## Logging

Use the logging macros (defined in logging.h):
```c
#include "logging.h"

EOS_EResult EOS_Initialize(const EOS_InitializeOptions* Options) {
    EOS_LOG_TRACE("EOS_Initialize called");

    if (Options == NULL) {
        EOS_LOG_ERROR("EOS_Initialize: Options is NULL");
        return EOS_InvalidParameters;
    }

    EOS_LOG_INFO("Initializing EOS-LAN v%s for %s", EOS_LAN_VERSION, Options->ProductName);
    // ...

    EOS_LOG_DEBUG("EOS_Initialize returning EOS_Success");
    return EOS_Success;
}
```
