/*
 * Copyright (C) 2020 Nemirtingas
 * This file is part of the Nemirtingas's Epic Emulator
 *
 * Stub implementations for remaining missing EOS SDK functions (part 3)
 */

#include "common_includes.h"

// Forward declarations and type mappings for SDK 1.16+ types

// Lobby types - map to versioned types
#ifndef EOS_Lobby_AddNotifyLobbyInviteRejectedOptions
#define EOS_Lobby_AddNotifyLobbyInviteRejectedOptions EOS_Lobby_AddNotifyLobbyInviteRejectedOptions001
#endif
#ifndef EOS_Lobby_AddNotifyLeaveLobbyRequestedOptions
#define EOS_Lobby_AddNotifyLeaveLobbyRequestedOptions EOS_Lobby_AddNotifyLeaveLobbyRequestedOptions001
#endif
#ifndef EOS_Lobby_AddNotifySendLobbyNativeInviteRequestedOptions
#define EOS_Lobby_AddNotifySendLobbyNativeInviteRequestedOptions EOS_Lobby_AddNotifySendLobbyNativeInviteRequestedOptions001
#endif
#ifndef EOS_Lobby_HardMuteMemberOptions
#define EOS_Lobby_HardMuteMemberOptions EOS_Lobby_HardMuteMemberOptions001
#endif
#ifndef EOS_Lobby_GetConnectStringOptions
#define EOS_Lobby_GetConnectStringOptions EOS_Lobby_GetConnectStringOptions001
#endif
#ifndef EOS_Lobby_ParseConnectStringOptions
#define EOS_Lobby_ParseConnectStringOptions EOS_Lobby_ParseConnectStringOptions001
#endif
#ifndef EOS_Lobby_JoinLobbyByIdOptions
#define EOS_Lobby_JoinLobbyByIdOptions EOS_Lobby_JoinLobbyByIdOptions002
#endif
#ifndef EOS_LobbyModification_SetAllowedPlatformIdsOptions
#define EOS_LobbyModification_SetAllowedPlatformIdsOptions EOS_LobbyModification_SetAllowedPlatformIdsOptions001
#endif
#ifndef EOS_LobbyDetails_CopyMemberInfoOptions
#define EOS_LobbyDetails_CopyMemberInfoOptions EOS_LobbyDetails_CopyMemberInfoOptions001
#endif
#ifndef EOS_LobbyDetails_MemberInfo
#define EOS_LobbyDetails_MemberInfo EOS_LobbyDetails_MemberInfo001
#endif

// Lobby RTC types (new in SDK 1.16+)
#ifndef EOS_LOBBY_JOINRTCROOM_API_LATEST
#define EOS_LOBBY_JOINRTCROOM_API_LATEST 1
EOS_STRUCT(EOS_Lobby_JoinRTCRoomOptions, (
    int32_t ApiVersion;
    EOS_LobbyId LobbyId;
    EOS_ProductUserId LocalUserId;
    EOS_Bool bEnableRTCRoom;
));
typedef struct EOS_Lobby_JoinRTCRoomCallbackInfoInternal {
    EOS_EResult ResultCode;
    void* ClientData;
    EOS_LobbyId LobbyId;
} EOS_Lobby_JoinRTCRoomCallbackInfo;
typedef void (EOS_CALL* EOS_Lobby_OnJoinRTCRoomCallback)(const EOS_Lobby_JoinRTCRoomCallbackInfo* Data);
#endif

#ifndef EOS_LOBBY_LEAVERTCROOM_API_LATEST
#define EOS_LOBBY_LEAVERTCROOM_API_LATEST 1
EOS_STRUCT(EOS_Lobby_LeaveRTCRoomOptions, (
    int32_t ApiVersion;
    EOS_LobbyId LobbyId;
    EOS_ProductUserId LocalUserId;
));
typedef struct EOS_Lobby_LeaveRTCRoomCallbackInfoInternal {
    EOS_EResult ResultCode;
    void* ClientData;
    EOS_LobbyId LobbyId;
} EOS_Lobby_LeaveRTCRoomCallbackInfo;
typedef void (EOS_CALL* EOS_Lobby_OnLeaveRTCRoomCallback)(const EOS_Lobby_LeaveRTCRoomCallbackInfo* Data);
#endif

