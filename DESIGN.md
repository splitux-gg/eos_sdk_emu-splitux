# EOS-LAN Emulator Design Document

## Project Goal
Build a clean-room EOS SDK replacement focused on LAN multiplayer for split-screen gaming.
Target: Palworld as first game, generalizable to other EOS games.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                         Game (Palworld)                         │
├─────────────────────────────────────────────────────────────────┤
│                     EOS-LAN Emulator DLL                        │
│  ┌───────────┐  ┌───────────┐  ┌───────────┐  ┌───────────┐    │
│  │  Platform │  │  Connect  │  │ Sessions  │  │    P2P    │    │
│  │   Layer   │  │   Layer   │  │   Layer   │  │   Layer   │    │
│  └─────┬─────┘  └─────┬─────┘  └─────┬─────┘  └─────┬─────┘    │
│        │              │              │              │          │
│  ┌─────┴──────────────┴──────────────┴──────────────┴─────┐    │
│  │                    Callback System                      │    │
│  └─────────────────────────┬───────────────────────────────┘    │
│                            │                                    │
│  ┌─────────────────────────┴───────────────────────────────┐    │
│  │                   LAN Network Layer                      │    │
│  │  ┌──────────────┐  ┌───────────────┐  ┌──────────────┐  │    │
│  │  │   Discovery  │  │   Session     │  │   Direct     │  │    │
│  │  │  (UDP Bcast) │  │   Registry    │  │   P2P UDP    │  │    │
│  │  └──────────────┘  └───────────────┘  └──────────────┘  │    │
│  └──────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────┘
```

## Subsystems

### 1. Platform Layer
**Owner**: Can be implemented independently
**Complexity**: Low
**Dependencies**: None

#### Functions to Implement
```c
// Core lifecycle
EOS_EResult EOS_Initialize(const EOS_InitializeOptions* Options);
EOS_EResult EOS_Shutdown();
EOS_HPlatform EOS_Platform_Create(const EOS_Platform_Options* Options);
void EOS_Platform_Release(EOS_HPlatform Handle);
void EOS_Platform_Tick(EOS_HPlatform Handle);

// Interface getters (return handles to other subsystems)
EOS_HConnect EOS_Platform_GetConnectInterface(EOS_HPlatform Handle);
EOS_HSessions EOS_Platform_GetSessionsInterface(EOS_HPlatform Handle);
EOS_HP2P EOS_Platform_GetP2PInterface(EOS_HPlatform Handle);
EOS_HAuth EOS_Platform_GetAuthInterface(EOS_HPlatform Handle);
// ... other interfaces return dummy handles

// Status functions
EOS_EResult EOS_Platform_SetNetworkStatus(EOS_HPlatform Handle, EOS_ENetworkStatus Status);
EOS_ENetworkStatus EOS_Platform_GetNetworkStatus(EOS_HPlatform Handle);
EOS_EResult EOS_Platform_SetApplicationStatus(EOS_HPlatform Handle, EOS_EApplicationStatus Status);
EOS_EApplicationStatus EOS_Platform_GetApplicationStatus(EOS_HPlatform Handle);
```

#### Internal State
```c
typedef struct {
    bool initialized;
    EOS_Platform_Options options;
    EOS_ENetworkStatus network_status;
    EOS_EApplicationStatus app_status;

    // Subsystem handles
    ConnectState* connect;
    SessionsState* sessions;
    P2PState* p2p;

    // Callback queue
    CallbackQueue* callbacks;
} PlatformState;

// Global state
static bool g_sdk_initialized = false;
static PlatformState* g_platforms[MAX_PLATFORMS];
```

#### Behavior
- `EOS_Initialize`: Set global flag, store product name/version
- `EOS_Platform_Create`: Allocate PlatformState, initialize subsystems
- `EOS_Platform_Tick`: Process callback queue, call pending callbacks
- `EOS_Platform_Release`: Clean up all subsystem state

---

### 2. Connect Layer (Fake Auth)
**Owner**: Can be implemented independently
**Complexity**: Low
**Dependencies**: Platform Layer

#### Functions to Implement
```c
// Core authentication
void EOS_Connect_Login(EOS_HConnect Handle, const EOS_Connect_LoginOptions* Options,
                       void* ClientData, const EOS_Connect_OnLoginCallback Callback);
