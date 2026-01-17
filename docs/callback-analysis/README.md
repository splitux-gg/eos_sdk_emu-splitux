# EOS SDK Emulator - Callback Analysis Documentation

**Date:** 2026-01-17
**Branch:** `claude/callback-analysis-docs-R05Xf`

## Overview

This directory contains comprehensive documentation analyzing the callback and notification system across the EOS SDK emulator. The analysis identifies critical gaps that cause games like **Palworld** to fail initialization and fall back to offline mode.

---

## Documentation Files

### 📋 [CALLBACK_GAPS_ANALYSIS.md](./CALLBACK_GAPS_ANALYSIS.md)
**Comprehensive analysis of callback implementation gaps**

- Executive summary of the Palworld offline mode issue
- Complete breakdown of all 17 services (working vs broken)
- Detailed examples from working services (Lobby, Sessions, P2P)
- Detailed breakdown of broken services (Auth, Connect, etc.)
- Root cause analysis
- Priority-based fix recommendations

**Start here if you want to understand the full scope of the problem.**

---

### 🏗️ [CALLBACK_ARCHITECTURE.md](./CALLBACK_ARCHITECTURE.md)
**Technical deep-dive into the callback system design**

- System components (FrameResult, Callback Manager, Base_Hook)
- Callback types (one-time vs persistent notifications)
- Complete flow diagrams (registration → trigger → execution)
- Working patterns from successful implementations
- Common mistakes and how to avoid them
- Best practices for implementation

**Read this to understand how the callback system works internally.**

---

### ⚡ [QUICK_REFERENCE.md](./QUICK_REFERENCE.md)
**At-a-glance service status and quick fixes**

- Service status summary table
- Priority fix order (P1 Critical → P6 Minimal)
- Quick fix code snippets for each broken service
- Testing checklist
- Reference file locations

**Use this for quick lookups and priority planning.**

---

### 🔧 [IMPLEMENTATION_GUIDES.md](./IMPLEMENTATION_GUIDES.md)
**Step-by-step instructions for fixing broken services**

- Detailed Auth service fix (P1 Critical)
- Detailed Connect service fix (P1 Critical)
- Detailed Presence service fix (P3 Medium)
- General implementation template
- Testing guidelines and templates
- Common pitfalls to avoid

**Follow these guides when implementing the fixes.**

---

## Key Findings

### The Palworld Problem

Palworld's initialization sequence:
1. Register notification: `AddNotifyLoginStatusChanged()`
2. Call `EOS_Auth_Login()`
3. Receive login callback ✅
4. **Wait for LoginStatusChanged notification** ❌ **NEVER FIRES**
5. Timeout → Fall back to offline mode

### Root Cause

The Auth and Connect services **register notifications but never trigger them**:

```cpp
// Registration works
EOS_NotificationId AddNotifyLoginStatusChanged(...)
{
    return GetCB_Manager().add_notification(this, res);  // Stored
}

// Login completes
void Login(...)
{
    _logged_in = true;
    // ❌ MISSING: No trigger for stored notifications!
}
```

### Service Status

| Status | Count | Services |
|--------|-------|----------|
| ✅ Working | 3 | Lobby, Sessions, P2P |
| ⚠️ Partial | 2 | Presence, Friends |
| ❌ Broken | 5 | **Auth**, **Connect**, Achievements, UI, RTCData |
| No Notifications | 6 | Leaderboards, Stats, UserInfo, Ecom, Storage services |

---

## Priority Fixes

### P1: CRITICAL (Blocks Palworld)

#### 1. Auth - LoginStatusChanged
- **File:** `eos_dll/eossdk_auth.cpp`
- **Lines:** 135, 449-462
- **Effort:** LOW
- **Impact:** HIGH - Fixes Palworld offline mode

#### 2. Connect - LoginStatusChanged
- **File:** `eos_dll/eossdk_connect.cpp`
- **Lines:** 313, 658-676
- **Effort:** LOW
- **Impact:** HIGH - Fixes online features

### Quick Fix Preview

```cpp
// Add trigger method
void EOSSDK_Auth::trigger_login_status_changed(
    EOS_ELoginStatus prev, EOS_ELoginStatus curr)
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

// Call in Login() after _logged_in = true
trigger_login_status_changed(
    EOS_ELoginStatus::EOS_LS_NotLoggedIn,
    EOS_ELoginStatus::EOS_LS_LoggedIn
);
```

---

## How to Use This Documentation

