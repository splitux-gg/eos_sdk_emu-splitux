/**
 * EOS-LAN Emulator - Stub Functions
 *
 * This file contains stub implementations for all EOS SDK functions
 * that are not yet implemented. Each stub:
 * - Logs the function call
 * - Returns success (or appropriate default value)
 * - Will be replaced with real implementation in subsystem-specific files
 */

#include "eos/eos_sdk.h"
#include "eos/eos_connect.h"
#include "eos/eos_sessions.h"
#include "eos/eos_p2p.h"
#include "eos/eos_auth.h"
#include "eos/eos_friends.h"
#include "eos/eos_presence.h"
#include "eos/eos_userinfo.h"
#include "eos/eos_ecom.h"
#include "eos/eos_ui.h"
#include "eos/eos_lobby.h"
#include "eos/eos_metrics.h"
#include "internal/logging.h"

// ============================================================================
// Platform Management Stubs (remaining functions not in platform.c)
// ============================================================================

EOS_DECLARE_FUNC(EOS_HMetrics) EOS_Platform_GetMetricsInterface(EOS_HPlatform Handle) {
    EOS_LOG_API_ENTER();
    return (EOS_HMetrics)Handle;
}

EOS_DECLARE_FUNC(EOS_HFriends) EOS_Platform_GetFriendsInterface(EOS_HPlatform Handle) {
    EOS_LOG_API_ENTER();
    return (EOS_HFriends)Handle;
}

EOS_DECLARE_FUNC(EOS_HPresence) EOS_Platform_GetPresenceInterface(EOS_HPlatform Handle) {
    EOS_LOG_API_ENTER();
    return (EOS_HPresence)Handle;
}

EOS_DECLARE_FUNC(EOS_HUserInfo) EOS_Platform_GetUserInfoInterface(EOS_HPlatform Handle) {
    EOS_LOG_API_ENTER();
    return (EOS_HUserInfo)Handle;
}

EOS_DECLARE_FUNC(EOS_HEcom) EOS_Platform_GetEcomInterface(EOS_HPlatform Handle) {
    EOS_LOG_API_ENTER();
    return (EOS_HEcom)Handle;
}

EOS_DECLARE_FUNC(EOS_HUI) EOS_Platform_GetUIInterface(EOS_HPlatform Handle) {
    EOS_LOG_API_ENTER();
    return (EOS_HUI)Handle;
}

EOS_DECLARE_FUNC(EOS_HLobby) EOS_Platform_GetLobbyInterface(EOS_HPlatform Handle) {
    EOS_LOG_API_ENTER();
    return (EOS_HLobby)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_CheckForLauncherAndRestart(EOS_HPlatform Handle) {
    EOS_LOG_API_ENTER();
    EOS_LOG_API_RETURN(EOS_Success);
}