void EOS_Connect_Logout(EOS_HConnect Handle, const EOS_Connect_LogoutOptions* Options,
                        void* ClientData, const EOS_Connect_OnLogoutCallback Callback);

// User ID management
EOS_ProductUserId EOS_Connect_GetLoggedInUserByIndex(EOS_HConnect Handle, int32_t Index);
int32_t EOS_Connect_GetLoggedInUsersCount(EOS_HConnect Handle);
EOS_ELoginStatus EOS_Connect_GetLoginStatus(EOS_HConnect Handle, EOS_ProductUserId UserId);

// Product User ID utilities
EOS_Bool EOS_ProductUserId_IsValid(EOS_ProductUserId AccountId);
EOS_EResult EOS_ProductUserId_ToString(EOS_ProductUserId AccountId, char* OutBuffer, int32_t* InOutBufferLength);
EOS_ProductUserId EOS_ProductUserId_FromString(const char* ProductUserIdString);
```

#### Internal State
```c
typedef struct {
    char user_id_string[33];  // 32 chars + null
    EOS_ProductUserId handle;
    EOS_ELoginStatus status;
} LoggedInUser;

typedef struct {
    LoggedInUser users[MAX_LOCAL_USERS];  // Usually 1-4 for split-screen
    int user_count;
    char instance_id[33];  // Unique per-instance identifier
} ConnectState;

// ProductUserId is just a pointer to a user_id_string
typedef struct EOS_ProductUserIdDetails {
    char id[33];
} EOS_ProductUserIdDetails;
```

#### Behavior
- `EOS_Connect_Login`:
  1. Generate deterministic user ID from username/config file OR
  2. Generate random user ID if not configured
  3. Format: `EOSLAN_<instance_id>_<user_index>` (32 chars, hex encoded)
  4. Queue callback with EOS_Success

- User ID format for LAN:
  - Must be unique across instances
  - Should be deterministic from config (for consistent identity)
  - Example: `EOSLAN_A1B2C3D4E5F6_0001`

---

### 3. Sessions Layer (Core Problem)
**Owner**: Most complex, needs careful design
**Complexity**: High
**Dependencies**: Platform, Connect, LAN Network Layer

#### Functions to Implement
```c
// Session creation
EOS_EResult EOS_Sessions_CreateSessionModification(EOS_HSessions Handle,
    const EOS_Sessions_CreateSessionModificationOptions* Options,
    EOS_HSessionModification* OutHandle);
void EOS_Sessions_UpdateSession(EOS_HSessions Handle,
    const EOS_Sessions_UpdateSessionOptions* Options,
    void* ClientData, const EOS_Sessions_OnUpdateSessionCallback Callback);
void EOS_Sessions_DestroySession(EOS_HSessions Handle,
    const EOS_Sessions_DestroySessionOptions* Options,
    void* ClientData, const EOS_Sessions_OnDestroySessionCallback Callback);

// Session modification
EOS_EResult EOS_SessionModification_SetBucketId(EOS_HSessionModification Handle, ...);
EOS_EResult EOS_SessionModification_SetHostAddress(EOS_HSessionModification Handle, ...);
EOS_EResult EOS_SessionModification_SetMaxPlayers(EOS_HSessionModification Handle, ...);
EOS_EResult EOS_SessionModification_SetPermissionLevel(EOS_HSessionModification Handle, ...);
EOS_EResult EOS_SessionModification_SetJoinInProgressAllowed(EOS_HSessionModification Handle, ...);
EOS_EResult EOS_SessionModification_AddAttribute(EOS_HSessionModification Handle, ...);
void EOS_SessionModification_Release(EOS_HSessionModification Handle);