// Sessions types
#ifndef EOS_SESSIONS_ADDNOTIFYSESSIONINVITEREJECTED_API_LATEST
#define EOS_SESSIONS_ADDNOTIFYSESSIONINVITEREJECTED_API_LATEST 1
EOS_STRUCT(EOS_Sessions_AddNotifySessionInviteRejectedOptions, (int32_t ApiVersion;));
typedef void (EOS_CALL* EOS_Sessions_OnSessionInviteRejectedCallback)(const void* Data);
EOS_STRUCT(EOS_Sessions_AddNotifyLeaveSessionRequestedOptions, (int32_t ApiVersion;));
typedef void (EOS_CALL* EOS_Sessions_OnLeaveSessionRequestedCallback)(const void* Data);
EOS_STRUCT(EOS_Sessions_AddNotifySendSessionNativeInviteRequestedOptions, (int32_t ApiVersion;));
typedef void (EOS_CALL* EOS_Sessions_OnSendSessionNativeInviteRequestedCallback)(const void* Data);
#endif

#ifndef EOS_SESSIONMODIFICATION_SETALLOWEDPLATFORMIDS_API_LATEST
#define EOS_SESSIONMODIFICATION_SETALLOWEDPLATFORMIDS_API_LATEST 1
EOS_STRUCT(EOS_SessionModification_SetAllowedPlatformIdsOptions, (int32_t ApiVersion; const uint32_t* AllowedPlatformIds; uint32_t AllowedPlatformIdsCount;));
#endif

#ifndef EOS_P2P_PacketQueueInfo
#define EOS_P2P_PacketQueueInfo EOS_P2P_PacketQueueInfo001
#endif

// Sanctions types
#ifndef EOS_SANCTIONS_CREATEPLAYERSANCTIONAPPEAL_API_LATEST
#define EOS_SANCTIONS_CREATEPLAYERSANCTIONAPPEAL_API_LATEST 1
EOS_STRUCT(EOS_Sanctions_CreatePlayerSanctionAppealOptions, (
    int32_t ApiVersion;
    EOS_ProductUserId LocalUserId;
    const char* SanctionIdToAppeal;
    const char* Reason;
));
typedef struct EOS_Sanctions_CreatePlayerSanctionAppealCallbackInfoInternal {
    EOS_EResult ResultCode;
    void* ClientData;
} EOS_Sanctions_CreatePlayerSanctionAppealCallbackInfo;
typedef void (EOS_CALL* EOS_Sanctions_OnCreatePlayerSanctionAppealCallback)(const EOS_Sanctions_CreatePlayerSanctionAppealCallbackInfo* Data);
#endif

// UI types
#ifndef EOS_UI_ADDNOTIFYMEMORYMONITOR_API_LATEST
#define EOS_UI_ADDNOTIFYMEMORYMONITOR_API_LATEST 1
EOS_STRUCT(EOS_UI_AddNotifyMemoryMonitorOptions, (int32_t ApiVersion;));
typedef void (EOS_CALL* EOS_UI_OnMemoryMonitorCallback)(const void* Data);
#endif

#ifndef EOS_UI_SETTOGGLEFRIENDSBUTTON_API_LATEST
#define EOS_UI_SETTOGGLEFRIENDSBUTTON_API_LATEST 1
EOS_STRUCT(EOS_UI_SetToggleFriendsButtonOptions, (int32_t ApiVersion; uint32_t ButtonCombination;));
EOS_STRUCT(EOS_UI_GetToggleFriendsButtonOptions, (int32_t ApiVersion;));
#endif

#ifndef EOS_UI_GETFRIENDSEXCLUSIVEINPUT_API_LATEST
#define EOS_UI_GETFRIENDSEXCLUSIVEINPUT_API_LATEST 1
EOS_STRUCT(EOS_UI_GetFriendsExclusiveInputOptions, (int32_t ApiVersion; EOS_EpicAccountId LocalUserId;));
#endif

#ifndef EOS_UI_REPORTINPUTSTATE_API_LATEST
#define EOS_UI_REPORTINPUTSTATE_API_LATEST 1
EOS_STRUCT(EOS_UI_ReportInputStateOptions, (int32_t ApiVersion;));
#endif

#ifndef EOS_UI_ISSOCIALOVERLAYPAUSED_API_LATEST
#define EOS_UI_ISSOCIALOVERLAYPAUSED_API_LATEST 1
EOS_STRUCT(EOS_UI_IsSocialOverlayPausedOptions, (int32_t ApiVersion; EOS_EpicAccountId LocalUserId;));
EOS_STRUCT(EOS_UI_PauseSocialOverlayOptions, (int32_t ApiVersion; EOS_Bool bIsPaused;));
#endif