### If you're trying to fix Palworld:
1. Read the [Quick Reference](./QUICK_REFERENCE.md#auth-service--critical) for Auth fix
2. Follow [Implementation Guide - Auth](./IMPLEMENTATION_GUIDES.md#auth-service-fix)
3. Test with [Testing Guidelines](./IMPLEMENTATION_GUIDES.md#testing-guidelines)

### If you want to understand the problem:
1. Read [Callback Gaps Analysis](./CALLBACK_GAPS_ANALYSIS.md)
2. Review [Callback Architecture](./CALLBACK_ARCHITECTURE.md)
3. See working examples in Gaps Analysis → Working Services

### If you're implementing a new notification:
1. Check [Callback Architecture](./CALLBACK_ARCHITECTURE.md#working-patterns) for patterns
2. Use [Implementation Guides - Template](./IMPLEMENTATION_GUIDES.md#general-implementation-template)
3. Follow [Best Practices](./CALLBACK_ARCHITECTURE.md#best-practices)

### If you need quick answers:
1. Check [Quick Reference](./QUICK_REFERENCE.md) for service status
2. Find file locations and line numbers
3. Copy quick fix code snippets

---

## Working Examples to Reference

### Lobby Service (Network-Triggered)
**File:** `eos_dll/eossdk_lobby.cpp`

Pattern:
- Network message arrives
- Handler calls `notify_lobby_update()`
- Trigger retrieves notifications via `get_notifications()`
- Fires each notification

**Reference lines:** 260-272 (trigger), 449-470 (network handler)

### P2P Service (Hybrid)
**File:** `eos_dll/eossdk_p2p.cpp`

Pattern:
- Network handlers for connection requests
- `CBRunFrame()` for timeout detection
- Both fire notifications using same mechanism

**Reference lines:** 921-930 (periodic), 945-968 (network)

### Presence Service (Working + Broken)
**File:** `eos_dll/eossdk_presence.cpp`

Good example:
- `trigger_presence_change()` (line 670) - works correctly

Bad example:
- `AddNotifyJoinGameAccepted()` (line 688) - never fires

---

## Testing

### After Auth/Connect Fixes

```cpp
// Basic test
1. Register AddNotifyLoginStatusChanged()
2. Call EOS_Auth_Login()
3. Verify notification fires with correct status
4. Confirm PreviousStatus = NotLoggedIn, CurrentStatus = LoggedIn

// Palworld test
1. Launch Palworld with fixed SDK
2. Monitor initialization sequence
3. Verify game reaches online mode (not offline)
4. Test multiplayer features
```

### Regression Testing

Ensure existing working services still function:
- [ ] Lobby notifications
- [ ] Session notifications
- [ ] P2P notifications
- [ ] Presence change notifications

---

## File Locations Reference

### Broken Services (Need Fixes)

```
eos_dll/eossdk_auth.cpp          - Auth service (P1 Critical)
eos_dll/eossdk_auth.h
eos_dll/eossdk_connect.cpp       - Connect service (P1 Critical)
eos_dll/eossdk_connect.h
eos_dll/eossdk_presence.cpp      - Presence service (P3 Medium)
eos_dll/eossdk_presence.h
eos_dll/eossdk_achievements.cpp  - Achievements (P4 Low)
eos_dll/eossdk_achievements.h
eos_dll/eossdk_ui.cpp            - UI (P6 Minimal)
eos_dll/eossdk_ui.h
eos_dll/eossdk_rtcdata.cpp       - RTC Data (P5 Very Low)
eos_dll/eossdk_rtcdata.h
```

### Working Services (Reference Examples)

```
eos_dll/eossdk_lobby.cpp         - Best reference for network-triggered
eos_dll/eossdk_lobby.h
eos_dll/eossdk_sessions.cpp      - Direct firing in handlers
eos_dll/eossdk_sessions.h
eos_dll/eossdk_p2p.cpp           - Hybrid network + periodic
eos_dll/eossdk_p2p.h
eos_dll/eossdk_presence.cpp      - Good example of triggers
eos_dll/eossdk_presence.h
```

### Callback System Core

```
eos_dll/callback_manager.h       - Callback Manager interface
eos_dll/callback_manager.cpp     - Callback Manager implementation
eos_dll/base_hook.h              - Base class for all services
```

---

## Callback Flow Summary

### One-Time Callback (Working)
```
Game calls API
  → SDK creates FrameResult
  → Async work starts
  → Work completes, sets done=true
  → add_callback() queues for execution
  → EOS_Platform_Tick() → run_callbacks()
  → Callback fires
  → Removed from queue
```

### Persistent Notification (Working Pattern)
```
Game calls AddNotifyXXX()
  → SDK stores notification
  → Event occurs (network/state/timeout)
  → Service calls get_notifications()
  → Service populates callback data
  → Service fires each notification
  → Notification remains for future events
```

### Persistent Notification (BROKEN - Auth/Connect)
```
Game calls AddNotifyXXX()
  → SDK stores notification
  → Event occurs (login succeeds)
  → ❌ NO trigger mechanism
  → ❌ get_notifications() never called
  → ❌ Notification never fires
  → Game waits forever
  → Timeout → Offline mode
```

---

## Next Steps

### Immediate (P1 Critical)
1. Implement Auth LoginStatusChanged trigger
2. Implement Connect LoginStatusChanged trigger
3. Test with Palworld
4. Verify online mode works

### Short Term (P3-P4)
1. Fix Presence JoinGameAccepted
2. Fix Achievements notifications
3. Consider Friends service architecture

### Long Term
1. Implement RTCData notifications (requires RTC infrastructure)
2. Consider auth expiration detection
3. Add comprehensive notification tests
4. Document notification contracts for each service

---

## Related Issues

This analysis addresses the core initialization handshake problem preventing Palworld from detecting the online backend. The fixes outlined here should:

- ✅ Allow Palworld to complete online initialization
- ✅ Receive proper login status notifications
- ✅ Recognize the SDK backend as available
- ✅ Enable multiplayer features

---

## Contributing

When implementing fixes:

1. Follow the patterns from working services (Lobby, P2P)
2. Use the implementation template from guides
3. Add TRACE_FUNC() and debug logging
4. Test thoroughly before committing
5. Update this documentation if you find new issues

---

## Questions?

See the detailed documents:
- **What's broken?** → [Callback Gaps Analysis](./CALLBACK_GAPS_ANALYSIS.md)
- **How does it work?** → [Callback Architecture](./CALLBACK_ARCHITECTURE.md)
- **Quick fix?** → [Quick Reference](./QUICK_REFERENCE.md)
- **Step by step?** → [Implementation Guides](./IMPLEMENTATION_GUIDES.md)

---

**Documentation created:** 2026-01-17
**Branch:** claude/callback-analysis-docs-R05Xf
**Related Issue:** Palworld offline mode / initialization handshake failure