// Session search
EOS_EResult EOS_Sessions_CreateSessionSearch(EOS_HSessions Handle,
    const EOS_Sessions_CreateSessionSearchOptions* Options,
    EOS_HSessionSearch* OutHandle);
void EOS_SessionSearch_Find(EOS_HSessionSearch Handle,
    const EOS_SessionSearch_FindOptions* Options,
    void* ClientData, const EOS_SessionSearch_OnFindCallback Callback);
uint32_t EOS_SessionSearch_GetSearchResultCount(EOS_HSessionSearch Handle, ...);
EOS_EResult EOS_SessionSearch_CopySearchResultByIndex(EOS_HSessionSearch Handle, ...);
EOS_EResult EOS_SessionSearch_SetParameter(EOS_HSessionSearch Handle, ...);
void EOS_SessionSearch_Release(EOS_HSessionSearch Handle);

// Session join
void EOS_Sessions_JoinSession(EOS_HSessions Handle,
    const EOS_Sessions_JoinSessionOptions* Options,
    void* ClientData, const EOS_Sessions_OnJoinSessionCallback Callback);
void EOS_Sessions_StartSession(EOS_HSessions Handle, ...);
void EOS_Sessions_EndSession(EOS_HSessions Handle, ...);

// Session details
EOS_EResult EOS_SessionDetails_CopyInfo(EOS_HSessionDetails Handle, ...);
uint32_t EOS_SessionDetails_GetSessionAttributeCount(EOS_HSessionDetails Handle, ...);
EOS_EResult EOS_SessionDetails_CopySessionAttributeByIndex(EOS_HSessionDetails Handle, ...);
void EOS_SessionDetails_Release(EOS_HSessionDetails Handle);

// Active session
EOS_EResult EOS_Sessions_CopyActiveSessionHandle(EOS_HSessions Handle, ...);
EOS_EResult EOS_ActiveSession_CopyInfo(EOS_HActiveSession Handle, ...);
void EOS_ActiveSession_Release(EOS_HActiveSession Handle);

// Notifications
EOS_NotificationId EOS_Sessions_AddNotifySessionInviteReceived(EOS_HSessions Handle, ...);
EOS_NotificationId EOS_Sessions_AddNotifyJoinSessionAccepted(EOS_HSessions Handle, ...);
void EOS_Sessions_RemoveNotifySessionInviteReceived(EOS_HSessions Handle, EOS_NotificationId Id);
void EOS_Sessions_RemoveNotifyJoinSessionAccepted(EOS_HSessions Handle, EOS_NotificationId Id);
```

#### Internal State
```c
typedef struct {
    char key[65];
    EOS_ESessionAttributeType type;
    union {
        EOS_Bool as_bool;
        int64_t as_int64;
        double as_double;
        char as_string[256];
    } value;
    EOS_ESessionAttributeAdvertisementType advertisement;
} SessionAttribute;

typedef struct {
    char session_id[65];
    char session_name[256];
    char bucket_id[256];
    char host_address[256];       // LAN: IP:port
    EOS_ProductUserId owner_id;
    uint32_t max_players;
    uint32_t current_players;
    EOS_EOnlineSessionPermissionLevel permission_level;
    EOS_EOnlineSessionState state;
    bool join_in_progress_allowed;
    bool invites_allowed;
    bool presence_enabled;

    SessionAttribute attributes[64];
    int attribute_count;

    EOS_ProductUserId registered_players[64];
    int registered_player_count;
} Session;

