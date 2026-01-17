# EOS SDK Emulator - Callback Implementation Guides

**Date:** 2026-01-17

## Table of Contents

1. [Auth Service Fix](#auth-service-fix)
2. [Connect Service Fix](#connect-service-fix)
3. [Presence Service Fix](#presence-service-fix)
4. [General Implementation Template](#general-implementation-template)
5. [Testing Guidelines](#testing-guidelines)

---

## Auth Service Fix

**Priority:** P1 CRITICAL - Blocks Palworld

**Files:** `eos_dll/eossdk_auth.h`, `eos_dll/eossdk_auth.cpp`

### Problem Summary

- `AddNotifyLoginStatusChanged()` registers notifications but never fires them
- Palworld waits for this callback during initialization
- Timeout causes offline mode fallback

### Step-by-Step Fix

#### Step 1: Add Trigger Method Declaration

**File:** `eos_dll/eossdk_auth.h`

**Location:** Add to `EOSSDK_Auth` class (around line 50-60)

```cpp
class EOSSDK_Auth : public IRunCallback
{
    // ... existing members ...

public:
    // ... existing methods ...

    // Add this method declaration
    void trigger_login_status_changed(
        EOS_ELoginStatus previous_status,
        EOS_ELoginStatus current_status
    );
};
```

#### Step 2: Implement Trigger Method

**File:** `eos_dll/eossdk_auth.cpp`

**Location:** Add after existing methods (e.g., after `RemoveNotifyLoginStatusChanged`)

```cpp
void EOSSDK_Auth::trigger_login_status_changed(
    EOS_ELoginStatus previous_status,
    EOS_ELoginStatus current_status)
{
    TRACE_FUNC();

    // Retrieve all stored LoginStatusChanged notifications
    auto notifs = std::move(GetCB_Manager().get_notifications(
        this,
        EOS_Auth_LoginStatusChangedCallbackInfo::k_iCallback
    ));

    // Fire each registered notification
    for (auto& notif : notifs)
    {
        auto& lscci = notif->GetCallback<EOS_Auth_LoginStatusChangedCallbackInfo>();

        // Populate callback data with current event info
        lscci.PreviousStatus = previous_status;
        lscci.CurrentStatus = current_status;
        lscci.LocalUserId = Settings::Inst().userid;

        // Execute the callback
        notif->GetFunc()(notif->GetFuncParam());
    }
}
```

#### Step 3: Call Trigger in Login Method

**File:** `eos_dll/eossdk_auth.cpp`

**Location:** In `Login()` method, after `_logged_in = true` (around line 135)

**Before:**
```cpp
void EOSSDK_Auth::Login(const EOS_Auth_LoginOptions* Options, ...)
{
    // ... async work ...

    GetCB_Manager().add_delayed_callback(
        delayed_cbk,
        [this, res, Options]()
        {
            // ... validation ...

            _logged_in = true;  // Line 135

            res->done = true;
            res->res = EOS_EResult::EOS_Success;

            // Populate login callback
            lci.ResultCode = res->res;
            lci.LocalUserId = Settings::Inst().userid;
            // ...

            GetCB_Manager().add_callback(this, res);
        }
    );
}
```

**After:**
```cpp
void EOSSDK_Auth::Login(const EOS_Auth_LoginOptions* Options, ...)
{
    // ... async work ...

    GetCB_Manager().add_delayed_callback(
        delayed_cbk,
        [this, res, Options]()
        {
            // ... validation ...

            _logged_in = true;  // Line 135

            res->done = true;
            res->res = EOS_EResult::EOS_Success;

            // Populate login callback
            lci.ResultCode = res->res;
            lci.LocalUserId = Settings::Inst().userid;
            // ...

            GetCB_Manager().add_callback(this, res);

            // NEW: Trigger login status changed notifications
            trigger_login_status_changed(
                EOS_ELoginStatus::EOS_LS_NotLoggedIn,
                EOS_ELoginStatus::EOS_LS_LoggedIn
            );
        }
    );
}
```

#### Step 4: Fix AddNotifyLoginStatusChanged

**File:** `eos_dll/eossdk_auth.cpp`

**Location:** `AddNotifyLoginStatusChanged()` method (lines 449-462)

**Before:**
```cpp
EOS_NotificationId EOSSDK_Auth::AddNotifyLoginStatusChanged(
    const EOS_Auth_AddNotifyLoginStatusChangedOptions* Options,
    void* ClientData,
    const EOS_Auth_OnLoginStatusChangedCallback Notification)
{
    pFrameResult_t res(new FrameResult);

    EOS_Auth_LoginStatusChangedCallbackInfo& lscci =
        res->CreateCallback<EOS_Auth_LoginStatusChangedCallbackInfo>(
            EOS_Auth_LoginStatusChangedCallbackInfo::k_iCallback,
            Notification,
            ClientData
        );

    lscci.ClientData = ClientData;
    // Missing: PreviousStatus, CurrentStatus, LocalUserId

    return GetCB_Manager().add_notification(this, res);
}
```

**After:**
```cpp
EOS_NotificationId EOSSDK_Auth::AddNotifyLoginStatusChanged(
    const EOS_Auth_AddNotifyLoginStatusChangedOptions* Options,
    void* ClientData,
    const EOS_Auth_OnLoginStatusChangedCallback Notification)
{
    pFrameResult_t res(new FrameResult);

    EOS_Auth_LoginStatusChangedCallbackInfo& lscci =
        res->CreateCallback<EOS_Auth_LoginStatusChangedCallbackInfo>(
            EOS_Auth_LoginStatusChangedCallbackInfo::k_iCallback,
            Notification,
            ClientData
        );

    lscci.ClientData = ClientData;

    // NEW: Populate initial status
    lscci.PreviousStatus = EOS_ELoginStatus::EOS_LS_NotLoggedIn;
    lscci.CurrentStatus = _logged_in ?
        EOS_ELoginStatus::EOS_LS_LoggedIn :
        EOS_ELoginStatus::EOS_LS_NotLoggedIn;
    lscci.LocalUserId = Settings::Inst().userid;

    return GetCB_Manager().add_notification(this, res);
}
```

#### Step 5: Handle Logout

**File:** `eos_dll/eossdk_auth.cpp`

**Location:** In `Logout()` method (if exists) or create one

```cpp
void EOSSDK_Auth::Logout(...)
{
    // ... logout logic ...

    bool was_logged_in = _logged_in;
    _logged_in = false;

    // Trigger logout status change
    if (was_logged_in)
    {
        trigger_login_status_changed(
            EOS_ELoginStatus::EOS_LS_LoggedIn,
            EOS_ELoginStatus::EOS_LS_NotLoggedIn
        );
    }
}
```

### Testing

1. **Basic Test:**
```cpp
// Pseudo-code
void test_auth_notification()
{
    bool notification_fired = false;
    EOS_ELoginStatus received_previous = EOS_LS_NotLoggedIn;
    EOS_ELoginStatus received_current = EOS_LS_NotLoggedIn;

    auto callback = [](const EOS_Auth_LoginStatusChangedCallbackInfo* data) {
        notification_fired = true;
        received_previous = data->PreviousStatus;
        received_current = data->CurrentStatus;
    };

    // Register notification
    EOS_Auth_AddNotifyLoginStatusChanged(options, clientData, callback);

    // Login
    EOS_Auth_Login(loginOptions, clientData, loginCallback);

    // Wait for async completion
    while (!login_complete) {
        EOS_Platform_Tick();
    }

    // Verify
    assert(notification_fired == true);
    assert(received_previous == EOS_LS_NotLoggedIn);
    assert(received_current == EOS_LS_LoggedIn);
}
```

2. **Palworld Test:**
   - Launch Palworld with fixed SDK
   - Monitor for "online mode" vs "offline mode"
   - Check logs for LoginStatusChanged callback execution

---

## Connect Service Fix

**Priority:** P1 CRITICAL - Blocks online features

**Files:** `eos_dll/eossdk_connect.h`, `eos_dll/eossdk_connect.cpp`

### Problem Summary

- Same issue as Auth
- `AddNotifyLoginStatusChanged()` and `AddNotifyAuthExpiration()` never fire

### Step-by-Step Fix

#### Step 1: Add Trigger Method Declarations

**File:** `eos_dll/eossdk_connect.h`

**Location:** Add to `EOSSDK_Connect` class

```cpp
class EOSSDK_Connect : public IRunCallback
{
    // ... existing members ...

public:
    // ... existing methods ...

    // Add these method declarations
    void trigger_login_status_changed(
        EOS_ELoginStatus previous_status,
        EOS_ELoginStatus current_status
    );

    void trigger_auth_expiration();
};
```

#### Step 2: Implement Login Status Trigger

**File:** `eos_dll/eossdk_connect.cpp`

**Location:** Add after existing methods

```cpp
void EOSSDK_Connect::trigger_login_status_changed(
    EOS_ELoginStatus previous_status,
    EOS_ELoginStatus current_status)
{
    TRACE_FUNC();

    // Retrieve all stored LoginStatusChanged notifications
    auto notifs = std::move(GetCB_Manager().get_notifications(
        this,
        EOS_Connect_LoginStatusChangedCallbackInfo::k_iCallback
    ));

    // Fire each registered notification
    for (auto& notif : notifs)
    {
        auto& lscci = notif->GetCallback<EOS_Connect_LoginStatusChangedCallbackInfo>();

        // Populate callback data
        lscci.PreviousStatus = previous_status;
        lscci.CurrentStatus = current_status;
        lscci.LocalUserId = Settings::Inst().productuserid;

        // Execute the callback
        notif->GetFunc()(notif->GetFuncParam());
    }
}
```

#### Step 3: Implement Auth Expiration Trigger

**File:** `eos_dll/eossdk_connect.cpp`

**Location:** Add after login status trigger

```cpp
void EOSSDK_Connect::trigger_auth_expiration()
{
    TRACE_FUNC();

    // Retrieve all stored AuthExpiration notifications
    auto notifs = std::move(GetCB_Manager().get_notifications(
        this,
        EOS_Connect_AuthExpirationCallbackInfo::k_iCallback
    ));

    // Fire each registered notification
    for (auto& notif : notifs)
    {
        auto& aeci = notif->GetCallback<EOS_Connect_AuthExpirationCallbackInfo>();

        // Populate callback data
        aeci.LocalUserId = Settings::Inst().productuserid;

        // Execute the callback
        notif->GetFunc()(notif->GetFuncParam());
    }
}
```

#### Step 4: Call Trigger in Login Method

**File:** `eos_dll/eossdk_connect.cpp`

**Location:** In `Login()` method, after `_logged_in = true` (around line 313)

**Find this section:**
```cpp
void EOSSDK_Connect::Login(const EOS_Connect_LoginOptions* Options, ...)
{
    // ... async work ...

    GetCB_Manager().add_delayed_callback(
        delayed_cbk,
        [this, res, Options]()
        {
            // ... validation ...

            _logged_in = true;  // Around line 313

            // Populate login callback
            lci.ResultCode = EOS_EResult::EOS_Success;
            lci.LocalUserId = Settings::Inst().productuserid;

            GetCB_Manager().add_callback(this, res);
        }
    );
}
```

**Add trigger call:**
```cpp
            GetCB_Manager().add_callback(this, res);

            // NEW: Trigger login status changed notifications
            trigger_login_status_changed(
                EOS_ELoginStatus::EOS_LS_NotLoggedIn,
                EOS_ELoginStatus::EOS_LS_LoggedIn
            );
```

#### Step 5: Fix AddNotifyLoginStatusChanged

**File:** `eos_dll/eossdk_connect.cpp`

**Location:** `AddNotifyLoginStatusChanged()` method (lines 658-676)

**Before:**
```cpp
EOS_NotificationId EOSSDK_Connect::AddNotifyLoginStatusChanged(...)
{
    pFrameResult_t res(new FrameResult);

    EOS_Connect_LoginStatusChangedCallbackInfo& lscci =
        res->CreateCallback<...>(...);

    lscci.ClientData = ClientData;
    lscci.PreviousStatus = EOS_ELoginStatus::EOS_LS_LoggedIn;  // Static!
    lscci.CurrentStatus = EOS_ELoginStatus::EOS_LS_LoggedIn;   // Static!
    lscci.LocalUserId = Settings::Inst().productuserid;

    return GetCB_Manager().add_notification(this, res);
}
```

**After:**
```cpp
EOS_NotificationId EOSSDK_Connect::AddNotifyLoginStatusChanged(...)
{
    pFrameResult_t res(new FrameResult);

    EOS_Connect_LoginStatusChangedCallbackInfo& lscci =
        res->CreateCallback<...>(...);

    lscci.ClientData = ClientData;

    // NEW: Use actual current status
    lscci.PreviousStatus = EOS_ELoginStatus::EOS_LS_NotLoggedIn;
    lscci.CurrentStatus = _logged_in ?
        EOS_ELoginStatus::EOS_LS_LoggedIn :
        EOS_ELoginStatus::EOS_LS_NotLoggedIn;
    lscci.LocalUserId = Settings::Inst().productuserid;

    return GetCB_Manager().add_notification(this, res);
}
```

#### Step 6: Implement Auth Expiration Detection (Optional)

**File:** `eos_dll/eossdk_connect.cpp`

**Location:** In `CBRunFrame()` method (lines 1003-1005)

**Before:**
```cpp
bool EOSSDK_Connect::CBRunFrame()
{
    return false;
}
```

**After (if auth expiration needed):**
```cpp
bool EOSSDK_Connect::CBRunFrame()
{
    // Check if authentication has expired
    // This would depend on your auth token expiration logic

    if (_logged_in && auth_token_expired())
    {
        // Fire auth expiration notification
        trigger_auth_expiration();

        // Optional: Auto-logout
        _logged_in = false;
        trigger_login_status_changed(
            EOS_ELoginStatus::EOS_LS_LoggedIn,
            EOS_ELoginStatus::EOS_LS_NotLoggedIn
        );
    }

    return true;  // Continue processing
}

private:
bool auth_token_expired()
{
    // Implement based on your token management
    // Example:
    // auto now = std::chrono::steady_clock::now();
    // return (now > _auth_token_expiry_time);
    return false;  // Stub for now
}
```

### Testing

Similar to Auth service testing:

1. Register `AddNotifyLoginStatusChanged()` before Connect login
2. Call `EOS_Connect_Login()`
3. Verify notification fires with correct status
4. Test auth expiration if implemented

---

## Presence Service Fix

**Priority:** P3 MEDIUM - Join game feature

**Files:** `eos_dll/eossdk_presence.h`, `eos_dll/eossdk_presence.cpp`

### Problem Summary

- `AddNotifyJoinGameAccepted()` registers but never fires
- `AddNotifyOnPresenceChanged()` already works (good reference)

### Step-by-Step Fix

#### Step 1: Add Trigger Method Declaration

**File:** `eos_dll/eossdk_presence.h`

**Location:** Add to `EOSSDK_Presence` class

```cpp
class EOSSDK_Presence
{
    // ... existing members ...

    // Existing trigger (already works):
    void trigger_presence_change(EOS_EpicAccountId userid);

public:
    // ... existing methods ...

    // NEW: Add this method declaration
    void trigger_join_game_accepted(
        EOS_UI_EventId ui_event_id,
        EOS_EpicAccountId local_user_id,
        EOS_EpicAccountId target_user_id,
        const char* join_info
    );
};
```

#### Step 2: Implement Trigger Method

**File:** `eos_dll/eossdk_presence.cpp`

**Location:** Add after `trigger_presence_change()` (around line 682)

```cpp
void EOSSDK_Presence::trigger_join_game_accepted(
    EOS_UI_EventId ui_event_id,
    EOS_EpicAccountId local_user_id,
    EOS_EpicAccountId target_user_id,
    const char* join_info)
{
    TRACE_FUNC();

    // Retrieve all stored JoinGameAccepted notifications
    auto notifs = std::move(GetCB_Manager().get_notifications(
        this,
        EOS_Presence_JoinGameAcceptedCallbackInfo::k_iCallback
    ));

    // Fire each registered notification
    for (auto& notif : notifs)
    {
        auto& jgaci = notif->GetCallback<EOS_Presence_JoinGameAcceptedCallbackInfo>();

        // Populate callback data
        jgaci.UiEventId = ui_event_id;
        jgaci.LocalUserId = local_user_id;
        jgaci.TargetUserId = target_user_id;
        jgaci.JoinInfo = join_info;

        // Execute the callback
        notif->GetFunc()(notif->GetFuncParam());
    }
}
```

#### Step 3: Add Network Handler (if needed)

**File:** `eos_dll/eossdk_presence.cpp`

**Location:** In `RunNetwork()` method or add new handler

This depends on how join game invites are sent. You may need to:

1. Add a new network message type for join game invites
2. Handle the message in `RunNetwork()`
3. Call `trigger_join_game_accepted()` when processing the message

**Example:**
```cpp
bool EOSSDK_Presence::RunNetwork(void* callback_msg)
{
    Common_Message* msg = (Common_Message*)callback_msg;

    // ... existing handlers ...

    if (msg->has_presence_join_game())
    {
        on_join_game_invite(msg->presence_join_game());
        return true;
    }

    return false;
}

void EOSSDK_Presence::on_join_game_invite(Network_Callback_Message_pb const& msg)
{
    // Parse message
    EOS_UI_EventId ui_event_id = ...; // from message
    EOS_EpicAccountId local_user = ...; // from message
    EOS_EpicAccountId target_user = ...; // from message
    const char* join_info = ...; // from message

    // Trigger notification
    trigger_join_game_accepted(ui_event_id, local_user, target_user, join_info);
}
```

### Testing

1. Send join game invite from one client
2. Accept invite on another client
3. Verify `JoinGameAccepted` notification fires
4. Verify join info is correct

---

## General Implementation Template

Use this template for implementing any missing notification trigger:

### Template Code

```cpp
// ============================================
// STEP 1: Header Declaration
// ============================================
// In eossdk_yourservice.h

class EOSSDK_YourService
{
public:
    void trigger_your_notification(/* event-specific parameters */);
};

// ============================================
// STEP 2: Implementation
// ============================================
// In eossdk_yourservice.cpp

void EOSSDK_YourService::trigger_your_notification(/* parameters */)
{
    TRACE_FUNC();

    // Get stored notifications of this type
    auto notifs = std::move(GetCB_Manager().get_notifications(
        this,
        YourCallbackInfo::k_iCallback  // Use actual callback type
    ));

    // Fire each registered notification
    for (auto& notif : notifs)
    {
        // Get typed callback info
        auto& callback_info = notif->GetCallback<YourCallbackInfo>();

        // Populate event data
        callback_info.Field1 = value1;
        callback_info.Field2 = value2;
        // ... populate all required fields ...

        // Execute the callback
        notif->GetFunc()(notif->GetFuncParam());
    }
}

// ============================================
// STEP 3: Call from Event Source
// ============================================

// Option A: Network handler
void on_your_network_event(Network_Message_pb const& msg)
{
    // Parse message
    auto data = extract_data(msg);

    // Update local state
    update_state(data);

    // Trigger notifications
    trigger_your_notification(data);
}

// Option B: State change
void YourMethod()
{
    // Do work
    perform_operation();

    // State changed
    _state = new_value;

    // Trigger notifications
    trigger_your_notification(new_value);
}

// Option C: Periodic check (CBRunFrame)
bool CBRunFrame() override
{
    // Check condition
    if (should_notify())
    {
        // Trigger notification
        trigger_your_notification(/* params */);
    }

    return true;  // Continue processing
}
```

### Checklist

- [ ] Add trigger method declaration to header
- [ ] Implement trigger method in .cpp
- [ ] Identify event source (network/state/periodic)
- [ ] Call trigger from event source
- [ ] Populate all callback info fields
- [ ] Test notification fires
- [ ] Test multiple registered listeners
- [ ] Test registration after event occurs
- [ ] Test notification removal

---

## Testing Guidelines

### Unit Test Template

```cpp
void test_notification()
{
    // Setup
    bool notification_fired = false;
    YourCallbackData received_data;

    auto callback = [&](const YourCallbackInfo* data) {
        notification_fired = true;
        received_data = *data;
    };

    // Register notification
    EOS_NotificationId id = YourService_AddNotifyXXX(
        options,
        &callback,  // client data
        callback
    );

    // Trigger event
    perform_action_that_triggers_event();

    // Process callbacks
    EOS_Platform_Tick();

    // Verify
    assert(notification_fired == true);
    assert(received_data.Field1 == expected_value1);
    assert(received_data.Field2 == expected_value2);

    // Cleanup
    YourService_RemoveNotifyXXX(id);
}
```

### Integration Test with Real Game

1. **Enable Debug Logging:**
```cpp
// Add to notification trigger
TRACE_FUNC();
APP_LOG(Log::LogLevel::DEBUG, "Firing notification: %d listeners", notifs.size());
```

2. **Monitor Game Behavior:**
   - Check if game reaches expected state
   - Verify no timeout occurs
   - Confirm online features work

3. **Log Callback Execution:**
```cpp
// In trigger method
for (auto& notif : notifs)
{
    APP_LOG(Log::LogLevel::DEBUG, "Executing callback for type: %d", notif->m_iCallback);
    auto& callback_info = notif->GetCallback<...>();
    // ... populate and fire ...
    APP_LOG(Log::LogLevel::DEBUG, "Callback executed successfully");
}
```

### Regression Testing

After implementing fixes:

1. **Test all existing working services still work:**
   - Lobby notifications
   - Session notifications
   - P2P notifications

2. **Test edge cases:**
   - Register notification before event
   - Register notification after event
   - Multiple listeners for same event
   - Remove notification and verify it doesn't fire
   - Rapid connect/disconnect cycles

3. **Test with Palworld:**
   - Launch game
   - Monitor initialization sequence
   - Verify online mode
   - Test multiplayer features
   - Check for any errors or warnings

### Performance Testing

1. **Many Listeners:**
   - Register 100+ listeners
   - Trigger event
   - Verify all fire
   - Check for performance issues

2. **Rapid Events:**
   - Trigger notifications rapidly
   - Verify no callbacks lost
   - Check for memory leaks

3. **Long Running:**
   - Keep notifications registered for hours
   - Trigger periodically
   - Verify no memory growth
   - Check callback still fires correctly

---

## Common Pitfalls to Avoid

### 1. Not Using std::move()

```cpp
// ❌ WRONG - Creates copies
auto notifs = GetCB_Manager().get_notifications(...);

// ✅ CORRECT - Transfers ownership
auto notifs = std::move(GetCB_Manager().get_notifications(...));
```

### 2. Wrong Callback Type ID

```cpp
// ❌ WRONG - Using wrong callback ID
auto notifs = GetCB_Manager().get_notifications(
    this,
    EOS_Auth_LoginCallbackInfo::k_iCallback  // Login, not LoginStatusChanged!
);

// ✅ CORRECT - Using matching callback type
auto notifs = GetCB_Manager().get_notifications(
    this,
    EOS_Auth_LoginStatusChangedCallbackInfo::k_iCallback
);
```

### 3. Forgetting to Populate Fields

```cpp
// ❌ WRONG - Fields not populated
auto& lscci = notif->GetCallback<EOS_Auth_LoginStatusChangedCallbackInfo>();
notif->GetFunc()(notif->GetFuncParam());  // Fields are garbage!

// ✅ CORRECT - All fields populated
auto& lscci = notif->GetCallback<EOS_Auth_LoginStatusChangedCallbackInfo>();
lscci.PreviousStatus = prev;
lscci.CurrentStatus = curr;
lscci.LocalUserId = user_id;
notif->GetFunc()(notif->GetFuncParam());
```

### 4. Wrong Timing

```cpp
// ❌ WRONG - Trigger before callback registered
trigger_login_status_changed(...);
GetCB_Manager().add_callback(this, login_res);

// ✅ CORRECT - Callback first, then notification
GetCB_Manager().add_callback(this, login_res);
trigger_login_status_changed(...);  // After login callback queued
```

### 5. Using add_callback Instead of add_notification

```cpp
// ❌ WRONG - In AddNotifyXXX method
return GetCB_Manager().add_callback(this, res);  // One-time!

// ✅ CORRECT - For persistent notifications
return GetCB_Manager().add_notification(this, res);  // Persistent
```

---

## Related Documentation

- [Callback Gaps Analysis](./CALLBACK_GAPS_ANALYSIS.md) - Detailed problem analysis
- [Callback Architecture](./CALLBACK_ARCHITECTURE.md) - System design
- [Quick Reference](./QUICK_REFERENCE.md) - Service status overview

---

**End of Implementation Guides**