#ifndef EOS_UI_SHOWBLOCKPLAYER_API_LATEST
#define EOS_UI_SHOWBLOCKPLAYER_API_LATEST 1
EOS_STRUCT(EOS_UI_ShowBlockPlayerOptions, (int32_t ApiVersion; EOS_EpicAccountId LocalUserId; EOS_EpicAccountId TargetUserId;));
typedef struct EOS_UI_ShowBlockPlayerCallbackInfoInternal { EOS_EResult ResultCode; void* ClientData; EOS_EpicAccountId LocalUserId; EOS_EpicAccountId TargetUserId; } EOS_UI_ShowBlockPlayerCallbackInfo;
typedef void (EOS_CALL* EOS_UI_OnShowBlockPlayerCallback)(const EOS_UI_ShowBlockPlayerCallbackInfo* Data);
#endif

#ifndef EOS_UI_SHOWREPORTPLAYER_API_LATEST
#define EOS_UI_SHOWREPORTPLAYER_API_LATEST 1
EOS_STRUCT(EOS_UI_ShowReportPlayerOptions, (int32_t ApiVersion; EOS_EpicAccountId LocalUserId; EOS_EpicAccountId TargetUserId;));
typedef struct EOS_UI_ShowReportPlayerCallbackInfoInternal { EOS_EResult ResultCode; void* ClientData; EOS_EpicAccountId LocalUserId; EOS_EpicAccountId TargetUserId; } EOS_UI_ShowReportPlayerCallbackInfo;
typedef void (EOS_CALL* EOS_UI_OnShowReportPlayerCallback)(const EOS_UI_ShowReportPlayerCallbackInfo* Data);
#endif

#ifndef EOS_UI_SHOWNATIVEPROFILE_API_LATEST
#define EOS_UI_SHOWNATIVEPROFILE_API_LATEST 1
EOS_STRUCT(EOS_UI_ShowNativeProfileOptions, (int32_t ApiVersion; EOS_EpicAccountId LocalUserId; EOS_EpicAccountId TargetUserId;));
typedef struct EOS_UI_ShowNativeProfileCallbackInfoInternal { EOS_EResult ResultCode; void* ClientData; EOS_EpicAccountId LocalUserId; EOS_EpicAccountId TargetUserId; } EOS_UI_ShowNativeProfileCallbackInfo;
typedef void (EOS_CALL* EOS_UI_OnShowNativeProfileCallback)(const EOS_UI_ShowNativeProfileCallbackInfo* Data);
#endif

