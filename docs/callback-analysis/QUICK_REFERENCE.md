# EOS SDK Emulator - Callback Status Quick Reference

**Date:** 2026-01-17

## Service Status Summary

| Service | Status | Notifications | Working | Broken | Priority |
|---------|--------|--------------|---------|--------|----------|
| **Lobby** | ✅ | 6 | 6 | 0 | - |
| **Sessions** | ✅ | 3 | 3 | 0 | - |
| **P2P** | ✅ | 4 | 4 | 0 | - |
| **Presence** | ⚠️ | 2 | 1 | 1 | P3 |
| **Auth** | ❌ | 2 | 0 | 2 | **P1 CRITICAL** |
| **Connect** | ❌ | 2 | 0 | 2 | **P1 CRITICAL** |
| **Friends** | ⚠️ | 1 | 1* | 0 | P4 |
| **Achievements** | ❌ | 2 | 0 | 2 | P4 |
| **UI** | ❌ | 1 | 0 | 1 | P6 |
| **RTCData** | ❌ | 1 | 0 | 1 | P5 |

*Friends notifications fire but only via Connect service

---

## Legend

- ✅ **Working**: All notifications properly implemented
- ⚠️ **Partial**: Some notifications work, some don't
- ❌ **Broken**: Notifications registered but never fire

---

## Auth Service ❌ CRITICAL

**Priority:** P1 - **BLOCKS PALWORLD**

**File:** `eos_dll/eossdk_auth.cpp` & `.h`

### Broken Notifications

| Notification | Method | Status | Impact |
|--------------|--------|--------|--------|
| LoginStatusChanged | `AddNotifyLoginStatusChanged()` | ❌ Never fires | Palworld offline mode |
| LoginStatusChanged (Old) | `AddNotifyLoginStatusChangedOld()` | ❌ Never fires | Legacy games fail |

### What's Missing

- No trigger mechanism exists
- `CBRunFrame()` returns `false`
- No network handler
- Callback data not populated

### Quick Fix

```cpp
// In eossdk_auth.h
void trigger_login_status_changed(EOS_ELoginStatus prev, EOS_ELoginStatus curr);

// In eossdk_auth.cpp
void EOSSDK_Auth::trigger_login_status_changed(
    EOS_ELoginStatus prev, EOS_ELoginStatus curr)
{
    auto notifs = GetCB_Manager().get_notifications(
        this, EOS_Auth_LoginStatusChangedCallbackInfo::k_iCallback);

    for (auto& notif : notifs) {
        auto& lscci = notif->GetCallback<EOS_Auth_LoginStatusChangedCallbackInfo>();
        lscci.PreviousStatus = prev;
        lscci.CurrentStatus = curr;
        lscci.LocalUserId = Settings::Inst().userid;
        notif->GetFunc()(notif->GetFuncParam());
    }
}

// In Login() method after _logged_in = true
trigger_login_status_changed(
    EOS_ELoginStatus::EOS_LS_NotLoggedIn,
    EOS_ELoginStatus::EOS_LS_LoggedIn
);
```

**Lines to modify:** 135 (add trigger call), 449-462 (fix registration), add new method

---

## Connect Service ❌ CRITICAL

**Priority:** P1 - **BLOCKS ONLINE FEATURES**

**File:** `eos_dll/eossdk_connect.cpp` & `.h`

### Broken Notifications

| Notification | Method | Status | Impact |
|--------------|--------|--------|--------|
| LoginStatusChanged | `AddNotifyLoginStatusChanged()` | ❌ Never fires | Online status unknown |
| AuthExpiration | `AddNotifyAuthExpiration()` | ❌ Never fires | Auth expiry undetected |

### Interesting Note

Connect **does** fire notifications, but only for the **Friends** service (line 903), not its own!

### Quick Fix

Same pattern as Auth:

```cpp
// Add trigger method
void trigger_login_status_changed(EOS_ELoginStatus prev, EOS_ELoginStatus curr);

// Call in Login() method after _logged_in = true (around line 313)
trigger_login_status_changed(
    EOS_ELoginStatus::EOS_LS_NotLoggedIn,
    EOS_ELoginStatus::EOS_LS_LoggedIn
);
```

**Lines to modify:** 313 (add trigger call), 658-676 (registration), add new method

---

## Presence Service ⚠️ PARTIAL

**Priority:** P3 - **ONE NOTIFICATION BROKEN**

**File:** `eos_dll/eossdk_presence.cpp` & `.h`

### Working Notifications ✅

| Notification | Method | Trigger | Status |
|--------------|--------|---------|--------|
| PresenceChanged | `AddNotifyOnPresenceChanged()` | `trigger_presence_change()` | ✅ Works |

### Broken Notifications ❌

| Notification | Method | Status | Impact |
|--------------|--------|--------|--------|
| JoinGameAccepted | `AddNotifyJoinGameAccepted()` | ❌ Never fires | Join game invites broken |

### Quick Fix

Implement trigger for `JoinGameAccepted`:

```cpp
void trigger_join_game_accepted(/* game invite params */);
```

Call when join game invite is accepted (needs network handler).

