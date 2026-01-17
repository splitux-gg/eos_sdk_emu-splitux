# EOS SDK Emulator - Callback Architecture

**Date:** 2026-01-17

## Overview

The EOS SDK emulator uses a callback system to asynchronously notify game code of events. This document explains the architecture, components, and patterns used throughout the codebase.

---

## Table of Contents

1. [System Components](#system-components)
2. [Callback Types](#callback-types)
3. [Callback Flow](#callback-flow)
4. [Callback Manager](#callback-manager)
5. [Working Patterns](#working-patterns)
6. [Common Mistakes](#common-mistakes)

---

## System Components

### 1. FrameResult

**File:** `eos_dll/callback_manager.h` (lines 18-44)

Core data structure for storing callback information:

```cpp
struct FrameResult
{
    std::atomic<bool> done;        // Async completion flag
    EOS_EResult res;               // Result code
    std::chrono::steady_clock::time_point created_time;
    int32 m_iCallback;             // Callback type identifier

    void* create_callback(int32 callback_id)
    {
        void* ptr = new uint8_t[callback_size(callback_id)];
        m_iCallback = callback_id;
        new (ptr) Common_CallbackData;  // Placement new
        return ptr;
    }

    template<typename T>
    T& CreateCallback(int32 callback_id, void(*fnCallback)(T const*), void* pCallbackContext)
    {
        T* ptr = reinterpret_cast<T*>(create_callback(callback_id));
        ptr->m_iCallback = callback_id;
        ptr->fnCallback = fnCallback;
        ptr->pCallbackContext = pCallbackContext;
        return *ptr;
    }

    void* GetFunc()
    {
        return get_callback_func(m_iCallback);
    }

    void* GetFuncParam()
    {
        return get_callback_param(m_iCallback);
    }
};
```

**Key Fields:**
- `done`: Flag indicating async operation completion
- `res`: EOS result code (success/failure)
- `m_iCallback`: Type identifier (e.g., `EOS_Auth_LoginCallbackInfo::k_iCallback`)
- Callback function pointer and context stored in type-specific structure

---

### 2. Callback Manager

**File:** `eos_dll/callback_manager.h` & `.cpp`

Central management system for all callbacks and notifications:

```cpp
class Callback_Manager
{
    struct notification_info {
        Base_Hook* hook;
        pFrameResult_t res;
    };

    std::map<CallbackId, pFrameResult_t> _callbacks_to_run;
    std::map<NotificationId, notification_info> _notifications;

public:
    // For one-time callbacks (async operations)
    CallbackId add_callback(Base_Hook* hook, pFrameResult_t res);
    void remove_callback(CallbackId id);

    // For persistent notifications (event listeners)
    NotificationId add_notification(Base_Hook* hook, pFrameResult_t res);
    std::vector<pFrameResult_t> get_notifications(Base_Hook* hook, int32 type);
    void remove_notification(NotificationId id);

    // Execute callbacks each frame
    void run_callbacks();
};
```

**Two Storage Maps:**

1. **`_callbacks_to_run`**: One-time callbacks (Login, Query, etc.)
   - Added via `add_callback()`
   - Executed once via `run_callbacks()`
   - Automatically removed after execution

2. **`_notifications`**: Persistent notifications (LoginStatusChanged, etc.)
   - Added via `add_notification()`
   - Retrieved via `get_notifications()` when events occur
   - Remain until explicitly removed

---

### 3. Base_Hook

**File:** `eos_dll/base_hook.h`

Base class for all SDK service implementations:

```cpp
class Base_Hook
{
    friend class Callback_Manager;

protected:
    Callback_Manager& GetCB_Manager() { return *_cb_manager; }

public:
    virtual bool CBRunFrame() { return false; }
    virtual bool RunNetwork(void* callback_msg) { return false; }
};
```

**Key Methods:**
- `CBRunFrame()`: Called every frame for periodic processing
- `RunNetwork()`: Called when network messages arrive
- `GetCB_Manager()`: Access to callback manager

---

## Callback Types

### 1. One-Time Callbacks (Async Operations)

Used for asynchronous operations that complete once:

**Examples:**
- `EOS_Auth_Login()` → `EOS_Auth_LoginCallbackInfo`
- `EOS_Lobby_CreateLobby()` → `EOS_Lobby_CreateLobbyCallbackInfo`
- `EOS_UserInfo_QueryUserInfo()` → `EOS_UserInfo_QueryUserInfoCallbackInfo`

**Flow:**

```cpp
// Game initiates async operation
void Login(const EOS_Auth_LoginOptions* Options, void* ClientData,
           const EOS_Auth_OnLoginCallback CompletionDelegate)
{
    // Create callback result
    pFrameResult_t res(new FrameResult);

    EOS_Auth_LoginCallbackInfo& lci =
        res->CreateCallback<EOS_Auth_LoginCallbackInfo>(
            EOS_Auth_LoginCallbackInfo::k_iCallback,
            CompletionDelegate,
            ClientData
        );

    // Start async work (simulated with delay)
    res->done = false;

    // Delayed completion
    delayed_callback(res, 1000ms, [this, res]() {
        // Populate result
        res->res = EOS_EResult::EOS_Success;
        res->done = true;

        // Add to callback queue for execution
        GetCB_Manager().add_callback(this, res);
    });
}
```

**Execution:**

```cpp
// In game loop (EOS_Platform_Tick)
void run_callbacks()
{
    for (auto& [id, res] : _callbacks_to_run)
    {
        if (res->done)
        {
            // Call the callback function
            res->GetFunc()(res->GetFuncParam());

            // Mark for removal
            to_remove.push_back(id);
        }
    }

    // Remove executed callbacks
    for (auto id : to_remove)
        _callbacks_to_run.erase(id);
}
```

**Characteristics:**
- Single execution
- Automatically removed after calling
- Stored in `_callbacks_to_run`
- Triggered by `run_callbacks()` when `done == true`

---

### 2. Persistent Notifications (Event Listeners)

Used for ongoing event notifications:

**Examples:**
- `EOS_Auth_AddNotifyLoginStatusChanged()` → `EOS_Auth_LoginStatusChangedCallbackInfo`
- `EOS_Lobby_AddNotifyLobbyUpdateReceived()` → `EOS_Lobby_LobbyUpdateReceivedCallbackInfo`
- `EOS_P2P_AddNotifyPeerConnectionRequest()` → `EOS_P2P_OnIncomingConnectionRequestInfo`

**Flow:**

```cpp
// Game registers notification listener
EOS_NotificationId AddNotifyLobbyUpdateReceived(
    const EOS_Lobby_AddNotifyLobbyUpdateReceivedOptions* Options,
    void* ClientData,
    const EOS_Lobby_OnLobbyUpdateReceivedCallback NotificationFn)
{
    // Create notification callback
    pFrameResult_t res(new FrameResult);

    EOS_Lobby_LobbyUpdateReceivedCallbackInfo& luci =
        res->CreateCallback<EOS_Lobby_LobbyUpdateReceivedCallbackInfo>(
            EOS_Lobby_LobbyUpdateReceivedCallbackInfo::k_iCallback,
            NotificationFn,
            ClientData
        );

    luci.ClientData = ClientData;

    // Store notification (NOT in callbacks_to_run!)
    return GetCB_Manager().add_notification(this, res);
}
```

**Trigger (Working Pattern):**

```cpp
// When event occurs (e.g., network message received)
void notify_lobby_update(std::string const& lobby_id)
{
    // Retrieve all registered notifications of this type
    auto notifs = std::move(GetCB_Manager().get_notifications(
        this,
        EOS_Lobby_LobbyUpdateReceivedCallbackInfo::k_iCallback
    ));

    // Fire each registered notification
    for (auto& notif : notifs)
    {
        auto& luci = notif->GetCallback<EOS_Lobby_LobbyUpdateReceivedCallbackInfo>();

        // Populate event-specific data
        luci.LobbyId = lobby_id.c_str();

        // Call the notification callback
        notif->GetFunc()(notif->GetFuncParam());
    }
}
```

**Characteristics:**
- Multiple executions (every time event occurs)
- Persist until removed via `EOS_XXX_RemoveNotify()`
- Stored in `_notifications`
- Triggered by explicit `get_notifications()` + manual firing
- **CRITICAL:** Not automatically executed - service must explicitly fire them

---

## Callback Flow

### Complete Async Operation Flow

```
┌──────────────────────────────────────────────────────────────┐
│ 1. GAME CALLS API                                            │
│    EOS_Auth_Login(options, clientData, callback)             │
└────────────────────────┬─────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────────┐
│ 2. SDK CREATES FRAMERESULT                                   │
│    res = new FrameResult                                     │
│    res->CreateCallback<LoginCallbackInfo>(...)               │
│    res->done = false                                         │
└────────────────────────┬─────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────────┐
│ 3. SDK STARTS ASYNC WORK                                     │
│    - Network request                                         │
│    - Simulated delay                                         │
│    - File I/O                                                │
└────────────────────────┬─────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────────┐
│ 4. WORK COMPLETES (async callback/thread)                    │
│    res->res = EOS_Success                                    │
│    res->done = true                                          │
│    GetCB_Manager().add_callback(this, res)  ◄─ Queued        │
└────────────────────────┬─────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────────┐
│ 5. GAME CALLS EOS_Platform_Tick()                            │
│    → run_callbacks()                                         │
└────────────────────────┬─────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────────┐
│ 6. CALLBACK MANAGER EXECUTES READY CALLBACKS                 │
│    for (auto& res : _callbacks_to_run)                       │
│      if (res->done)                                          │
│        res->GetFunc()(res->GetFuncParam())  ◄─ FIRES         │
└────────────────────────┬─────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────────┐
│ 7. GAME CALLBACK EXECUTES                                    │
│    void OnLoginCallback(const LoginCallbackInfo* data)       │
│    {                                                         │
│        if (data->ResultCode == EOS_Success)                  │
│            // Handle successful login                        │
│    }                                                         │
└────────────────────────┬─────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────────┐
│ 8. CALLBACK REMOVED                                          │
│    _callbacks_to_run.erase(id)                               │
└──────────────────────────────────────────────────────────────┘
```

---

### Complete Notification Flow (WORKING Pattern)

```
┌──────────────────────────────────────────────────────────────┐
│ 1. GAME REGISTERS NOTIFICATION                               │
│    id = AddNotifyLoginStatusChanged(callback, clientData)    │
└────────────────────────┬─────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────────┐
│ 2. SDK STORES NOTIFICATION                                   │
│    res = new FrameResult                                     │
│    res->CreateCallback<LoginStatusChangedCallbackInfo>(...)  │
│    return GetCB_Manager().add_notification(this, res)        │
│                                                              │
│    _notifications[id] = res  ◄─ Stored, waiting for events   │
└──────────────────────────────────────────────────────────────┘
                         │
                         │ (Time passes...)
                         │
                         ▼
┌──────────────────────────────────────────────────────────────┐
│ 3. EVENT OCCURS                                              │
│    - Network message received                                │
│    - State change detected                                   │
│    - Timeout occurred                                        │
└────────────────────────┬─────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────────┐
│ 4. SERVICE TRIGGERS NOTIFICATION                             │
│    trigger_login_status_changed(prev, curr)                  │
└────────────────────────┬─────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────────┐
│ 5. RETRIEVE STORED NOTIFICATIONS                             │
│    auto notifs = GetCB_Manager().get_notifications(          │
│        this, LoginStatusChangedCallbackInfo::k_iCallback     │
│    );                                                        │
└────────────────────────┬─────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────────┐
│ 6. FIRE EACH NOTIFICATION                                    │
│    for (auto& notif : notifs)                                │
│    {                                                         │
│        auto& info = notif->GetCallback<...>();               │
│        info.PreviousStatus = prev;                           │
│        info.CurrentStatus = curr;                            │
│        notif->GetFunc()(notif->GetFuncParam());  ◄─ FIRES    │
│    }                                                         │
└────────────────────────┬─────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────────┐
│ 7. GAME CALLBACK EXECUTES                                    │
│    void OnStatusChanged(const LoginStatusChangedInfo* data)  │
│    {                                                         │
│        // Handle status change                               │
│    }                                                         │
└────────────────────────┬─────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────────┐
│ 8. NOTIFICATION REMAINS ACTIVE                               │
│    _notifications[id] still exists for future events         │
│    (Until RemoveNotify called)                               │
└──────────────────────────────────────────────────────────────┘
```

---

### Broken Notification Flow (CURRENT AUTH STATE)

```
┌──────────────────────────────────────────────────────────────┐
│ 1. GAME REGISTERS NOTIFICATION                               │
│    id = AddNotifyLoginStatusChanged(callback, clientData)    │
└────────────────────────┬─────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────────┐
│ 2. SDK STORES NOTIFICATION                                   │
│    res = new FrameResult                                     │
│    res->CreateCallback<LoginStatusChangedCallbackInfo>(...)  │
│    return GetCB_Manager().add_notification(this, res)        │
│                                                              │
│    _notifications[id] = res  ◄─ Stored                       │
└──────────────────────────────────────────────────────────────┘
                         │
                         │ (Time passes...)
                         │
                         ▼
┌──────────────────────────────────────────────────────────────┐
│ 3. EVENT OCCURS                                              │
│    Login() completes successfully                            │
│    _logged_in = true                                         │
└────────────────────────┬─────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────────┐
│ 4. ❌ NO TRIGGER MECHANISM                                   │
│    (Code doesn't call any notification trigger)              │
└────────────────────────┬─────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────────┐
│ 5. ❌ NOTIFICATIONS NEVER RETRIEVED                          │
│    get_notifications() never called                          │
└────────────────────────┬─────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────────┐
│ 6. ❌ NOTIFICATIONS NEVER FIRED                              │
│    Callbacks stored but never executed                       │
└────────────────────────┬─────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────────┐
│ 7. ❌ GAME WAITS FOREVER                                     │
│    OnStatusChanged() never called                            │
│    Palworld initialization times out                         │
│    → Falls back to OFFLINE MODE                              │
└──────────────────────────────────────────────────────────────┘
```

---

## Callback Manager

### Key Methods

#### add_callback()

**Purpose:** Queue one-time callback for execution

**File:** `callback_manager.cpp` (lines 46-61)

```cpp
CallbackId Callback_Manager::add_callback(Base_Hook* hook, pFrameResult_t res)
{
    std::lock_guard<std::recursive_mutex> lock(_cb_mutex);

    // Generate unique ID
    CallbackId id;
    do {
        id.high = get_random_u64();
        id.low = get_random_u64();
    } while (_callbacks_to_run.count(id));

    // Store in execution queue
    _callbacks_to_run.emplace(id, std::move(res));

    return id;
}
```

---

#### add_notification()

**Purpose:** Register persistent notification listener

**File:** `callback_manager.cpp` (lines 70-91)

```cpp
NotificationId Callback_Manager::add_notification(Base_Hook* hook, pFrameResult_t res)
{
    std::lock_guard<std::recursive_mutex> lock(_cb_mutex);

    // Generate unique ID
    NotificationId id;
    do {
        id.high = get_random_u64();
        id.low = get_random_u64();
    } while (_notifications.count(id));

    // Store with hook association
    notification_info info;
    info.hook = hook;
    info.res = std::move(res);

    _notifications.emplace(id, std::move(info));

    return id;
}
```

---

#### get_notifications()

**Purpose:** Retrieve notifications for specific hook and type

**File:** `callback_manager.cpp` (lines 93-114)

```cpp
std::vector<pFrameResult_t> Callback_Manager::get_notifications(
    Base_Hook* hook,
    int32 notification_type)
{
    std::lock_guard<std::recursive_mutex> lock(_cb_mutex);
    std::vector<pFrameResult_t> notifs;

    // Find all notifications matching hook and type
    for (auto& [id, info] : _notifications)
    {
        if (info.hook == hook &&
            info.res->m_iCallback == notification_type)
        {
            // Return COPY (original stays in _notifications)
            notifs.push_back(info.res);
        }
    }

    return notifs;
}
```

**CRITICAL:** Returns copies, originals persist in map for future firings.

---

#### run_callbacks()

**Purpose:** Execute all ready one-time callbacks

**File:** `callback_manager.cpp` (lines 147-184)

```cpp
void Callback_Manager::run_callbacks()
{
    std::lock_guard<std::recursive_mutex> lock(_cb_mutex);
    std::vector<CallbackId> to_remove;

    // Check each queued callback
    for (auto& [id, res] : _callbacks_to_run)
    {
        if (res->done)
        {
            // Execute callback
            void (*callback_fn)(void*) =
                reinterpret_cast<void(*)(void*)>(res->GetFunc());
            void* callback_param = res->GetFuncParam();

            callback_fn(callback_param);

            // Mark for cleanup
            to_remove.push_back(id);
        }
    }

    // Remove executed callbacks
    for (auto id : to_remove)
    {
        _callbacks_to_run.erase(id);
    }
}
```

**Note:** Only processes `_callbacks_to_run`, NOT `_notifications`. This is why notifications must be manually fired.

---

## Working Patterns

### Pattern 1: Network-Triggered Notifications (Lobby)

**When to use:** Event driven by incoming network messages

**Example:** Lobby updates

```cpp
// eossdk_lobby.cpp

class EOSSDK_Lobby
{
    // Trigger method
    void notify_lobby_update(std::string const& lobby_id)
    {
        auto notifs = std::move(GetCB_Manager().get_notifications(
            this, EOS_Lobby_LobbyUpdateReceivedCallbackInfo::k_iCallback));

        for (auto& notif : notifs)
        {
            auto& luci = notif->GetCallback<...>();
            luci.LobbyId = lobby_id.c_str();
            notif->GetFunc()(notif->GetFuncParam());
        }
    }

    // Network handler
    void on_lobby_updated(Network_Message_pb const& msg)
    {
        // Parse message
        std::string lobby_id = msg.lobby_id();

        // Update local state
        update_lobby_data(lobby_id, msg);

        // Fire notifications
        notify_lobby_update(lobby_id);
    }
};
```

**Trigger Point:** Network message arrival

---

### Pattern 2: State-Change Notifications (Auth - PROPOSED)

**When to use:** Event driven by internal state changes

**Example:** Login status change

```cpp
// eossdk_auth.cpp (PROPOSED FIX)

class EOSSDK_Auth
{
    bool _logged_in = false;

    // Trigger method
    void trigger_login_status_changed(
        EOS_ELoginStatus prev,
        EOS_ELoginStatus curr)
    {
        auto notifs = std::move(GetCB_Manager().get_notifications(
            this, EOS_Auth_LoginStatusChangedCallbackInfo::k_iCallback));

        for (auto& notif : notifs)
        {
            auto& lscci = notif->GetCallback<...>();
            lscci.PreviousStatus = prev;
            lscci.CurrentStatus = curr;
            lscci.LocalUserId = Settings::Inst().userid;
            notif->GetFunc()(notif->GetFuncParam());
        }
    }

    // State change method
    void Login(...)
    {
        // ... async work ...

        // State changes
        bool was_logged_in = _logged_in;
        _logged_in = true;

        // Fire notification
        trigger_login_status_changed(
            was_logged_in ? EOS_LS_LoggedIn : EOS_LS_NotLoggedIn,
            EOS_LS_LoggedIn
        );
    }
};
```

**Trigger Point:** State variable change

---

### Pattern 3: Periodic/Timeout Notifications (P2P)

**When to use:** Events detected through periodic checks

**Example:** P2P connection timeout

```cpp
// eossdk_p2p.cpp

class EOSSDK_P2P
{
    bool CBRunFrame() override
    {
        auto now = std::chrono::steady_clock::now();

        // Check for timed-out connections
        for (auto& [socket_id, connection] : _connections)
        {
            if (connection.last_activity + 30s < now)
            {
                // Fire interrupted notification
                auto notifs = GetCB_Manager().get_notifications(
                    this,
                    EOS_P2P_OnPeerConnectionInterruptedInfo::k_iCallback
                );

                for (auto& notif : notifs)
                {
                    auto& opci = notif->GetCallback<...>();
                    opci.SocketId = &socket_id;
                    opci.LocalUserId = connection.local;
                    opci.RemoteUserId = connection.remote;
                    notif->GetFunc()(notif->GetFuncParam());
                }

                connection.interrupted = true;
            }
        }

        return true;  // Continue processing
    }
};
```

**Trigger Point:** Frame tick / timeout detection

---

### Pattern 4: Hybrid (Sessions)

**When to use:** Combine multiple patterns

**Example:** Session invites (network) + timeouts (periodic)

```cpp
// eossdk_sessions.cpp

class EOSSDK_Sessions
{
    // Network-triggered
    void on_session_invite(Network_Message_pb const& msg)
    {
        auto notifs = GetCB_Manager().get_notifications(...);
        // Fire notifications
    }

    // Periodic cleanup
    bool CBRunFrame() override
    {
        // Clean up expired invites
        auto now = std::chrono::steady_clock::now();

        for (auto it = _invites.begin(); it != _invites.end();)
        {
            if (it->second.expires_at < now)
            {
                it = _invites.erase(it);
            }
            else
            {
                ++it;
            }
        }

        return true;
    }
};
```

---

## Common Mistakes

### Mistake 1: Registering But Not Firing

```cpp
// ❌ WRONG - Auth current implementation
EOS_NotificationId AddNotifyLoginStatusChanged(...)
{
    pFrameResult_t res(new FrameResult);
    // ... setup ...
    return GetCB_Manager().add_notification(this, res);
    // Notification stored but NEVER retrieved and fired
}

void Login(...)
{
    // ... work ...
    _logged_in = true;
    // ❌ Missing: trigger_login_status_changed()
}
```

**Fix:** Add trigger method and call it when state changes.

---

### Mistake 2: Using add_callback() Instead of add_notification()

```cpp
// ❌ WRONG - Treats notification as one-time callback
EOS_NotificationId AddNotifyLobbyUpdate(...)
{
    pFrameResult_t res(new FrameResult);
    // ... setup ...
    return GetCB_Manager().add_callback(this, res);  // ← Wrong!
    // This goes into _callbacks_to_run and gets removed after one execution
}
```

**Fix:** Use `add_notification()` for persistent listeners.

---

### Mistake 3: Not Populating Callback Data

```cpp
// ❌ WRONG - Auth current implementation
EOS_NotificationId AddNotifyLoginStatusChanged(...)
{
    pFrameResult_t res(new FrameResult);
    EOS_Auth_LoginStatusChangedCallbackInfo& lscci =
        res->CreateCallback<...>(...);

    lscci.ClientData = ClientData;
    // ❌ PreviousStatus, CurrentStatus, LocalUserId never set!

    return GetCB_Manager().add_notification(this, res);
}
```

**Fix:** Populate data when registering OR when firing (preferred).

---

### Mistake 4: Returning false from CBRunFrame

```cpp
// ❌ WRONG - Prevents periodic processing
bool CBRunFrame()
{
    return false;  // Service never does periodic work
}
```

**Fix:** If service needs periodic checks (timeouts, etc.), return `true` and implement logic.

---

### Mistake 5: No Network Handler

```cpp
// ❌ WRONG - Auth has no network handler
class EOSSDK_Auth
{
    // Missing: bool RunNetwork(void* callback_msg) override
    // Can't receive network events that would trigger notifications
};
```

**Fix:** Implement `RunNetwork()` to handle network messages if service is network-aware.

---

## Best Practices

### 1. Always Implement Trigger Methods

For every `AddNotify*()`, create corresponding `trigger_*()` or `notify_*()` method:

```cpp
// Registration
EOS_NotificationId AddNotifyXXX(...);

// Trigger
void trigger_xxx(...);
```

### 2. Document Trigger Points

```cpp
// Fired when: User authentication status changes
void trigger_login_status_changed(...);

// Fired when: Network lobby update message received
void notify_lobby_update(...);

// Fired when: P2P connection timeout detected in CBRunFrame
void notify_connection_interrupted(...);
```

### 3. Populate Data at Fire Time, Not Registration

```cpp
// ✓ GOOD - Data populated when event occurs
void trigger_login_status_changed(EOS_ELoginStatus prev, EOS_ELoginStatus curr)
{
    auto notifs = GetCB_Manager().get_notifications(...);

    for (auto& notif : notifs)
    {
        auto& lscci = notif->GetCallback<...>();
        lscci.PreviousStatus = prev;      // Fresh data
        lscci.CurrentStatus = curr;        // Event-specific
        lscci.LocalUserId = get_user();    // Current state
        notif->GetFunc()(notif->GetFuncParam());
    }
}
```

### 4. Use Descriptive Method Names

```cpp
// ✓ GOOD
void trigger_presence_change(EOS_EpicAccountId userid);
void notify_lobby_update(std::string const& lobby_id);
void fire_achievement_unlocked(std::string const& achievement_id);

// ❌ BAD
void handle_event();
void do_callback();
void process();
```

### 5. Consider Timing

```cpp
void Login(...)
{
    // Async work completes
    _logged_in = true;

    // Fire Login callback FIRST
    GetCB_Manager().add_callback(this, login_res);

    // Then fire status change notification
    trigger_login_status_changed(...);

    // Game receives: Login callback → LoginStatusChanged notification
}
```

---

## Summary

**Callback System Design:**
- Two types: One-time callbacks, Persistent notifications
- Callback Manager handles storage and execution
- Services responsible for triggering notifications

**Key Difference:**
- **One-time callbacks:** Automatically executed by `run_callbacks()`
- **Notifications:** Must be explicitly retrieved and fired by services

**Common Pattern:**
1. Event occurs (network/state/timeout)
2. Service calls `get_notifications()`
3. Service populates callback data
4. Service fires each notification

**Current Issue:**
- Many services (Auth, Connect, etc.) skip steps 2-4
- Notifications registered but never fired
- Games wait indefinitely for callbacks that never come

---

## Related Documentation

- [Callback Gaps Analysis](./CALLBACK_GAPS_ANALYSIS.md) - Which services are broken
- [Quick Reference](./QUICK_REFERENCE.md) - Service status summary
- [Implementation Guides](./IMPLEMENTATION_GUIDES.md) - How to fix broken services

---

**End of Document**