typedef struct {
    // Local sessions we've created or joined
    Session local_sessions[8];
    int local_session_count;

    // Discovered sessions from LAN
    Session discovered_sessions[32];
    int discovered_session_count;
    uint64_t last_discovery_time;

    // Pending searches
    SessionSearch* active_searches[4];
    int active_search_count;

    // Notification handlers
    NotificationHandler invite_handlers[8];
    NotificationHandler join_handlers[8];
} SessionsState;
```

#### LAN Session Protocol
```
Session Announcement (UDP broadcast, port 23456):
┌────────────────────────────────────────────┐
│ Magic: "EOSLAN" (6 bytes)                  │
│ Version: uint16                            │
│ Message Type: uint8 (ANNOUNCE=1, QUERY=2)  │
│ Session ID: 64 bytes                       │
│ Session Name: 256 bytes                    │
│ Bucket ID: 256 bytes                       │
│ Host Address: 64 bytes (IP:port)           │
│ Owner ID: 33 bytes                         │
│ Max Players: uint32                        │
│ Current Players: uint32                    │
│ State: uint8                               │
│ Flags: uint8 (join_in_progress, public)    │
│ Attribute Count: uint16                    │
│ Attributes: [key, type, value]...          │
│ Checksum: uint32                           │
└────────────────────────────────────────────┘

Session Query (UDP broadcast):
┌────────────────────────────────────────────┐
│ Magic: "EOSLAN"                            │
│ Version: uint16                            │
│ Message Type: QUERY                        │
│ Requester ID: 33 bytes                     │
│ Bucket ID Filter: 256 bytes (optional)     │
└────────────────────────────────────────────┘
```

#### Behavior

**Session Creation Flow:**
1. `EOS_Sessions_CreateSessionModification` - allocate modification handle
2. `EOS_SessionModification_Set*` - configure session properties
3. `EOS_Sessions_UpdateSession` - commit session:
   - Generate session ID if not provided
   - Store in local_sessions[]
   - Start broadcasting session announcements (every 2 seconds)
   - Queue success callback

**Session Search Flow:**
1. `EOS_Sessions_CreateSessionSearch` - allocate search handle
2. `EOS_SessionSearch_SetParameter` - configure filters
3. `EOS_SessionSearch_Find`:
   - Send QUERY broadcast
   - Wait 500ms for responses (or use cached discoveries)
   - Filter results based on parameters
   - Queue callback with result count
4. `EOS_SessionSearch_CopySearchResultByIndex` - get session details

**Session Join Flow:**
1. `EOS_Sessions_JoinSession`:
   - Connect to host via host_address
   - Send join request
   - Host validates and adds to registered players
   - Store in local_sessions[]
   - Queue success callback

---

### 4. P2P Layer
**Owner**: Can be implemented independently
**Complexity**: Medium
**Dependencies**: Platform, Connect, LAN Network Layer

#### Functions to Implement
```c
// Packet transmission
EOS_EResult EOS_P2P_SendPacket(EOS_HP2P Handle, const EOS_P2P_SendPacketOptions* Options);
EOS_EResult EOS_P2P_GetNextReceivedPacketSize(EOS_HP2P Handle,
    const EOS_P2P_GetNextReceivedPacketSizeOptions* Options, uint32_t* OutPacketSize);
EOS_EResult EOS_P2P_ReceivePacket(EOS_HP2P Handle,
    const EOS_P2P_ReceivePacketOptions* Options,
    EOS_ProductUserId* OutPeerId, EOS_P2P_SocketId* OutSocketId,
    uint8_t* OutChannel, void* OutData, uint32_t* OutBytesWritten);

// Connection management
EOS_EResult EOS_P2P_AcceptConnection(EOS_HP2P Handle, const EOS_P2P_AcceptConnectionOptions* Options);
EOS_EResult EOS_P2P_CloseConnection(EOS_HP2P Handle, const EOS_P2P_CloseConnectionOptions* Options);
EOS_EResult EOS_P2P_CloseConnections(EOS_HP2P Handle, const EOS_P2P_CloseConnectionsOptions* Options);

// Notifications
EOS_NotificationId EOS_P2P_AddNotifyPeerConnectionRequest(EOS_HP2P Handle, ...);
EOS_NotificationId EOS_P2P_AddNotifyPeerConnectionEstablished(EOS_HP2P Handle, ...);
EOS_NotificationId EOS_P2P_AddNotifyPeerConnectionClosed(EOS_HP2P Handle, ...);
void EOS_P2P_RemoveNotifyPeerConnectionRequest(EOS_HP2P Handle, EOS_NotificationId Id);
// etc.