// ============================================================================
// Lobby Additional Stubs
// ============================================================================

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyInviteRejected(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLobbyInviteRejectedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyInviteRejectedCallback NotificationFn)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyInviteRejected(EOS_HLobby Handle, EOS_NotificationId InId)
{
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLeaveLobbyRequested(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLeaveLobbyRequestedOptions* Options, void* ClientData, const EOS_Lobby_OnLeaveLobbyRequestedCallback NotificationFn)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLeaveLobbyRequested(EOS_HLobby Handle, EOS_NotificationId InId)
{
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifySendLobbyNativeInviteRequested(EOS_HLobby Handle, const EOS_Lobby_AddNotifySendLobbyNativeInviteRequestedOptions* Options, void* ClientData, const EOS_Lobby_OnSendLobbyNativeInviteRequestedCallback NotificationFn)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifySendLobbyNativeInviteRequested(EOS_HLobby Handle, EOS_NotificationId InId)
{
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyRTCRoomConnectionChanged(EOS_HLobby Handle, const EOS_Lobby_AddNotifyRTCRoomConnectionChangedOptions* Options, void* ClientData, const EOS_Lobby_OnRTCRoomConnectionChangedCallback NotificationFn)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyRTCRoomConnectionChanged(EOS_HLobby Handle, EOS_NotificationId InId)
{
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_GetRTCRoomName(EOS_HLobby Handle, const EOS_Lobby_GetRTCRoomNameOptions* Options, char* OutBuffer, uint32_t* InOutBufferLength)
{
    return EOS_EResult::EOS_NotFound;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_IsRTCRoomConnected(EOS_HLobby Handle, const EOS_Lobby_IsRTCRoomConnectedOptions* Options, EOS_Bool* bOutIsConnected)
{
    if (bOutIsConnected)
        *bOutIsConnected = EOS_FALSE;
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_JoinRTCRoom(EOS_HLobby Handle, const EOS_Lobby_JoinRTCRoomOptions* Options, void* ClientData, const EOS_Lobby_OnJoinRTCRoomCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_Lobby_JoinRTCRoomCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_Lobby_LeaveRTCRoom(EOS_HLobby Handle, const EOS_Lobby_LeaveRTCRoomOptions* Options, void* ClientData, const EOS_Lobby_OnLeaveRTCRoomCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_Lobby_LeaveRTCRoomCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_Lobby_HardMuteMember(EOS_HLobby Handle, const EOS_Lobby_HardMuteMemberOptions* Options, void* ClientData, const EOS_Lobby_OnHardMuteMemberCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_Lobby_HardMuteMemberCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_GetConnectString(EOS_HLobby Handle, const EOS_Lobby_GetConnectStringOptions* Options, char* OutBuffer, uint32_t* InOutBufferLength)
{
    return EOS_EResult::EOS_NotFound;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_ParseConnectString(EOS_HLobby Handle, const EOS_Lobby_ParseConnectStringOptions* Options, char* OutBuffer, uint32_t* InOutBufferLength)
{
    return EOS_EResult::EOS_NotFound;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_JoinLobbyById(EOS_HLobby Handle, const EOS_Lobby_JoinLobbyByIdOptions* Options, void* ClientData, const EOS_Lobby_OnJoinLobbyByIdCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_Lobby_JoinLobbyByIdCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_NotFound;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_SetAllowedPlatformIds(EOS_HLobbyModification Handle, const EOS_LobbyModification_SetAllowedPlatformIdsOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyMemberInfo(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyMemberInfoOptions* Options, EOS_LobbyDetails_MemberInfo** OutMemberInfo)
{
    return EOS_EResult::EOS_NotFound;
}

EOS_DECLARE_FUNC(void) EOS_LobbyDetails_MemberInfo_Release(EOS_LobbyDetails_MemberInfo* MemberInfo)
{
}

// ============================================================================
// Sessions Additional Stubs
// ============================================================================

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifySessionInviteRejected(EOS_HSessions Handle, const EOS_Sessions_AddNotifySessionInviteRejectedOptions* Options, void* ClientData, const EOS_Sessions_OnSessionInviteRejectedCallback NotificationFn)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifySessionInviteRejected(EOS_HSessions Handle, EOS_NotificationId InId)
{
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifyLeaveSessionRequested(EOS_HSessions Handle, const EOS_Sessions_AddNotifyLeaveSessionRequestedOptions* Options, void* ClientData, const EOS_Sessions_OnLeaveSessionRequestedCallback NotificationFn)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifyLeaveSessionRequested(EOS_HSessions Handle, EOS_NotificationId InId)
{
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifySendSessionNativeInviteRequested(EOS_HSessions Handle, const EOS_Sessions_AddNotifySendSessionNativeInviteRequestedOptions* Options, void* ClientData, const EOS_Sessions_OnSendSessionNativeInviteRequestedCallback NotificationFn)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifySendSessionNativeInviteRequested(EOS_HSessions Handle, EOS_NotificationId InId)
{
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetAllowedPlatformIds(EOS_HSessionModification Handle, const EOS_SessionModification_SetAllowedPlatformIdsOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

// ============================================================================
// P2P Additional Stubs
// ============================================================================

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_P2P_AddNotifyIncomingPacketQueueFull(EOS_HP2P Handle, const EOS_P2P_AddNotifyIncomingPacketQueueFullOptions* Options, void* ClientData, const EOS_P2P_OnIncomingPacketQueueFullCallback IncomingPacketQueueFullHandler)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_P2P_RemoveNotifyIncomingPacketQueueFull(EOS_HP2P Handle, EOS_NotificationId NotificationId)
{
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_GetPacketQueueInfo(EOS_HP2P Handle, const EOS_P2P_GetPacketQueueInfoOptions* Options, EOS_P2P_PacketQueueInfo* OutPacketQueueInfo)
{
    if (OutPacketQueueInfo)
    {
        memset(OutPacketQueueInfo, 0, sizeof(*OutPacketQueueInfo));
    }
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_SetPacketQueueSize(EOS_HP2P Handle, const EOS_P2P_SetPacketQueueSizeOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_ClearPacketQueue(EOS_HP2P Handle, const EOS_P2P_ClearPacketQueueOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

// ============================================================================
// Mods Stubs
// ============================================================================

EOS_DECLARE_FUNC(void) EOS_Mods_EnumerateMods(EOS_HMods Handle, const EOS_Mods_EnumerateModsOptions* Options, void* ClientData, const EOS_Mods_OnEnumerateModsCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_Mods_EnumerateModsCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Mods_CopyModInfo(EOS_HMods Handle, const EOS_Mods_CopyModInfoOptions* Options, EOS_Mods_ModInfo** OutModInfo)
{
    return EOS_EResult::EOS_NotFound;
}

EOS_DECLARE_FUNC(void) EOS_Mods_ModInfo_Release(EOS_Mods_ModInfo* ModInfo)
{
}

EOS_DECLARE_FUNC(void) EOS_Mods_InstallMod(EOS_HMods Handle, const EOS_Mods_InstallModOptions* Options, void* ClientData, const EOS_Mods_OnInstallModCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_Mods_InstallModCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_Mods_UninstallMod(EOS_HMods Handle, const EOS_Mods_UninstallModOptions* Options, void* ClientData, const EOS_Mods_OnUninstallModCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_Mods_UninstallModCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_Mods_UpdateMod(EOS_HMods Handle, const EOS_Mods_UpdateModOptions* Options, void* ClientData, const EOS_Mods_OnUpdateModCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_Mods_UpdateModCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

// ============================================================================
// Sanctions Stubs
// ============================================================================

EOS_DECLARE_FUNC(void) EOS_Sanctions_QueryActivePlayerSanctions(EOS_HSanctions Handle, const EOS_Sanctions_QueryActivePlayerSanctionsOptions* Options, void* ClientData, const EOS_Sanctions_OnQueryActivePlayerSanctionsCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_Sanctions_QueryActivePlayerSanctionsCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(uint32_t) EOS_Sanctions_GetPlayerSanctionCount(EOS_HSanctions Handle, const EOS_Sanctions_GetPlayerSanctionCountOptions* Options)
{
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sanctions_CopyPlayerSanctionByIndex(EOS_HSanctions Handle, const EOS_Sanctions_CopyPlayerSanctionByIndexOptions* Options, EOS_Sanctions_PlayerSanction** OutSanction)
{
    return EOS_EResult::EOS_NotFound;
}

EOS_DECLARE_FUNC(void) EOS_Sanctions_PlayerSanction_Release(EOS_Sanctions_PlayerSanction* Sanction)
{
}

EOS_DECLARE_FUNC(void) EOS_Sanctions_CreatePlayerSanctionAppeal(EOS_HSanctions Handle, const EOS_Sanctions_CreatePlayerSanctionAppealOptions* Options, void* ClientData, const EOS_Sanctions_OnCreatePlayerSanctionAppealCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_Sanctions_CreatePlayerSanctionAppealCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

// ============================================================================
// Reports Stubs
// ============================================================================

EOS_DECLARE_FUNC(void) EOS_Reports_SendPlayerBehaviorReport(EOS_HReports Handle, const EOS_Reports_SendPlayerBehaviorReportOptions* Options, void* ClientData, const EOS_Reports_OnSendPlayerBehaviorReportCompleteCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_Reports_SendPlayerBehaviorReportCompleteCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

// ============================================================================
// ProgressionSnapshot Stubs
// ============================================================================

EOS_DECLARE_FUNC(EOS_EResult) EOS_ProgressionSnapshot_BeginSnapshot(EOS_HProgressionSnapshot Handle, const EOS_ProgressionSnapshot_BeginSnapshotOptions* Options, uint32_t* OutSnapshotId)
{
    if (OutSnapshotId)
        *OutSnapshotId = 1;
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_ProgressionSnapshot_AddProgression(EOS_HProgressionSnapshot Handle, const EOS_ProgressionSnapshot_AddProgressionOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(void) EOS_ProgressionSnapshot_SubmitSnapshot(EOS_HProgressionSnapshot Handle, const EOS_ProgressionSnapshot_SubmitSnapshotOptions* Options, void* ClientData, const EOS_ProgressionSnapshot_OnSubmitSnapshotCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_ProgressionSnapshot_SubmitSnapshotCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_ProgressionSnapshot_EndSnapshot(EOS_HProgressionSnapshot Handle, const EOS_ProgressionSnapshot_EndSnapshotOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(void) EOS_ProgressionSnapshot_DeleteSnapshot(EOS_HProgressionSnapshot Handle, const EOS_ProgressionSnapshot_DeleteSnapshotOptions* Options, void* ClientData, const EOS_ProgressionSnapshot_OnDeleteSnapshotCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_ProgressionSnapshot_DeleteSnapshotCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

// ============================================================================
// UI Additional Stubs
// ============================================================================

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_UI_AddNotifyMemoryMonitor(EOS_HUI Handle, const EOS_UI_AddNotifyMemoryMonitorOptions* Options, void* ClientData, const EOS_UI_OnMemoryMonitorCallback NotificationFn)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_UI_RemoveNotifyMemoryMonitor(EOS_HUI Handle, EOS_NotificationId Id)
{
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_SetToggleFriendsButton(EOS_HUI Handle, const EOS_UI_SetToggleFriendsButtonOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_UI_EKeyCombination) EOS_UI_GetToggleFriendsButton(EOS_HUI Handle, const EOS_UI_GetToggleFriendsButtonOptions* Options)
{
    return EOS_UI_EKeyCombination::EOS_UIK_None;
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_GetFriendsExclusiveInput(EOS_HUI Handle, const EOS_UI_GetFriendsExclusiveInputOptions* Options)
{
    return EOS_FALSE;
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_IsValidButtonCombination(EOS_HUI Handle, EOS_UI_EKeyCombination ButtonCombination)
{
    return EOS_TRUE;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_ReportInputState(EOS_HUI Handle, const EOS_UI_ReportInputStateOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_PrePresent(EOS_HUI Handle, const EOS_UI_PrePresentOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_IsSocialOverlayPaused(EOS_HUI Handle, const EOS_UI_IsSocialOverlayPausedOptions* Options)
{
    return EOS_FALSE;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_PauseSocialOverlay(EOS_HUI Handle, const EOS_UI_PauseSocialOverlayOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(void) EOS_UI_ShowBlockPlayer(EOS_HUI Handle, const EOS_UI_ShowBlockPlayerOptions* Options, void* ClientData, const EOS_UI_OnShowBlockPlayerCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_UI_ShowBlockPlayerCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_UI_ShowReportPlayer(EOS_HUI Handle, const EOS_UI_ShowReportPlayerOptions* Options, void* ClientData, const EOS_UI_OnShowReportPlayerCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_UI_ShowReportPlayerCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_UI_ShowNativeProfile(EOS_HUI Handle, const EOS_UI_ShowNativeProfileOptions* Options, void* ClientData, const EOS_UI_OnShowNativeProfileCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_UI_ShowNativeProfileCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

// ============================================================================
// UserInfo Additional Stubs
// ============================================================================

EOS_DECLARE_FUNC(EOS_EResult) EOS_UserInfo_CopyBestDisplayName(EOS_HUserInfo Handle, const EOS_UserInfo_CopyBestDisplayNameOptions* Options, EOS_UserInfo_BestDisplayName** OutBestDisplayName)
{
    return EOS_EResult::EOS_NotFound;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UserInfo_CopyBestDisplayNameWithPlatform(EOS_HUserInfo Handle, const EOS_UserInfo_CopyBestDisplayNameWithPlatformOptions* Options, EOS_UserInfo_BestDisplayName** OutBestDisplayName)
{
    return EOS_EResult::EOS_NotFound;
}

EOS_DECLARE_FUNC(void) EOS_UserInfo_BestDisplayName_Release(EOS_UserInfo_BestDisplayName* BestDisplayName)
{
}

EOS_DECLARE_FUNC(uint32_t) EOS_UserInfo_GetLocalPlatformType(EOS_HUserInfo Handle, const EOS_UserInfo_GetLocalPlatformTypeOptions* Options)
{
    return 0;
}

// ============================================================================
// PlayerDataStorage Additional Stubs
// ============================================================================

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorage_DeleteCache(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_DeleteCacheOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

// ============================================================================
// Mercury Stubs (internal/deprecated)
// ============================================================================

EOS_DECLARE_FUNC(EOS_EResult) EOS_Mercury_Initialize()
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(void) EOS_Mercury_Shutdown()
{
}

EOS_DECLARE_FUNC(void) EOS_Mercury_Tick()
{
}
