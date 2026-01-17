# EOS SDK Emulator - Callback & Notification Gaps Analysis

**Date:** 2026-01-17
**Branch:** claude/callback-analysis-docs-R05Xf

## Executive Summary

This document provides a comprehensive analysis of callback and notification implementations across all EOS SDK emulator services. The analysis reveals critical gaps in notification delivery mechanisms that cause games like **Palworld to fail initialization and fall back to offline mode**.

**Key Finding:** Only 4 out of 17 services properly implement notification triggers. The Auth and Connect services, critical for game initialization, have completely broken notification mechanisms.

---

## Table of Contents

1. [Critical Issue: Palworld Offline Mode](#critical-issue-palworld-offline-mode)
2. [Services Status Overview](#services-status-overview)
3. [Working Services (Reference Implementations)](#working-services-reference-implementations)
4. [Broken Services (Missing Triggers)](#broken-services-missing-triggers)
5. [Root Cause Analysis](#root-cause-analysis)
6. [Recommended Fixes](#recommended-fixes)

---

## Critical Issue: Palworld Offline Mode

### Problem Description

Palworld's initialization sequence expects the following callback flow:

```
1. Call EOS_Platform_Create()
2. Register notifications: AddNotifyLoginStatusChanged()
3. Call EOS_Auth_Login()
4. Receive EOS_Auth_Login callback ✅ (works)
5. Receive LoginStatusChanged notification ❌ (NEVER FIRES)
6. Complete online initialization
```

**What happens instead:**
- Step 5 never occurs because notifications are registered but never triggered
- Palworld waits for the LoginStatusChanged callback as part of its handshake
- After timeout, Palworld assumes no online backend is available
- Game falls back to offline mode

### Affected Services

The same pattern affects multiple services, but **Auth** and **Connect** are critical for game startup.

---

## Services Status Overview

| Service | Notifications | Working | Broken | Missing | Status |
|---------|--------------|---------|--------|---------|--------|
| **Lobby** | 6 | 6 | 0 | 0 | ✅ WORKING |
| **Sessions** | 3 | 3 | 0 | 0 | ✅ WORKING |
| **P2P** | 4 | 4 | 0 | 0 | ✅ WORKING |
| **Presence** | 2 | 1 | 0 | 1 | ⚠️ PARTIAL |
| **Auth** | 2 | 0 | 2 | 0 | ❌ BROKEN |
| **Connect** | 2 | 0 | 2 | 0 | ❌ BROKEN |
| **Friends** | 1 | 1* | 0 | 0 | ⚠️ INDIRECT |
| **Achievements** | 2 | 0 | 2 | 0 | ❌ BROKEN |
| **UI** | 1 | 0 | 1 | 0 | ❌ BROKEN |
| **RTCData** | 1 | 0 | 1 | 0 | ❌ BROKEN |
| **Leaderboards** | 0 | - | - | - | ⚠️ NO NOTIFS |
| **Stats** | 0 | - | - | - | ⚠️ NO NOTIFS |
| **UserInfo** | 0 | - | - | - | ⚠️ NO NOTIFS |
| **Ecom** | 0 | - | - | - | ⚠️ NO NOTIFS |
| **PlayerDataStorage** | 0 | - | - | - | ⚠️ NO NOTIFS |
| **TitleStorage** | 0 | - | - | - | ⚠️ NO NOTIFS |
| **RTC/RTCAdmin** | Many | 0 | All | 0 | ❌ STUBS ONLY |

*Friends notifications fire but only when triggered by Connect service

**Summary:**
- ✅ **Fully Working:** 3 services
- ⚠️ **Partially Working:** 2 services
- ❌ **Broken:** 5 services
- ⚠️ **No Notifications Implemented:** 6 services
- ❌ **Stub Only:** 1 service

---

## Working Services (Reference Implementations)

These services demonstrate the **correct pattern** for notification delivery.

### 1. Lobby Service ✅

**File:** `eos_dll/eossdk_lobby.cpp`

**Notifications Implemented:**
- `AddNotifyLobbyUpdateReceived()`
- `AddNotifyLobbyMemberUpdateReceived()`
- `AddNotifyLobbyMemberStatusReceived()`
- `AddNotifyLobbyInviteReceived()`
- `AddNotifyLobbyInviteAccepted()`
- `AddNotifyJoinLobbyAccepted()`

**How It Works:**

```cpp
// Registration (lines 238-246)
EOS_NotificationId AddNotifyLobbyUpdateReceived(...)
{
    pFrameResult_t res(new FrameResult);
    // ... setup callback info ...
    return GetCB_Manager().add_notification(this, res);
}

// Trigger mechanism (lines 260-272)
void notify_lobby_update(std::string const& lobby_id)
{
    // 1. Retrieve stored notifications
    auto notifs = std::move(GetCB_Manager().get_notifications(
        this, EOS_Lobby_LobbyUpdateReceivedCallbackInfo::k_iCallback));

    // 2. Fire each notification
    for (auto& notif : notifs)
    {
        auto& luci = notif->GetCallback<EOS_Lobby_LobbyUpdateReceivedCallbackInfo>();
        luci.LobbyId = lobby_id.c_str();
        notif->GetFunc()(notif->GetFuncParam());  // ← FIRES CALLBACK
    }
}

// Network handler calls trigger (lines 449-470)
void on_lobby_updated(Network_Message_pb const& msg)
{
    // ... process message ...
    notify_lobby_update(lobby_id);  // ← TRIGGERS NOTIFICATIONS
}
```

**Key Pattern:**
1. Network message arrives → handler processes it
2. Handler calls `notify_*()` method
3. `notify_*()` retrieves stored notifications via `get_notifications()`
4. Loop through and fire each notification

---

### 2. Sessions Service ✅

**File:** `eos_dll/eossdk_sessions.cpp`

**Notifications Implemented:**
- `AddNotifySessionInviteReceived()`
- `AddNotifySessionInviteAccepted()`
- `AddNotifyJoinSessionAccepted()`

**How It Works:**

```cpp
// Direct firing in network handler (lines 1972-1987)
void on_session_invite(Network_Message_pb const& msg)
{
    // ... parse message ...

    // Get stored notifications
    auto notifs = std::move(GetCB_Manager().get_notifications(
        this, EOS_Sessions_SessionInviteReceivedCallbackInfo::k_iCallback));

    // Fire each notification
    for (auto& notif : notifs)
    {
        auto& sirci = notif->GetCallback<...>();
        sirci.InviteId = invite_id.c_str();
        sirci.LocalUserId = local_user;
        sirci.TargetUserId = target_user;
        notif->GetFunc()(notif->GetFuncParam());
    }
}
```

**Variation:** Sessions fires notifications directly in network handlers instead of using separate `notify_*()` methods. Both patterns work.

---

### 3. P2P Service ✅

**File:** `eos_dll/eossdk_p2p.cpp`

**Notifications Implemented:**
- `AddNotifyPeerConnectionRequest()`
- `AddNotifyPeerConnectionEstablished()`
- `AddNotifyPeerConnectionInterrupted()`
- `AddNotifyPeerConnectionClosed()`

**How It Works:**

```cpp
// Network handler (lines 945-968)
void on_p2p_connection_request(Network_Message_pb const& msg)
{
    // ... process request ...

    auto notifs = std::move(GetCB_Manager().get_notifications(
        this, EOS_P2P_OnIncomingConnectionRequestInfo::k_iCallback));

    for (auto& notif : notifs)
    {
        auto& oicri = notif->GetCallback<...>();
        oicri.LocalUserId = local_user;
        oicri.RemoteUserId = remote_user;
        oicri.SocketId = &socket_id;
        notif->GetFunc()(notif->GetFuncParam());
    }
}

// Also uses CBRunFrame for timeout detection (lines 921-930)
bool CBRunFrame()
{
    // ... detect connection timeouts ...

    // Fire interrupted/closed notifications for timed-out connections
    auto notifs = GetCB_Manager().get_notifications(...);
    // ... fire them ...

    return true;  // Continue frame processing
}
```

**Variation:** P2P combines network handlers AND CBRunFrame for different notification types. Shows flexibility.

---

### 4. Presence Service ✅ (Partial)

**File:** `eos_dll/eossdk_presence.cpp`

**Notifications Implemented:**
- `AddNotifyOnPresenceChanged()` ✅ Works
- `AddNotifyJoinGameAccepted()` ❌ Never fires

**How It Works:**

```cpp
// Trigger mechanism (lines 670-681)
void trigger_presence_change(EOS_EpicAccountId userid)
{
    auto notifs = std::move(GetCB_Manager().get_notifications(
        this, EOS_Presence_PresenceChangedCallbackInfo::k_iCallback));

    for (auto& notif : notifs)
    {
        auto& pcci = notif->GetCallback<...>();
        pcci.PresenceUserId = userid;
        notif->GetFunc()(notif->GetFuncParam());
    }
}

// Network handler calls it (lines 534-570)
void on_presence_infos(Network_Callback_Message_pb const& msg)
{
    // ... update presence data ...
    trigger_presence_change(user_id);
}
```

**Issue:** `JoinGameAccepted` is registered (lines 688-701) but no code triggers it.

---

## Broken Services (Missing Triggers)

### 1. Auth Service ❌ CRITICAL

**File:** `eos_dll/eossdk_auth.cpp`

**Registered Notifications:**
- `AddNotifyLoginStatusChanged()` (lines 449-462)
- `AddNotifyLoginStatusChangedOld()` (lines 464-476)

**What's Broken:**

```cpp
// Registration works
EOS_NotificationId AddNotifyLoginStatusChanged(...)
{
    pFrameResult_t res(new FrameResult);
    EOS_Auth_LoginStatusChangedCallbackInfo& lscci =
        res->CreateCallback<...>(...);

    lscci.ClientData = ClientData;
    // NOTE: CurrentStatus, PreviousStatus, LocalUserId NEVER SET!

    return GetCB_Manager().add_notification(this, res);  // Stored
}

// Login completes successfully (lines 81-151)
void Login(...)
{
    // ... async delay ...

    res->done = true;
    res->res = EOS_EResult::EOS_Success;

    _logged_in = true;  // State updated

    // ❌ MISSING: No code to trigger LoginStatusChanged notifications!

    GetCB_Manager().add_callback(this, res);  // Only fires Login callback
}

// Frame processing does nothing (lines 481-483)
bool CBRunFrame()
{
    return false;  // Never processes notifications
}

// No network handler
// No trigger mechanism exists
```

**Impact:**
- Games waiting for login status notifications hang indefinitely
- Palworld's initialization handshake fails
- No way to detect when users log in/out

**What's Needed:**

```cpp
// Add trigger method
void trigger_login_status_changed(EOS_ELoginStatus prev, EOS_ELoginStatus curr)
{
    auto notifs = GetCB_Manager().get_notifications(
        this, EOS_Auth_LoginStatusChangedCallbackInfo::k_iCallback);

    for (auto& notif : notifs) {
        auto& lscci = notif->GetCallback<...>();
        lscci.PreviousStatus = prev;
        lscci.CurrentStatus = curr;
        lscci.LocalUserId = Settings::Inst().userid;
        notif->GetFunc()(notif->GetFuncParam());
    }
}

// Call in Login() after setting _logged_in = true
trigger_login_status_changed(
    EOS_ELoginStatus::EOS_LS_NotLoggedIn,
    EOS_ELoginStatus::EOS_LS_LoggedIn
);
```

---

### 2. Connect Service ❌ CRITICAL

**File:** `eos_dll/eossdk_connect.cpp`

**Registered Notifications:**
- `AddNotifyAuthExpiration()` (lines 640-656)
- `AddNotifyLoginStatusChanged()` (lines 658-676)

**What's Broken:**

```cpp
// Registration works (lines 658-676)
EOS_NotificationId AddNotifyLoginStatusChanged(...)
{
    pFrameResult_t res(new FrameResult);
    EOS_Connect_LoginStatusChangedCallbackInfo& lscci =
        res->CreateCallback<...>(...);

    lscci.ClientData = ClientData;
    lscci.PreviousStatus = EOS_ELoginStatus::EOS_LS_LoggedIn;  // Static!
    lscci.CurrentStatus = EOS_ELoginStatus::EOS_LS_LoggedIn;   // Static!
    lscci.LocalUserId = Settings::Inst().productuserid;

    return GetCB_Manager().add_notification(this, res);  // Stored but never fired
}

// Login succeeds but doesn't trigger status change (lines 224-330)
void Login(...)
{
    // ... async delay ...

    _logged_in = true;

    // ❌ MISSING: No trigger for LoginStatusChanged

    GetCB_Manager().add_callback(this, res);  // Only fires Login callback
}

// Frame processing does nothing (lines 1003-1005)
bool CBRunFrame()
{
    return false;
}
```

**Interesting Finding:**

Connect DOES fire notifications, but for **Friends**, not itself:

```cpp
// Network handler (lines 895-907)
void on_user_infos(Network_Callback_Message_pb const& msg)
{
    // ... process user info ...

    // Fire FRIENDS notifications (not Connect's own notifications!)
    std::vector<pFrameResult_t> notifs = std::move(
        GetCB_Manager().get_notifications(
            &GetEOS_Friends(),  // ← Different service!
            EOS_Friends_OnFriendsUpdateInfo::k_iCallback
        )
    );

    for (auto& notif : notifs) {
        // ... fire friend update ...
    }
}
```

This shows the developer knows HOW to fire notifications, but forgot to implement it for Connect's own callbacks.

**Impact:**
- No way to detect when Connect authentication changes
- Auth expiration never notified
- Games expecting Connect status updates hang

---

### 3. Friends Service ⚠️ INDIRECT

**File:** `eos_dll/eossdk_friends.cpp`

**Registered Notifications:**
- `AddNotifyFriendsUpdate()` (lines 268-281)

**What's Unusual:**

```cpp
// Registration works
EOS_NotificationId AddNotifyFriendsUpdate(...)
{
    pFrameResult_t res(new FrameResult);
    // ... setup ...
    return GetCB_Manager().add_notification(this, res);
}

// But Friends service has NO network handler
// No RunNetwork() method exists
// CBRunFrame() returns false (lines 303-305)

// Instead, CONNECT fires Friends notifications!
// See Connect::on_user_infos() line 903
```

**Status:** Notifications work, but only because Connect service fires them. Friends is passive.

**Architectural Question:** Is this intentional or a workaround? Should Friends have its own network handlers?

---

### 4. Achievements Service ❌

**File:** `eos_dll/eossdk_achievements.cpp`

**Registered Notifications:**
- `AddNotifyAchievementsUnlocked()` (lines 214-228)
- `AddNotifyAchievementsUnlockedV2()` (lines 230-244)

**What's Broken:**

```cpp
// Registration works
EOS_NotificationId AddNotifyAchievementsUnlocked(...)
{
    pFrameResult_t res(new FrameResult);
    // ... setup ...
    return GetCB_Manager().add_notification(this, res);
}

// No RunNetwork() method exists
// CBRunFrame() returns false (lines 286-288)
// No trigger mechanism
// FreeCallback has cleanup code but no firing logic
```

**Impact:**
- Games never receive achievement unlock notifications
- Achievements unlock silently without notifying the game
- Games that depend on achievement callbacks for UI updates won't show progress

---

### 5. UI Service ❌

**File:** `eos_dll/eossdk_ui.cpp`

**Registered Notifications:**
- `AddNotifyDisplaySettingsUpdated()` (lines 42-55)

**What's Broken:**

```cpp
// Registration works
EOS_NotificationId AddNotifyDisplaySettingsUpdated(...)
{
    pFrameResult_t res(new FrameResult);
    // ... setup ...
    return GetCB_Manager().add_notification(this, res);
}

// No RunNetwork() method
// CBRunFrame() returns false (lines 75-77)
// No mechanism to detect display settings changes
```

**Impact:** Limited, as most games don't dynamically change display settings during runtime.

---

### 6. RTCData Service ❌

**File:** `eos_dll/eossdk_rtcdata.cpp`

**Registered Notifications:**
- `AddNotifyDataReceived()` (lines 135-149)

**What's Broken:**

```cpp
// Registration works
EOS_NotificationId AddNotifyDataReceived(...)
{
    pFrameResult_t res(new FrameResult);
    // ... setup ...
    return GetCB_Manager().add_notification(this, res);
}

// No RunNetwork() method
// CBRunFrame() returns false (lines 169-171)
// No mechanism to handle incoming RTC data
```

**Impact:**
- RTC data communication completely broken
- Voice/video data reception won't trigger callbacks
- Games using RTC features won't work

---

## Root Cause Analysis

### Pattern 1: No Network Handlers

Services like Auth, Connect (for its own notifications), Friends, Achievements lack `RunNetwork()` methods.

**Issue:** Without network handlers, services can't receive network messages that would trigger notifications.

**Example:**
- Lobby has `on_lobby_updated()`, `on_lobby_invite()`, etc.
- Auth has NO `on_*()` network handlers at all

### Pattern 2: CBRunFrame Returns False

Most broken services have trivial `CBRunFrame()` implementations:

```cpp
bool CBRunFrame()
{
    return false;  // No periodic processing
}
```

**Working services** use `CBRunFrame()` for:
- Timeout detection (P2P)
- Periodic state checks
- Trigger accumulated notifications

### Pattern 3: Missing Trigger Methods

Working services define explicit trigger methods:
- `trigger_presence_change()`
- `notify_lobby_update()`
- `notify_lobby_member_status_update()`

**Broken services** have no equivalent methods.

### Pattern 4: Callback Manager Integration Incomplete

**How it should work:**

1. `AddNotify*()` calls `GetCB_Manager().add_notification(this, res)` ✅
2. Notification stored in `_notifications` map ✅
3. Later, when event occurs:
   - Call `GetCB_Manager().get_notifications(this, callback_id)` ❌ MISSING
   - Loop through results and fire callbacks ❌ MISSING

**Most services stop after step 2.**

### Pattern 5: Cross-Service Dependency

Connect fires Friends notifications shows:
- Services can trigger notifications for other services
- But this creates tight coupling
- If Connect fails, Friends notifications fail too

### Key Files in Callback System

**Callback Manager:** `eos_dll/callback_manager.h` & `.cpp`

```cpp
// Stores notification for later firing
NotificationId add_notification(Base_Hook* hook, pFrameResult_t res)
{
    // ... generates ID ...
    _notifications.emplace(id, std::move(res));
    return id;
}

// Retrieves stored notifications for firing
std::vector<pFrameResult_t> get_notifications(
    Base_Hook* hook, int32 notification_type)
{
    std::vector<pFrameResult_t> notifs;
    // ... find matching notifications ...
    return notifs;
}
```

**The gap:** Services call `add_notification()` but never call `get_notifications()`.

---

## Recommended Fixes

### Priority 1: Auth Service (Blocks Palworld)

**File:** `eos_dll/eossdk_auth.cpp` & `.h`

**Changes Needed:**

1. Add trigger method to header:
```cpp
class EOSSDK_Auth
{
    // ... existing methods ...

    void trigger_login_status_changed(
        EOS_ELoginStatus previous_status,
        EOS_ELoginStatus current_status
    );
};
```

2. Implement trigger in .cpp:
```cpp
void EOSSDK_Auth::trigger_login_status_changed(
    EOS_ELoginStatus previous_status,
    EOS_ELoginStatus current_status)
{
    auto notifs = std::move(GetCB_Manager().get_notifications(
        this, EOS_Auth_LoginStatusChangedCallbackInfo::k_iCallback));

    for (auto& notif : notifs)
    {
        auto& lscci = notif->GetCallback<EOS_Auth_LoginStatusChangedCallbackInfo>();
        lscci.PreviousStatus = previous_status;
        lscci.CurrentStatus = current_status;
        lscci.LocalUserId = Settings::Inst().userid;
        notif->GetFunc()(notif->GetFuncParam());
    }
}
```

3. Call in `Login()` method after success (around line 135):
```cpp
_logged_in = true;

// Trigger login status changed notification
trigger_login_status_changed(
    EOS_ELoginStatus::EOS_LS_NotLoggedIn,
    EOS_ELoginStatus::EOS_LS_LoggedIn
);
```

4. Fix `AddNotifyLoginStatusChanged()` to populate initial values:
```cpp
EOS_NotificationId AddNotifyLoginStatusChanged(...)
{
    pFrameResult_t res(new FrameResult);
    EOS_Auth_LoginStatusChangedCallbackInfo& lscci =
        res->CreateCallback<...>(...);

    lscci.ClientData = ClientData;
    // Add these:
    lscci.PreviousStatus = EOS_ELoginStatus::EOS_LS_NotLoggedIn;
    lscci.CurrentStatus = _logged_in ?
        EOS_ELoginStatus::EOS_LS_LoggedIn :
        EOS_ELoginStatus::EOS_LS_NotLoggedIn;
    lscci.LocalUserId = Settings::Inst().userid;

    return GetCB_Manager().add_notification(this, res);
}
```

**Impact:** Will fix Palworld and any other game waiting for login status notifications.

---

### Priority 2: Connect Service

**File:** `eos_dll/eossdk_connect.cpp` & `.h`

**Changes Needed:**

Similar to Auth:

1. Add `trigger_login_status_changed()` method
2. Call in `Login()` method (line ~313) after setting `_logged_in = true`
3. Consider implementing auth expiration detection:
   - Use `CBRunFrame()` to check expiration
   - Fire `AuthExpiration` notifications when detected

**Implementation:**

```cpp
// In header
void trigger_login_status_changed(
    EOS_ELoginStatus previous_status,
    EOS_ELoginStatus current_status
);

// In .cpp
void EOSSDK_Connect::trigger_login_status_changed(
    EOS_ELoginStatus previous_status,
    EOS_ELoginStatus current_status)
{
    auto notifs = std::move(GetCB_Manager().get_notifications(
        this, EOS_Connect_LoginStatusChangedCallbackInfo::k_iCallback));

    for (auto& notif : notifs)
    {
        auto& lscci = notif->GetCallback<EOS_Connect_LoginStatusChangedCallbackInfo>();
        lscci.PreviousStatus = previous_status;
        lscci.CurrentStatus = current_status;
        lscci.LocalUserId = Settings::Inst().productuserid;
        notif->GetFunc()(notif->GetFuncParam());
    }
}

// Call in Login() after _logged_in = true
trigger_login_status_changed(
    EOS_ELoginStatus::EOS_LS_NotLoggedIn,
    EOS_ELoginStatus::EOS_LS_LoggedIn
);
```

---

### Priority 3: Presence - JoinGameAccepted

**File:** `eos_dll/eossdk_presence.cpp`

**Changes Needed:**

1. Implement network handler for join game events
2. Add trigger mechanism similar to existing `trigger_presence_change()`

**Status:** Lower priority as most games don't use this feature.

---

### Priority 4: Achievements

**File:** `eos_dll/eossdk_achievements.cpp`

**Changes Needed:**

1. Determine trigger source (local achievement unlock or network sync)
2. Add trigger method following Lobby/Presence pattern
3. Call from appropriate location (likely in `UnlockAchievements()` method)

**Complexity:** Medium - need to determine when to fire (immediate or server-confirmed).

---

### Priority 5: RTCData

**File:** `eos_dll/eossdk_rtcdata.cpp`

**Changes Needed:**

1. Implement network handler for incoming RTC data
2. Fire `DataReceived` notifications when data arrives

**Complexity:** High - requires RTC infrastructure to be functional.

---

### Priority 6: UI - DisplaySettingsUpdated

**File:** `eos_dll/eossdk_ui.cpp`

**Status:** Very low priority - rarely used by games.

---

## Implementation Guide Template

For implementing missing notification triggers, follow this template based on working services:

### Step 1: Add Trigger Method to Header

```cpp
class EOSSDK_YourService
{
    // ... existing ...

    void trigger_your_notification(/* relevant params */);
};
```

### Step 2: Implement Trigger Method

```cpp
void EOSSDK_YourService::trigger_your_notification(/* params */)
{
    // Get stored notifications
    auto notifs = std::move(GetCB_Manager().get_notifications(
        this,
        YourCallbackInfo::k_iCallback
    ));

    // Fire each one
    for (auto& notif : notifs)
    {
        auto& callback_info = notif->GetCallback<YourCallbackInfo>();

        // Populate callback data
        callback_info.Field1 = value1;
        callback_info.Field2 = value2;
        // etc.

        // Fire the callback
        notif->GetFunc()(notif->GetFuncParam());
    }
}
```

### Step 3: Call From Event Source

**Option A: Network Handler**
```cpp
void on_your_network_event(Network_Message_pb const& msg)
{
    // ... process message ...

    trigger_your_notification(/* params */);
}
```

**Option B: State Change**
```cpp
void YourMethod()
{
    // ... do work ...

    _state_changed = true;

    trigger_your_notification(/* params */);
}
```

**Option C: CBRunFrame**
```cpp
bool CBRunFrame()
{
    // Check if notification should fire
    if (should_notify())
    {
        trigger_your_notification(/* params */);
    }

    return true;  // Continue processing
}
```

---

## Testing Recommendations

After implementing fixes:

### Test 1: Auth Login Notification
1. Register `AddNotifyLoginStatusChanged()` callback
2. Call `EOS_Auth_Login()`
3. Verify callback fires with:
   - `PreviousStatus = EOS_LS_NotLoggedIn`
   - `CurrentStatus = EOS_LS_LoggedIn`
   - Valid `LocalUserId`

### Test 2: Connect Login Notification
1. Register `AddNotifyLoginStatusChanged()` callback
2. Call `EOS_Connect_Login()`
3. Verify callback fires with correct status transition

### Test 3: Palworld Integration
1. Launch Palworld with fixed SDK
2. Monitor initialization sequence
3. Verify game reaches online mode
4. Confirm no timeout on login status callbacks

### Test 4: Notification Timing
1. Register notification AFTER login completes
2. Verify notification fires immediately with current state
3. Verify subsequent logins trigger notifications

---

## Related Documentation

- [Callback Architecture](./CALLBACK_ARCHITECTURE.md) - Detailed callback system design
- [Quick Reference](./QUICK_REFERENCE.md) - Service status at a glance
- [Implementation Guides](./IMPLEMENTATION_GUIDES.md) - Step-by-step fixes

---

## Appendix: Callback System Flow

```
Registration Phase:
┌─────────────────────┐
│ Game Code           │
│                     │
│ AddNotifyXXX()      │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Service             │
│                     │
│ CreateCallback()    │
│ add_notification()  │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Callback Manager    │
│                     │
│ _notifications[id]  │◄─── Stored, waiting
└─────────────────────┘


Trigger Phase (WORKING services):
┌─────────────────────┐
│ Network Event /     │
│ State Change /      │
│ Timeout             │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Service             │
│                     │
│ trigger_xxx()       │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Callback Manager    │
│                     │
│ get_notifications() │◄─── Retrieve
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Service             │
│                     │
│ Populate & Fire     │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Game Code           │
│                     │
│ Callback Executed!  │
└─────────────────────┘


Trigger Phase (BROKEN services):
┌─────────────────────┐
│ Network Event /     │
│ State Change /      │
│ Timeout             │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│ Service             │
│                     │
│ ❌ No trigger!      │
└─────────────────────┘


┌─────────────────────┐
│ Callback Manager    │
│                     │
│ _notifications[id]  │◄─── Still stored, never retrieved
└─────────────────────┘


           ❌
┌─────────────────────┐
│ Game Code           │
│                     │
│ Waiting forever...  │
│ → Timeout           │
│ → Offline Mode      │
└─────────────────────┘
```

---

**End of Analysis**