// NAT type (fake it for LAN)
EOS_EResult EOS_P2P_QueryNATType(EOS_HP2P Handle, ...);
EOS_EResult EOS_P2P_GetNATType(EOS_HP2P Handle, ...);
```

#### Internal State
```c
typedef struct {
    uint8_t data[4096];
    uint32_t size;
    EOS_ProductUserId sender;
    EOS_P2P_SocketId socket_id;
    uint8_t channel;
} ReceivedPacket;

typedef struct {
    EOS_ProductUserId peer_id;
    char peer_address[64];  // IP:port
    int socket_fd;
    bool connected;
    uint64_t last_activity;
} PeerConnection;

typedef struct {
    // Connection tracking
    PeerConnection connections[32];
    int connection_count;

    // Receive queue
    ReceivedPacket recv_queue[256];
    int recv_queue_head;
    int recv_queue_tail;

    // Listening socket
    int listen_socket;
    uint16_t listen_port;

    // Auto-accept settings
    bool auto_accept_connections;
    EOS_P2P_SocketId accepted_sockets[16];
    int accepted_socket_count;

    // Notification handlers
    NotificationHandler connection_request_handlers[8];
    NotificationHandler connection_established_handlers[8];
    NotificationHandler connection_closed_handlers[8];
} P2PState;
```

#### LAN P2P Protocol
```
P2P Packet Header:
┌────────────────────────────────────────────┐
│ Magic: "EOSP2P" (6 bytes)                  │
│ Sender ID: 33 bytes                        │
│ Socket Name: 32 bytes                      │
│ Channel: uint8                             │
│ Flags: uint8                               │
│ Sequence: uint32                           │
│ Payload Length: uint32                     │
│ Payload: [bytes...]                        │
└────────────────────────────────────────────┘
```

#### Behavior
- For LAN: Direct UDP to known peer address
- No NAT traversal needed (same network)
- Address resolution via session host_address
- Reliability: Optional (EOS has reliable channel flag)

---

### 5. Callback System
**Owner**: Shared infrastructure
**Complexity**: Medium
**Dependencies**: None (used by all)

#### Design
```c
typedef struct {
    void* callback_fn;
    void* callback_info;
    size_t callback_info_size;
    void* client_data;
    uint64_t execute_at;  // 0 = next tick
} PendingCallback;

typedef struct {
    PendingCallback queue[1024];
    int head;
    int tail;
    pthread_mutex_t mutex;
} CallbackQueue;

// API
void callback_queue_init(CallbackQueue* q);
void callback_queue_push(CallbackQueue* q, void* fn, void* info, size_t size, void* client_data);
void callback_queue_push_delayed(CallbackQueue* q, void* fn, void* info, size_t size,
                                  void* client_data, uint64_t delay_ms);
void callback_queue_process(CallbackQueue* q);  // Called from EOS_Platform_Tick
```

---

### 6. LAN Network Layer
**Owner**: Shared infrastructure
**Complexity**: Medium
**Dependencies**: None (used by Sessions, P2P)

#### Components

**Discovery Service:**
```c
typedef struct {
    int broadcast_socket;
    int listen_socket;
    uint16_t port;  // 23456

    // Received announcements cache
    struct {
        Session session;
        uint64_t received_at;
        char source_ip[16];
    } cache[64];
    int cache_count;
} DiscoveryService;

// API
void discovery_init(DiscoveryService* ds, uint16_t port);
void discovery_shutdown(DiscoveryService* ds);
void discovery_broadcast_session(DiscoveryService* ds, const Session* session);
void discovery_send_query(DiscoveryService* ds, const char* bucket_filter);
void discovery_poll(DiscoveryService* ds);  // Process incoming announcements
Session* discovery_get_sessions(DiscoveryService* ds, int* count);
```

**P2P Socket Manager:**
```c
typedef struct {
    int udp_socket;
    uint16_t port;  // Dynamic or configured

    // Address book (peer_id -> address)
    struct {
        EOS_ProductUserId peer_id;
        struct sockaddr_in addr;
    } peers[64];
    int peer_count;
} P2PSocketManager;