**Lines to modify:** 688-701 (registration), add trigger method, add network handler

---

## Lobby Service ✅ WORKING

**Reference Implementation**

**File:** `eos_dll/eossdk_lobby.cpp` & `.h`

### All Notifications Working

| Notification | Trigger Method |
|--------------|----------------|
| LobbyUpdateReceived | `notify_lobby_update()` (line 260) |
| LobbyMemberUpdateReceived | `notify_lobby_member_update()` (line 274) |
| LobbyMemberStatusReceived | `notify_lobby_member_status_update()` (line 288) |
| LobbyInviteReceived | `notify_lobby_invite_received()` (line 302) |
| LobbyInviteAccepted | Network handler (line ~800) |
| JoinLobbyAccepted | Network handler (line ~850) |

### Pattern to Follow

```cpp
// Trigger method
void notify_lobby_update(std::string const& lobby_id)
{
    auto notifs = GetCB_Manager().get_notifications(
        this, EOS_Lobby_LobbyUpdateReceivedCallbackInfo::k_iCallback);

    for (auto& notif : notifs) {
        auto& luci = notif->GetCallback<...>();
        luci.LobbyId = lobby_id.c_str();
        notif->GetFunc()(notif->GetFuncParam());
    }
}

// Network handler calls trigger
void on_lobby_updated(Network_Message_pb const& msg)
{
    // Process message
    notify_lobby_update(lobby_id);
}
```

---

## Sessions Service ✅ WORKING

**Reference Implementation**

**File:** `eos_dll/eossdk_sessions.cpp` & `.h`

### All Notifications Working

| Notification | Fired In |
|--------------|----------|
| SessionInviteReceived | `on_session_invite()` (line 1972) |
| SessionInviteAccepted | `on_session_invite_response()` (line 1990) |
| JoinSessionAccepted | Partially implemented |

### Pattern Variation

Sessions fires notifications directly in network handlers instead of separate trigger methods:

```cpp
void on_session_invite(Network_Message_pb const& msg)
{
    // Parse message

    auto notifs = GetCB_Manager().get_notifications(
        this, EOS_Sessions_SessionInviteReceivedCallbackInfo::k_iCallback);

    for (auto& notif : notifs) {
        // Populate and fire
    }
}
```

---

## P2P Service ✅ WORKING

**Reference Implementation**

**File:** `eos_dll/eossdk_p2p.cpp` & `.h`

### All Notifications Working

| Notification | Trigger Source | Line |
|--------------|---------------|------|
| PeerConnectionRequest | `on_p2p_connection_request()` | 945 |
| PeerConnectionEstablished | Network handlers | ~69 |
| PeerConnectionInterrupted | `CBRunFrame()` timeout | 921 |
| PeerConnectionClosed | Multiple handlers | Various |

### Pattern: Hybrid Network + Periodic

```cpp
// Network-triggered
void on_p2p_connection_request(Network_Message_pb const& msg)
{
    auto notifs = GetCB_Manager().get_notifications(...);
    // Fire notifications
}

// Periodic-triggered
bool CBRunFrame() override
{
    // Detect timeouts
    if (connection_timed_out) {
        auto notifs = GetCB_Manager().get_notifications(...);
        // Fire interrupted notifications
    }
    return true;
}
```

---

## Friends Service ⚠️ INDIRECT

**Priority:** P4 - **WORKS BUT UNUSUAL ARCHITECTURE**

**File:** `eos_dll/eossdk_friends.cpp` & `.h`

### How It Works

| Notification | Fired By | Where |
|--------------|----------|-------|
| FriendsUpdate | **Connect** service | `connect.cpp:903` |

### Unusual Pattern

Friends service is **passive**:
- Has no `RunNetwork()` handler
- `CBRunFrame()` returns `false`
- Notifications fired by **Connect** when users authenticate

```cpp
// In eossdk_connect.cpp line 903
void on_user_infos(...)
{
    // Fire FRIENDS notifications (not Connect's!)
    std::vector<pFrameResult_t> notifs = GetCB_Manager().get_notifications(
        &GetEOS_Friends(),  // ← Different service!
        EOS_Friends_OnFriendsUpdateInfo::k_iCallback
    );

    for (auto& notif : notifs) {
        // Fire friend update
    }
}
```

### Question

Is this intentional or should Friends have its own network handlers?

---

## Achievements Service ❌ BROKEN

**Priority:** P4 - **NON-CRITICAL**

**File:** `eos_dll/eossdk_achievements.cpp` & `.h`

### Broken Notifications

| Notification | Method | Status |
|--------------|--------|--------|
| AchievementsUnlocked | `AddNotifyAchievementsUnlocked()` | ❌ Never fires |
| AchievementsUnlockedV2 | `AddNotifyAchievementsUnlockedV2()` | ❌ Never fires |

### What's Missing

- No `RunNetwork()` handler
- `CBRunFrame()` returns `false` (line 286)
- No trigger mechanism

### Recommended Fix

Trigger in `UnlockAchievements()` method after achievement unlocks:

```cpp
void trigger_achievement_unlocked(std::string const& achievement_id);
```

Call when local or network achievement unlock occurs.

---