// API
void p2p_socket_init(P2PSocketManager* mgr, uint16_t port);
void p2p_socket_shutdown(P2PSocketManager* mgr);
void p2p_socket_register_peer(P2PSocketManager* mgr, EOS_ProductUserId peer, const char* address);
int p2p_socket_send(P2PSocketManager* mgr, EOS_ProductUserId peer, const void* data, size_t len);
int p2p_socket_recv(P2PSocketManager* mgr, EOS_ProductUserId* out_peer, void* data, size_t max_len);
```

---

### 7. Test Bench

#### Components

**Mock Game Client:**
```c
// test-bench/mock-game/main.c
// Exercises EOS API like a real game would

int main(int argc, char** argv) {
    // Parse args: --host or --join, --name "PlayerName"

    // 1. Initialize
    EOS_Initialize(...);
    EOS_HPlatform platform = EOS_Platform_Create(...);

    // 2. Login
    EOS_Connect_Login(...);
    wait_for_callback();

    // 3. Host or Join
    if (is_host) {
        create_session("TestSession", 4);
        wait_for_callback();
        printf("Session created, waiting for players...\n");
    } else {
        search_sessions();
        wait_for_callback();
        if (found_sessions > 0) {
            join_session(0);
            wait_for_callback();
        }
    }

    // 4. P2P test
    send_test_packets();
    receive_test_packets();

    // 5. Cleanup
    EOS_Platform_Release(platform);
    EOS_Shutdown();
}
```

**Test Scripts:**
```bash
# scripts/test-single.sh
# Run single instance to test basic API calls

# scripts/test-dual.sh
# Run two instances: one host, one client
# Verify session discovery and P2P communication

# scripts/test-compare.sh
# Run same test against real EOS SDK and our emulator
# Diff the logs for compatibility
```

**Logging Infrastructure:**
```c
// Every EOS API call logs:
// [TIMESTAMP] [FUNCTION] params... -> result

#define EOS_LOG(fmt, ...) \
    fprintf(g_log_file, "[%s] " fmt "\n", __func__, ##__VA_ARGS__)
```

---

## Implementation Phases

### Phase 1: Skeleton + Test Bench
- [ ] Project structure with Makefile/CMake
- [ ] All EOS functions as stubs returning EOS_Success
- [ ] Mock game client that exercises API
- [ ] Basic logging infrastructure

### Phase 2: Platform + Connect
- [ ] Platform lifecycle (init, create, tick, release)
- [ ] Callback queue processing
- [ ] Connect fake login with user ID generation
- [ ] Verify mock game can "login"

### Phase 3: Sessions (Local Only)
- [ ] Session creation and storage
- [ ] Session modification
- [ ] Session search (returns own sessions only)
- [ ] Session join (local only)
- [ ] Verify mock game host flow works

### Phase 4: LAN Discovery
- [ ] UDP broadcast infrastructure
- [ ] Session announcement protocol
- [ ] Session discovery protocol
- [ ] Dual-instance test working

### Phase 5: P2P
- [ ] Direct UDP between peers
- [ ] Packet queue and delivery
- [ ] Connection lifecycle
- [ ] P2P test in dual instance

### Phase 6: Palworld Integration
- [ ] Test with actual Palworld
- [ ] Fix any missing/broken APIs
- [ ] Handle game-specific quirks
- [ ] Document configuration

---

## File Structure

```
eos-lan-emu/
├── CMakeLists.txt
├── Makefile
├── include/
│   └── eos/                    # Copy of official headers (for compatibility)
├── src/
│   ├── platform.c              # Platform layer
│   ├── connect.c               # Connect layer
│   ├── sessions.c              # Sessions layer
│   ├── p2p.c                   # P2P layer
│   ├── callbacks.c             # Callback system
│   ├── lan_discovery.c         # LAN discovery service
│   ├── lan_p2p.c               # LAN P2P socket manager
│   ├── logging.c               # Logging infrastructure
│   └── stubs.c                 # All unimplemented functions
├── test-bench/
│   ├── mock-game/
│   │   ├── main.c
│   │   └── Makefile
│   ├── scripts/
│   │   ├── test-single.sh
│   │   ├── test-dual.sh
│   │   └── test-compare.sh
│   └── logs/
├── reference/
│   ├── sdk/                    # Official EOS SDK
│   └── nemirtingas/            # Nemirtingas DLL for comparison
└── docs/
    ├── DESIGN.md               # This file
    ├── API_COVERAGE.md         # Which functions are implemented
    └── PROTOCOL.md             # LAN protocol specification
```

---

## Configuration

```ini
# eos-lan.ini (next to DLL or in user config dir)

[identity]
# Deterministic user ID base (hex, 16 chars)
# If not set, random per-launch
instance_id = A1B2C3D4E5F6A1B2

# Display name
username = Player1

[network]
# LAN discovery port
discovery_port = 23456

# P2P base port (will try sequential if busy)
p2p_port = 23457

# Broadcast address (default: 255.255.255.255)
broadcast_addr = 255.255.255.255

[logging]
# Log file path (empty = no file logging)
log_file = eos-lan.log

# Log level: 0=none, 1=errors, 2=warnings, 3=info, 4=debug, 5=trace
log_level = 4
```

---

## Parallel Implementation Strategy

Each subsystem can be developed in a separate git worktree:

```bash
# Main branch has skeleton with stubs
git worktree add ../eos-lan-platform platform-impl
git worktree add ../eos-lan-connect connect-impl
git worktree add ../eos-lan-sessions sessions-impl
git worktree add ../eos-lan-p2p p2p-impl
git worktree add ../eos-lan-network network-impl
git worktree add ../eos-lan-testbench testbench-impl
```

Merge order:
1. testbench (can run against stubs)
2. platform + callbacks
3. connect
4. network layer
5. sessions
6. p2p

---

## Open Questions

1. **Session ID format**: Should we use a specific format for LAN session IDs to distinguish from real EOS?
2. **P2P reliability**: Does Palworld use reliable channels? Need to test.
3. **Multiple games**: How to isolate sessions between different games on same LAN?
4. **IPv6**: Support needed?
5. **Wine compatibility**: Any special considerations for running on Linux via Wine?

---

## Development Workflow

**See [DEVELOPMENT.md](DEVELOPMENT.md) for complete documentation on:**

### Windows VM Build Environment
- SSH access to Windows 11 VM (192.168.122.100)
- File synchronization between Linux and Windows
- **Native Windows DLL builds using Visual Studio (PRIMARY)**

### Build System
- CMake for Windows DLL builds on VM
- Makefile targets: `make sync-vm`, `make build-vm`, `make deploy-vm`
- **Target: EOSSDK-Win64-Shipping.dll ONLY**
- Linux builds: Not a priority (Windows first, Linux maybe later)

### Git Worktree Strategy
- Parallel development using git worktrees
- One worktree per component/feature
- Sub-agent task assignment via worktrees
- Branch naming: feature/<component>, fix/<issue>

### Testing with Palworld
- DLL deployment to Palworld directory
- Dual-instance testing workflow
- Log locations and debugging procedures

**Quick Start**:
```bash
# On Linux: Sync code to Windows VM and build Windows DLL
make deploy-vm

# On Windows VM: Deploy to Palworld
copy C:\Code\eos-lan\build\Release\EOSSDK-Win64-Shipping.dll "C:\Program Files (x86)\Steam\steamapps\common\Palworld\Pal\Binaries\Win64\"
```

**Note**: All builds produce Windows DLL. This is intentional - the DLL is used by Proton/Gamescope to run Windows games on Linux.