## UI Service ❌ BROKEN

**Priority:** P6 - **VERY LOW**

**File:** `eos_dll/eossdk_ui.cpp` & `.h`

### Broken Notifications

| Notification | Method | Status |
|--------------|--------|--------|
| DisplaySettingsUpdated | `AddNotifyDisplaySettingsUpdated()` | ❌ Never fires |

### What's Missing

- No mechanism to detect display settings changes
- `CBRunFrame()` returns `false`

### Impact

Minimal - most games don't dynamically change display settings during runtime.

---

## RTCData Service ❌ BROKEN

**Priority:** P5 - **RTC FEATURES**

**File:** `eos_dll/eossdk_rtcdata.cpp` & `.h`

### Broken Notifications

| Notification | Method | Status |
|--------------|--------|--------|
| DataReceived | `AddNotifyDataReceived()` | ❌ Never fires |

### What's Missing

- No `RunNetwork()` handler for incoming RTC data
- `CBRunFrame()` returns `false`

### Impact

RTC data communication completely broken - voice/video data won't work.

---

## Services Without Notifications

These services have no notification system implemented:

| Service | File | Notes |
|---------|------|-------|
| Leaderboards | `eossdk_leaderboards.cpp` | No notifications in API |
| Stats | `eossdk_stats.cpp` | No notifications in API |
| UserInfo | `eossdk_userinfo.cpp` | Query-based only |
| Ecom | `eossdk_ecom.cpp` | No notifications in API |
| PlayerDataStorage | `eossdk_playerdatastorage.cpp` | No notifications in API |
| TitleStorage | `eossdk_titlestorage.cpp` | No notifications in API |

---

## Priority Fix Order

### P1: CRITICAL (Blocks Palworld)

1. **Auth - LoginStatusChanged**
   - File: `eossdk_auth.cpp` lines 135, 449-462
   - Effort: LOW
   - Impact: HIGH - Fixes Palworld

2. **Connect - LoginStatusChanged**
   - File: `eossdk_connect.cpp` lines 313, 658-676
   - Effort: LOW
   - Impact: HIGH - Fixes online features

### P2: HIGH

None currently - P1 fixes should resolve Palworld issue

### P3: MEDIUM

3. **Presence - JoinGameAccepted**
   - File: `eossdk_presence.cpp` lines 688-701
   - Effort: MEDIUM (needs network handler)
   - Impact: MEDIUM - Join game feature

### P4: LOW

4. **Friends - Make self-sufficient**
   - File: `eossdk_friends.cpp`
   - Effort: MEDIUM (architectural change)
   - Impact: LOW - Already works via Connect

5. **Achievements - UnlockNotifications**
   - File: `eossdk_achievements.cpp` lines 214-244
   - Effort: MEDIUM
   - Impact: LOW - Silent achievements still work

### P5: VERY LOW

6. **RTCData - DataReceived**
   - File: `eossdk_rtcdata.cpp` lines 135-149
   - Effort: HIGH (needs RTC infrastructure)
   - Impact: LOW - RTC rarely used

### P6: MINIMAL

7. **UI - DisplaySettingsUpdated**
   - File: `eossdk_ui.cpp` lines 42-55
   - Effort: LOW
   - Impact: MINIMAL - Rarely used

---

## Testing Checklist

### After Auth Fix

- [ ] Register `AddNotifyLoginStatusChanged()` before login
- [ ] Call `EOS_Auth_Login()`
- [ ] Verify notification fires with correct status transition
- [ ] Test Palworld initialization - should reach online mode
- [ ] Register notification AFTER login - should fire immediately

### After Connect Fix

- [ ] Register `AddNotifyLoginStatusChanged()` before Connect login
- [ ] Call `EOS_Connect_Login()`
- [ ] Verify notification fires
- [ ] Test multiple login/logout cycles

### After Presence Fix

- [ ] Register `AddNotifyJoinGameAccepted()`
- [ ] Send join game invite
- [ ] Accept invite
- [ ] Verify notification fires

---

## Reference Files

**Working Examples:**
- `eos_dll/eossdk_lobby.cpp` - lines 260-319 (notification triggers)
- `eos_dll/eossdk_sessions.cpp` - lines 1972-1990 (direct firing)
- `eos_dll/eossdk_p2p.cpp` - lines 921-968 (hybrid pattern)
- `eos_dll/eossdk_presence.cpp` - lines 670-681 (trigger method)

**Broken Examples:**
- `eos_dll/eossdk_auth.cpp` - lines 449-476 (no trigger)
- `eos_dll/eossdk_connect.cpp` - lines 658-676 (no trigger)

**Callback System:**
- `eos_dll/callback_manager.h` - lines 46-67 (manager interface)
- `eos_dll/callback_manager.cpp` - lines 93-114 (get_notifications)

---

## Related Documentation

- [Detailed Gap Analysis](./CALLBACK_GAPS_ANALYSIS.md) - Full analysis with code examples
- [Callback Architecture](./CALLBACK_ARCHITECTURE.md) - How the system works
- [Implementation Guides](./IMPLEMENTATION_GUIDES.md) - Step-by-step fixes

---

**End of Quick Reference**
