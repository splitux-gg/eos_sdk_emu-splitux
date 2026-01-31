/**
 * EOS-LAN Emulator - Auto-generated Stub Functions
 * Generated from EOS SDK headers
 * Total stubs: 427
 */

#include "eos/eos_sdk.h"
#include "eos/eos_achievements.h"
#include "eos/eos_stats.h"
#include "eos/eos_leaderboards.h"
#include "eos/eos_anticheatclient.h"
#include "eos/eos_anticheatserver.h"
#include "eos/eos_reports.h"
#include "eos/eos_sanctions.h"
#include "eos/eos_mods.h"
#include "eos/eos_ecom.h"
#include "eos/eos_friends.h"
#include "eos/eos_presence.h"
#include "eos/eos_userinfo.h"
#include "eos/eos_ui.h"
#include "eos/eos_lobby.h"
#include "eos/eos_playerdatastorage.h"
#include "eos/eos_titlestorage.h"
#include "eos/eos_kws.h"
#include "eos/eos_rtc.h"
#include "eos/eos_rtc_audio.h"
#include "eos/eos_rtc_data.h"
#include "eos/eos_rtc_admin.h"
#include "eos/eos_progressionsnapshot.h"
#include "eos/eos_custominvites.h"
#include "eos/eos_integratedplatform.h"
#include "eos/eos_metrics.h"

// ============ Achievements ============

EOS_DECLARE_FUNC(void) EOS_Achievements_QueryDefinitions(EOS_HAchievements Handle, const EOS_Achievements_QueryDefinitionsOptions* Options, void* ClientData, const EOS_Achievements_OnQueryDefinitionsCompleteCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Achievements_GetAchievementDefinitionCount(EOS_HAchievements Handle, const EOS_Achievements_GetAchievementDefinitionCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyAchievementDefinitionV2ByIndex(EOS_HAchievements Handle, const EOS_Achievements_CopyAchievementDefinitionV2ByIndexOptions* Options, EOS_Achievements_DefinitionV2 ** OutDefinition) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyAchievementDefinitionV2ByAchievementId(EOS_HAchievements Handle, const EOS_Achievements_CopyAchievementDefinitionV2ByAchievementIdOptions* Options, EOS_Achievements_DefinitionV2 ** OutDefinition) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_Achievements_QueryPlayerAchievements(EOS_HAchievements Handle, const EOS_Achievements_QueryPlayerAchievementsOptions* Options, void* ClientData, const EOS_Achievements_OnQueryPlayerAchievementsCompleteCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Achievements_GetPlayerAchievementCount(EOS_HAchievements Handle, const EOS_Achievements_GetPlayerAchievementCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyPlayerAchievementByIndex(EOS_HAchievements Handle, const EOS_Achievements_CopyPlayerAchievementByIndexOptions* Options, EOS_Achievements_PlayerAchievement ** OutAchievement) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyPlayerAchievementByAchievementId(EOS_HAchievements Handle, const EOS_Achievements_CopyPlayerAchievementByAchievementIdOptions* Options, EOS_Achievements_PlayerAchievement ** OutAchievement) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_Achievements_UnlockAchievements(EOS_HAchievements Handle, const EOS_Achievements_UnlockAchievementsOptions* Options, void* ClientData, const EOS_Achievements_OnUnlockAchievementsCompleteCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Achievements_AddNotifyAchievementsUnlockedV2(EOS_HAchievements Handle, const EOS_Achievements_AddNotifyAchievementsUnlockedV2Options* Options, void* ClientData, const EOS_Achievements_OnAchievementsUnlockedCallbackV2 NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Achievements_RemoveNotifyAchievementsUnlocked(EOS_HAchievements Handle, EOS_NotificationId InId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Achievements_AddNotifyAchievementsUnlocked(EOS_HAchievements Handle, const EOS_Achievements_AddNotifyAchievementsUnlockedOptions* Options, void* ClientData, const EOS_Achievements_OnAchievementsUnlockedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

// ============ AntiCheatClient ============

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_AntiCheatClient_AddNotifyMessageToServer(EOS_HAntiCheatClient Handle, const EOS_AntiCheatClient_AddNotifyMessageToServerOptions* Options, void* ClientData, EOS_AntiCheatClient_OnMessageToServerCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_AntiCheatClient_RemoveNotifyMessageToServer(EOS_HAntiCheatClient Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_AntiCheatClient_AddNotifyMessageToPeer(EOS_HAntiCheatClient Handle, const EOS_AntiCheatClient_AddNotifyMessageToPeerOptions* Options, void* ClientData, EOS_AntiCheatClient_OnMessageToPeerCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_AntiCheatClient_RemoveNotifyMessageToPeer(EOS_HAntiCheatClient Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_AntiCheatClient_AddNotifyPeerActionRequired(EOS_HAntiCheatClient Handle, const EOS_AntiCheatClient_AddNotifyPeerActionRequiredOptions* Options, void* ClientData, EOS_AntiCheatClient_OnPeerActionRequiredCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_AntiCheatClient_RemoveNotifyPeerActionRequired(EOS_HAntiCheatClient Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_AntiCheatClient_AddNotifyPeerAuthStatusChanged(EOS_HAntiCheatClient Handle, const EOS_AntiCheatClient_AddNotifyPeerAuthStatusChangedOptions* Options, void* ClientData, EOS_AntiCheatClient_OnPeerAuthStatusChangedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_AntiCheatClient_RemoveNotifyPeerAuthStatusChanged(EOS_HAntiCheatClient Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_AntiCheatClient_AddNotifyClientIntegrityViolated(EOS_HAntiCheatClient Handle, const EOS_AntiCheatClient_AddNotifyClientIntegrityViolatedOptions* Options, void* ClientData, EOS_AntiCheatClient_OnClientIntegrityViolatedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_AntiCheatClient_RemoveNotifyClientIntegrityViolated(EOS_HAntiCheatClient Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatClient_BeginSession(EOS_HAntiCheatClient Handle, const EOS_AntiCheatClient_BeginSessionOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatClient_EndSession(EOS_HAntiCheatClient Handle, const EOS_AntiCheatClient_EndSessionOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatClient_PollStatus(EOS_HAntiCheatClient Handle, const EOS_AntiCheatClient_PollStatusOptions* Options, EOS_EAntiCheatClientViolationType* OutViolationType, char* OutMessage) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatClient_Reserved01(EOS_HAntiCheatClient Handle, const EOS_AntiCheatClient_Reserved01Options* Options, int32_t* OutValue) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatClient_AddExternalIntegrityCatalog(EOS_HAntiCheatClient Handle, const EOS_AntiCheatClient_AddExternalIntegrityCatalogOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatClient_ReceiveMessageFromServer(EOS_HAntiCheatClient Handle, const EOS_AntiCheatClient_ReceiveMessageFromServerOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatClient_GetProtectMessageOutputLength(EOS_HAntiCheatClient Handle, const EOS_AntiCheatClient_GetProtectMessageOutputLengthOptions* Options, uint32_t* OutBufferSizeBytes) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatClient_ProtectMessage(EOS_HAntiCheatClient Handle, const EOS_AntiCheatClient_ProtectMessageOptions* Options, void* OutBuffer, uint32_t* OutBytesWritten) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatClient_UnprotectMessage(EOS_HAntiCheatClient Handle, const EOS_AntiCheatClient_UnprotectMessageOptions* Options, void* OutBuffer, uint32_t* OutBytesWritten) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatClient_RegisterPeer(EOS_HAntiCheatClient Handle, const EOS_AntiCheatClient_RegisterPeerOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatClient_UnregisterPeer(EOS_HAntiCheatClient Handle, const EOS_AntiCheatClient_UnregisterPeerOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatClient_ReceiveMessageFromPeer(EOS_HAntiCheatClient Handle, const EOS_AntiCheatClient_ReceiveMessageFromPeerOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

// ============ AntiCheatServer ============

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_AntiCheatServer_AddNotifyMessageToClient(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_AddNotifyMessageToClientOptions* Options, void* ClientData, EOS_AntiCheatServer_OnMessageToClientCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_AntiCheatServer_RemoveNotifyMessageToClient(EOS_HAntiCheatServer Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_AntiCheatServer_AddNotifyClientActionRequired(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_AddNotifyClientActionRequiredOptions* Options, void* ClientData, EOS_AntiCheatServer_OnClientActionRequiredCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_AntiCheatServer_RemoveNotifyClientActionRequired(EOS_HAntiCheatServer Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_AntiCheatServer_AddNotifyClientAuthStatusChanged(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_AddNotifyClientAuthStatusChangedOptions* Options, void* ClientData, EOS_AntiCheatServer_OnClientAuthStatusChangedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_AntiCheatServer_RemoveNotifyClientAuthStatusChanged(EOS_HAntiCheatServer Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_BeginSession(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_BeginSessionOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_EndSession(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_EndSessionOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_RegisterClient(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_RegisterClientOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_UnregisterClient(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_UnregisterClientOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_ReceiveMessageFromClient(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_ReceiveMessageFromClientOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_SetClientDetails(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_SetClientDetailsOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_SetGameSessionId(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_SetGameSessionIdOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_SetClientNetworkState(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_SetClientNetworkStateOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_GetProtectMessageOutputLength(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_GetProtectMessageOutputLengthOptions* Options, uint32_t* OutBufferSizeBytes) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_ProtectMessage(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_ProtectMessageOptions* Options, void* OutBuffer, uint32_t* OutBytesWritten) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_UnprotectMessage(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_UnprotectMessageOptions* Options, void* OutBuffer, uint32_t* OutBytesWritten) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_RegisterEvent(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_RegisterEventOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_LogEvent(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_LogEventOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_LogGameRoundStart(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_LogGameRoundStartOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_LogGameRoundEnd(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_LogGameRoundEndOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_LogPlayerSpawn(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_LogPlayerSpawnOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_LogPlayerDespawn(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_LogPlayerDespawnOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_LogPlayerRevive(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_LogPlayerReviveOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_LogPlayerTick(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_LogPlayerTickOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_LogPlayerUseWeapon(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_LogPlayerUseWeaponOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_LogPlayerUseAbility(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_LogPlayerUseAbilityOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_LogPlayerTakeDamage(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_LogPlayerTakeDamageOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

// ============ CustomInvites ============

EOS_DECLARE_FUNC(EOS_EResult) EOS_CustomInvites_SetCustomInvite(EOS_HCustomInvites Handle, const EOS_CustomInvites_SetCustomInviteOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_SendCustomInvite(EOS_HCustomInvites Handle, const EOS_CustomInvites_SendCustomInviteOptions* Options, void* ClientData, const EOS_CustomInvites_OnSendCustomInviteCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifyCustomInviteReceived(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifyCustomInviteReceivedOptions* Options, void* ClientData, const EOS_CustomInvites_OnCustomInviteReceivedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifyCustomInviteReceived(EOS_HCustomInvites Handle, EOS_NotificationId InId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifyCustomInviteAccepted(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifyCustomInviteAcceptedOptions* Options, void* ClientData, const EOS_CustomInvites_OnCustomInviteAcceptedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifyCustomInviteAccepted(EOS_HCustomInvites Handle, EOS_NotificationId InId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifyCustomInviteRejected(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifyCustomInviteRejectedOptions* Options, void* ClientData, const EOS_CustomInvites_OnCustomInviteRejectedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifyCustomInviteRejected(EOS_HCustomInvites Handle, EOS_NotificationId InId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_CustomInvites_FinalizeInvite(EOS_HCustomInvites Handle, const EOS_CustomInvites_FinalizeInviteOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_SendRequestToJoin(EOS_HCustomInvites Handle, const EOS_CustomInvites_SendRequestToJoinOptions* Options, void* ClientData, const EOS_CustomInvites_OnSendRequestToJoinCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifyRequestToJoinResponseReceived(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifyRequestToJoinResponseReceivedOptions* Options, void* ClientData, const EOS_CustomInvites_OnRequestToJoinResponseReceivedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifyRequestToJoinResponseReceived(EOS_HCustomInvites Handle, EOS_NotificationId InId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifyRequestToJoinReceived(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifyRequestToJoinReceivedOptions* Options, void* ClientData, const EOS_CustomInvites_OnRequestToJoinReceivedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifyRequestToJoinReceived(EOS_HCustomInvites Handle, EOS_NotificationId InId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifySendCustomNativeInviteRequested(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifySendCustomNativeInviteRequestedOptions* Options, void* ClientData, const EOS_CustomInvites_OnSendCustomNativeInviteRequestedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifySendCustomNativeInviteRequested(EOS_HCustomInvites Handle, EOS_NotificationId InId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifyRequestToJoinAccepted(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifyRequestToJoinAcceptedOptions* Options, void* ClientData, const EOS_CustomInvites_OnRequestToJoinAcceptedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifyRequestToJoinAccepted(EOS_HCustomInvites Handle, EOS_NotificationId InId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifyRequestToJoinRejected(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifyRequestToJoinRejectedOptions* Options, void* ClientData, const EOS_CustomInvites_OnRequestToJoinRejectedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifyRequestToJoinRejected(EOS_HCustomInvites Handle, EOS_NotificationId InId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_AcceptRequestToJoin(EOS_HCustomInvites Handle, const EOS_CustomInvites_AcceptRequestToJoinOptions* Options, void* ClientData, const EOS_CustomInvites_OnAcceptRequestToJoinCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RejectRequestToJoin(EOS_HCustomInvites Handle, const EOS_CustomInvites_RejectRequestToJoinOptions* Options, void* ClientData, const EOS_CustomInvites_OnRejectRequestToJoinCallback CompletionDelegate) {
    (void)Handle;
}

// ============ Ecom ============

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOwnership(EOS_HEcom Handle, const EOS_Ecom_QueryOwnershipOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOwnershipBySandboxIds(EOS_HEcom Handle, const EOS_Ecom_QueryOwnershipBySandboxIdsOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipBySandboxIdsCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOwnershipToken(EOS_HEcom Handle, const EOS_Ecom_QueryOwnershipTokenOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipTokenCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryEntitlements(EOS_HEcom Handle, const EOS_Ecom_QueryEntitlementsOptions* Options, void* ClientData, const EOS_Ecom_OnQueryEntitlementsCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryEntitlementToken(EOS_HEcom Handle, const EOS_Ecom_QueryEntitlementTokenOptions* Options, void* ClientData, const EOS_Ecom_OnQueryEntitlementTokenCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOffers(EOS_HEcom Handle, const EOS_Ecom_QueryOffersOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOffersCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Ecom_Checkout(EOS_HEcom Handle, const EOS_Ecom_CheckoutOptions* Options, void* ClientData, const EOS_Ecom_OnCheckoutCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Ecom_RedeemEntitlements(EOS_HEcom Handle, const EOS_Ecom_RedeemEntitlementsOptions* Options, void* ClientData, const EOS_Ecom_OnRedeemEntitlementsCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetLastRedeemedEntitlementsCount(EOS_HEcom Handle, const EOS_Ecom_GetLastRedeemedEntitlementsCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyLastRedeemedEntitlementByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyLastRedeemedEntitlementByIndexOptions* Options, char* OutRedeemedEntitlementId, int32_t* InOutRedeemedEntitlementIdLength) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetEntitlementsCount(EOS_HEcom Handle, const EOS_Ecom_GetEntitlementsCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetEntitlementsByNameCount(EOS_HEcom Handle, const EOS_Ecom_GetEntitlementsByNameCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyEntitlementByIndexOptions* Options, EOS_Ecom_Entitlement ** OutEntitlement) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementByNameAndIndex(EOS_HEcom Handle, const EOS_Ecom_CopyEntitlementByNameAndIndexOptions* Options, EOS_Ecom_Entitlement ** OutEntitlement) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementById(EOS_HEcom Handle, const EOS_Ecom_CopyEntitlementByIdOptions* Options, EOS_Ecom_Entitlement ** OutEntitlement) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferCount(EOS_HEcom Handle, const EOS_Ecom_GetOfferCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyOfferByIndexOptions* Options, EOS_Ecom_CatalogOffer ** OutOffer) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferById(EOS_HEcom Handle, const EOS_Ecom_CopyOfferByIdOptions* Options, EOS_Ecom_CatalogOffer ** OutOffer) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferItemCount(EOS_HEcom Handle, const EOS_Ecom_GetOfferItemCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferItemByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyOfferItemByIndexOptions* Options, EOS_Ecom_CatalogItem ** OutItem) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyItemById(EOS_HEcom Handle, const EOS_Ecom_CopyItemByIdOptions* Options, EOS_Ecom_CatalogItem ** OutItem) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferImageInfoCount(EOS_HEcom Handle, const EOS_Ecom_GetOfferImageInfoCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferImageInfoByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyOfferImageInfoByIndexOptions* Options, EOS_Ecom_KeyImageInfo ** OutImageInfo) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetItemImageInfoCount(EOS_HEcom Handle, const EOS_Ecom_GetItemImageInfoCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyItemImageInfoByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyItemImageInfoByIndexOptions* Options, EOS_Ecom_KeyImageInfo ** OutImageInfo) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetItemReleaseCount(EOS_HEcom Handle, const EOS_Ecom_GetItemReleaseCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyItemReleaseByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyItemReleaseByIndexOptions* Options, EOS_Ecom_CatalogRelease ** OutRelease) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetTransactionCount(EOS_HEcom Handle, const EOS_Ecom_GetTransactionCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyTransactionByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyTransactionByIndexOptions* Options, EOS_Ecom_HTransaction* OutTransaction) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyTransactionById(EOS_HEcom Handle, const EOS_Ecom_CopyTransactionByIdOptions* Options, EOS_Ecom_HTransaction* OutTransaction) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_Transaction_GetTransactionId(EOS_Ecom_HTransaction Handle, char* OutBuffer, int32_t* InOutBufferLength) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_Transaction_GetEntitlementsCount(EOS_Ecom_HTransaction Handle, const EOS_Ecom_Transaction_GetEntitlementsCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_Transaction_CopyEntitlementByIndex(EOS_Ecom_HTransaction Handle, const EOS_Ecom_Transaction_CopyEntitlementByIndexOptions* Options, EOS_Ecom_Entitlement ** OutEntitlement) {
    (void)Handle;
    return EOS_NotConfigured;
}

// ============ Friends ============

EOS_DECLARE_FUNC(void) EOS_Friends_QueryFriends(EOS_HFriends Handle, const EOS_Friends_QueryFriendsOptions* Options, void* ClientData, const EOS_Friends_OnQueryFriendsCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Friends_SendInvite(EOS_HFriends Handle, const EOS_Friends_SendInviteOptions* Options, void* ClientData, const EOS_Friends_OnSendInviteCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Friends_AcceptInvite(EOS_HFriends Handle, const EOS_Friends_AcceptInviteOptions* Options, void* ClientData, const EOS_Friends_OnAcceptInviteCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Friends_RejectInvite(EOS_HFriends Handle, const EOS_Friends_RejectInviteOptions* Options, void* ClientData, const EOS_Friends_OnRejectInviteCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(int32_t) EOS_Friends_GetFriendsCount(EOS_HFriends Handle, const EOS_Friends_GetFriendsCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_Friends_GetFriendAtIndex(EOS_HFriends Handle, const EOS_Friends_GetFriendAtIndexOptions* Options) {
    (void)Handle;
    return (EOS_EpicAccountId)0;
}

EOS_DECLARE_FUNC(EOS_EFriendsStatus) EOS_Friends_GetStatus(EOS_HFriends Handle, const EOS_Friends_GetStatusOptions* Options) {
    (void)Handle;
    return EOS_FS_NotFriends;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Friends_AddNotifyFriendsUpdate(EOS_HFriends Handle, const EOS_Friends_AddNotifyFriendsUpdateOptions* Options, void* ClientData, const EOS_Friends_OnFriendsUpdateCallback FriendsUpdateHandler) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Friends_RemoveNotifyFriendsUpdate(EOS_HFriends Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(int32_t) EOS_Friends_GetBlockedUsersCount(EOS_HFriends Handle, const EOS_Friends_GetBlockedUsersCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_Friends_GetBlockedUserAtIndex(EOS_HFriends Handle, const EOS_Friends_GetBlockedUserAtIndexOptions* Options) {
    (void)Handle;
    return (EOS_EpicAccountId)0;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Friends_AddNotifyBlockedUsersUpdate(EOS_HFriends Handle, const EOS_Friends_AddNotifyBlockedUsersUpdateOptions* Options, void* ClientData, const EOS_Friends_OnBlockedUsersUpdateCallback BlockedUsersUpdateHandler) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Friends_RemoveNotifyBlockedUsersUpdate(EOS_HFriends Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

// ============ IntegratedPlatform ============

EOS_DECLARE_FUNC(EOS_EResult) EOS_IntegratedPlatform_SetUserLoginStatus(EOS_HIntegratedPlatform Handle, const EOS_IntegratedPlatform_SetUserLoginStatusOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_IntegratedPlatform_AddNotifyUserLoginStatusChanged(EOS_HIntegratedPlatform Handle, const EOS_IntegratedPlatform_AddNotifyUserLoginStatusChangedOptions* Options, void* ClientData, const EOS_IntegratedPlatform_OnUserLoginStatusChangedCallback CallbackFunction) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_IntegratedPlatform_RemoveNotifyUserLoginStatusChanged(EOS_HIntegratedPlatform Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_IntegratedPlatform_SetUserPreLogoutCallback(EOS_HIntegratedPlatform Handle, const EOS_IntegratedPlatform_SetUserPreLogoutCallbackOptions* Options, void* ClientData, EOS_IntegratedPlatform_OnUserPreLogoutCallback CallbackFunction) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_IntegratedPlatform_ClearUserPreLogoutCallback(EOS_HIntegratedPlatform Handle, const EOS_IntegratedPlatform_ClearUserPreLogoutCallbackOptions* Options) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_IntegratedPlatform_FinalizeDeferredUserLogout(EOS_HIntegratedPlatform Handle, const EOS_IntegratedPlatform_FinalizeDeferredUserLogoutOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

// ============ IntegratedPlatformOptionsContainer ============

EOS_DECLARE_FUNC(EOS_EResult) EOS_IntegratedPlatformOptionsContainer_Add(EOS_HIntegratedPlatformOptionsContainer Handle, const EOS_IntegratedPlatformOptionsContainer_AddOptions* InOptions) {
    (void)Handle;
    return EOS_NotConfigured;
}

// ============ KWS ============

EOS_DECLARE_FUNC(void) EOS_KWS_QueryAgeGate(EOS_HKWS Handle, const EOS_KWS_QueryAgeGateOptions* Options, void* ClientData, const EOS_KWS_OnQueryAgeGateCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_KWS_CreateUser(EOS_HKWS Handle, const EOS_KWS_CreateUserOptions* Options, void* ClientData, const EOS_KWS_OnCreateUserCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_KWS_QueryPermissions(EOS_HKWS Handle, const EOS_KWS_QueryPermissionsOptions* Options, void* ClientData, const EOS_KWS_OnQueryPermissionsCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_KWS_UpdateParentEmail(EOS_HKWS Handle, const EOS_KWS_UpdateParentEmailOptions* Options, void* ClientData, const EOS_KWS_OnUpdateParentEmailCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_KWS_RequestPermissions(EOS_HKWS Handle, const EOS_KWS_RequestPermissionsOptions* Options, void* ClientData, const EOS_KWS_OnRequestPermissionsCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(int32_t) EOS_KWS_GetPermissionsCount(EOS_HKWS Handle, const EOS_KWS_GetPermissionsCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_KWS_CopyPermissionByIndex(EOS_HKWS Handle, const EOS_KWS_CopyPermissionByIndexOptions* Options, EOS_KWS_PermissionStatus ** OutPermission) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_KWS_GetPermissionByKey(EOS_HKWS Handle, const EOS_KWS_GetPermissionByKeyOptions* Options, EOS_EKWSPermissionStatus* OutPermission) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_KWS_AddNotifyPermissionsUpdateReceived(EOS_HKWS Handle, const EOS_KWS_AddNotifyPermissionsUpdateReceivedOptions* Options, void* ClientData, const EOS_KWS_OnPermissionsUpdateReceivedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_KWS_RemoveNotifyPermissionsUpdateReceived(EOS_HKWS Handle, EOS_NotificationId InId) {
    (void)Handle;
}

// ============ Leaderboards ============

EOS_DECLARE_FUNC(void) EOS_Leaderboards_QueryLeaderboardDefinitions(EOS_HLeaderboards Handle, const EOS_Leaderboards_QueryLeaderboardDefinitionsOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Leaderboards_GetLeaderboardDefinitionCount(EOS_HLeaderboards Handle, const EOS_Leaderboards_GetLeaderboardDefinitionCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardDefinitionByIndex(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardDefinitionByIndexOptions* Options, EOS_Leaderboards_Definition ** OutLeaderboardDefinition) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardIdOptions* Options, EOS_Leaderboards_Definition ** OutLeaderboardDefinition) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_Leaderboards_QueryLeaderboardRanks(EOS_HLeaderboards Handle, const EOS_Leaderboards_QueryLeaderboardRanksOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardRanksCompleteCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Leaderboards_GetLeaderboardRecordCount(EOS_HLeaderboards Handle, const EOS_Leaderboards_GetLeaderboardRecordCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardRecordByIndex(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardRecordByIndexOptions* Options, EOS_Leaderboards_LeaderboardRecord ** OutLeaderboardRecord) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardRecordByUserId(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardRecordByUserIdOptions* Options, EOS_Leaderboards_LeaderboardRecord ** OutLeaderboardRecord) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_Leaderboards_QueryLeaderboardUserScores(EOS_HLeaderboards Handle, const EOS_Leaderboards_QueryLeaderboardUserScoresOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardUserScoresCompleteCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Leaderboards_GetLeaderboardUserScoreCount(EOS_HLeaderboards Handle, const EOS_Leaderboards_GetLeaderboardUserScoreCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardUserScoreByIndex(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardUserScoreByIndexOptions* Options, EOS_Leaderboards_LeaderboardUserScore ** OutLeaderboardUserScore) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardUserScoreByUserId(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardUserScoreByUserIdOptions* Options, EOS_Leaderboards_LeaderboardUserScore ** OutLeaderboardUserScore) {
    (void)Handle;
    return EOS_NotConfigured;
}

// ============ Lobby ============

EOS_DECLARE_FUNC(void) EOS_Lobby_CreateLobby(EOS_HLobby Handle, const EOS_Lobby_CreateLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnCreateLobbyCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_DestroyLobby(EOS_HLobby Handle, const EOS_Lobby_DestroyLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnDestroyLobbyCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_JoinLobby(EOS_HLobby Handle, const EOS_Lobby_JoinLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnJoinLobbyCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_JoinLobbyById(EOS_HLobby Handle, const EOS_Lobby_JoinLobbyByIdOptions* Options, void* ClientData, const EOS_Lobby_OnJoinLobbyByIdCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_LeaveLobby(EOS_HLobby Handle, const EOS_Lobby_LeaveLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnLeaveLobbyCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_UpdateLobbyModification(EOS_HLobby Handle, const EOS_Lobby_UpdateLobbyModificationOptions* Options, EOS_HLobbyModification* OutLobbyModificationHandle) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_UpdateLobby(EOS_HLobby Handle, const EOS_Lobby_UpdateLobbyOptions* Options, void* ClientData, const EOS_Lobby_OnUpdateLobbyCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_PromoteMember(EOS_HLobby Handle, const EOS_Lobby_PromoteMemberOptions* Options, void* ClientData, const EOS_Lobby_OnPromoteMemberCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_KickMember(EOS_HLobby Handle, const EOS_Lobby_KickMemberOptions* Options, void* ClientData, const EOS_Lobby_OnKickMemberCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_HardMuteMember(EOS_HLobby Handle, const EOS_Lobby_HardMuteMemberOptions* Options, void* ClientData, const EOS_Lobby_OnHardMuteMemberCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyUpdateReceived(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLobbyUpdateReceivedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyUpdateReceivedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyUpdateReceived(EOS_HLobby Handle, EOS_NotificationId InId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyMemberUpdateReceived(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLobbyMemberUpdateReceivedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyMemberUpdateReceivedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyMemberUpdateReceived(EOS_HLobby Handle, EOS_NotificationId InId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyMemberStatusReceived(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLobbyMemberStatusReceivedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyMemberStatusReceivedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyMemberStatusReceived(EOS_HLobby Handle, EOS_NotificationId InId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_SendInvite(EOS_HLobby Handle, const EOS_Lobby_SendInviteOptions* Options, void* ClientData, const EOS_Lobby_OnSendInviteCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RejectInvite(EOS_HLobby Handle, const EOS_Lobby_RejectInviteOptions* Options, void* ClientData, const EOS_Lobby_OnRejectInviteCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_QueryInvites(EOS_HLobby Handle, const EOS_Lobby_QueryInvitesOptions* Options, void* ClientData, const EOS_Lobby_OnQueryInvitesCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Lobby_GetInviteCount(EOS_HLobby Handle, const EOS_Lobby_GetInviteCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_GetInviteIdByIndex(EOS_HLobby Handle, const EOS_Lobby_GetInviteIdByIndexOptions* Options, char* OutBuffer, int32_t* InOutBufferLength) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_CreateLobbySearch(EOS_HLobby Handle, const EOS_Lobby_CreateLobbySearchOptions* Options, EOS_HLobbySearch* OutLobbySearchHandle) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyInviteReceived(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLobbyInviteReceivedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyInviteReceivedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyInviteReceived(EOS_HLobby Handle, EOS_NotificationId InId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyInviteAccepted(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLobbyInviteAcceptedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyInviteAcceptedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyInviteAccepted(EOS_HLobby Handle, EOS_NotificationId InId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyInviteRejected(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLobbyInviteRejectedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyInviteRejectedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyInviteRejected(EOS_HLobby Handle, EOS_NotificationId InId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyJoinLobbyAccepted(EOS_HLobby Handle, const EOS_Lobby_AddNotifyJoinLobbyAcceptedOptions* Options, void* ClientData, const EOS_Lobby_OnJoinLobbyAcceptedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyJoinLobbyAccepted(EOS_HLobby Handle, EOS_NotificationId InId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifySendLobbyNativeInviteRequested(EOS_HLobby Handle, const EOS_Lobby_AddNotifySendLobbyNativeInviteRequestedOptions* Options, void* ClientData, const EOS_Lobby_OnSendLobbyNativeInviteRequestedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifySendLobbyNativeInviteRequested(EOS_HLobby Handle, EOS_NotificationId InId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_CopyLobbyDetailsHandleByInviteId(EOS_HLobby Handle, const EOS_Lobby_CopyLobbyDetailsHandleByInviteIdOptions* Options, EOS_HLobbyDetails* OutLobbyDetailsHandle) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_CopyLobbyDetailsHandleByUiEventId(EOS_HLobby Handle, const EOS_Lobby_CopyLobbyDetailsHandleByUiEventIdOptions* Options, EOS_HLobbyDetails* OutLobbyDetailsHandle) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_CopyLobbyDetailsHandle(EOS_HLobby Handle, const EOS_Lobby_CopyLobbyDetailsHandleOptions* Options, EOS_HLobbyDetails* OutLobbyDetailsHandle) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_GetRTCRoomName(EOS_HLobby Handle, const EOS_Lobby_GetRTCRoomNameOptions* Options, char* OutBuffer, uint32_t* InOutBufferLength) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_JoinRTCRoom(EOS_HLobby Handle, const EOS_Lobby_JoinRTCRoomOptions* Options, void* ClientData, const EOS_Lobby_OnJoinRTCRoomCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_LeaveRTCRoom(EOS_HLobby Handle, const EOS_Lobby_LeaveRTCRoomOptions* Options, void* ClientData, const EOS_Lobby_OnLeaveRTCRoomCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_IsRTCRoomConnected(EOS_HLobby Handle, const EOS_Lobby_IsRTCRoomConnectedOptions* Options, EOS_Bool* bOutIsConnected) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyRTCRoomConnectionChanged(EOS_HLobby Handle, const EOS_Lobby_AddNotifyRTCRoomConnectionChangedOptions* Options, void* ClientData, const EOS_Lobby_OnRTCRoomConnectionChangedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyRTCRoomConnectionChanged(EOS_HLobby Handle, EOS_NotificationId InId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_GetConnectString(EOS_HLobby Handle, const EOS_Lobby_GetConnectStringOptions* Options, char* OutBuffer, uint32_t* InOutBufferLength) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_ParseConnectString(EOS_HLobby Handle, const EOS_Lobby_ParseConnectStringOptions* Options, char* OutBuffer, uint32_t* InOutBufferLength) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLeaveLobbyRequested(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLeaveLobbyRequestedOptions* Options, void* ClientData, const EOS_Lobby_OnLeaveLobbyRequestedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLeaveLobbyRequested(EOS_HLobby Handle, EOS_NotificationId InId) {
    (void)Handle;
}

// ============ LobbyDetails ============

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_LobbyDetails_GetLobbyOwner(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_GetLobbyOwnerOptions* Options) {
    (void)Handle;
    return NULL;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyInfo(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyInfoOptions* Options, EOS_LobbyDetails_Info ** OutLobbyDetailsInfo) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyMemberInfo(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyMemberInfoOptions* Options, EOS_LobbyDetails_MemberInfo ** OutLobbyDetailsMemberInfo) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(uint32_t) EOS_LobbyDetails_GetAttributeCount(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_GetAttributeCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyAttributeByIndex(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyAttributeByIndexOptions* Options, EOS_Lobby_Attribute ** OutAttribute) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyAttributeByKey(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyAttributeByKeyOptions* Options, EOS_Lobby_Attribute ** OutAttribute) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(uint32_t) EOS_LobbyDetails_GetMemberCount(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_GetMemberCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_LobbyDetails_GetMemberByIndex(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_GetMemberByIndexOptions* Options) {
    (void)Handle;
    return NULL;
}

EOS_DECLARE_FUNC(uint32_t) EOS_LobbyDetails_GetMemberAttributeCount(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_GetMemberAttributeCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyMemberAttributeByIndex(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyMemberAttributeByIndexOptions* Options, EOS_Lobby_Attribute ** OutAttribute) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyMemberAttributeByKey(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyMemberAttributeByKeyOptions* Options, EOS_Lobby_Attribute ** OutAttribute) {
    (void)Handle;
    return EOS_NotConfigured;
}

// ============ LobbyModification ============

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_SetBucketId(EOS_HLobbyModification Handle, const EOS_LobbyModification_SetBucketIdOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_SetPermissionLevel(EOS_HLobbyModification Handle, const EOS_LobbyModification_SetPermissionLevelOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_SetMaxMembers(EOS_HLobbyModification Handle, const EOS_LobbyModification_SetMaxMembersOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_SetInvitesAllowed(EOS_HLobbyModification Handle, const EOS_LobbyModification_SetInvitesAllowedOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_AddAttribute(EOS_HLobbyModification Handle, const EOS_LobbyModification_AddAttributeOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_RemoveAttribute(EOS_HLobbyModification Handle, const EOS_LobbyModification_RemoveAttributeOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_AddMemberAttribute(EOS_HLobbyModification Handle, const EOS_LobbyModification_AddMemberAttributeOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_RemoveMemberAttribute(EOS_HLobbyModification Handle, const EOS_LobbyModification_RemoveMemberAttributeOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_SetAllowedPlatformIds(EOS_HLobbyModification Handle, const EOS_LobbyModification_SetAllowedPlatformIdsOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

// ============ LobbySearch ============

EOS_DECLARE_FUNC(void) EOS_LobbySearch_Find(EOS_HLobbySearch Handle, const EOS_LobbySearch_FindOptions* Options, void* ClientData, const EOS_LobbySearch_OnFindCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetLobbyId(EOS_HLobbySearch Handle, const EOS_LobbySearch_SetLobbyIdOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetTargetUserId(EOS_HLobbySearch Handle, const EOS_LobbySearch_SetTargetUserIdOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetParameter(EOS_HLobbySearch Handle, const EOS_LobbySearch_SetParameterOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_RemoveParameter(EOS_HLobbySearch Handle, const EOS_LobbySearch_RemoveParameterOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetMaxResults(EOS_HLobbySearch Handle, const EOS_LobbySearch_SetMaxResultsOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(uint32_t) EOS_LobbySearch_GetSearchResultCount(EOS_HLobbySearch Handle, const EOS_LobbySearch_GetSearchResultCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_CopySearchResultByIndex(EOS_HLobbySearch Handle, const EOS_LobbySearch_CopySearchResultByIndexOptions* Options, EOS_HLobbyDetails* OutLobbyDetailsHandle) {
    (void)Handle;
    return EOS_NotConfigured;
}

// ============ Logging ============

EOS_DECLARE_FUNC(EOS_EResult) EOS_Logging_SetCallback(EOS_LogMessageFunc Callback) {
    (void)Callback;
    // Must return Success or Unreal thinks SDK isn't configured
    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Logging_SetLogLevel(EOS_ELogCategory LogCategory, EOS_ELogLevel LogLevel) {
    (void)LogCategory;
    (void)LogLevel;
    // Must return Success or Unreal thinks SDK isn't configured
    return EOS_Success;
}

// ============ Metrics ============

EOS_DECLARE_FUNC(EOS_EResult) EOS_Metrics_BeginPlayerSession(EOS_HMetrics Handle, const EOS_Metrics_BeginPlayerSessionOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Metrics_EndPlayerSession(EOS_HMetrics Handle, const EOS_Metrics_EndPlayerSessionOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

// ============ Mods ============

EOS_DECLARE_FUNC(void) EOS_Mods_InstallMod(EOS_HMods Handle, const EOS_Mods_InstallModOptions* Options, void* ClientData, const EOS_Mods_OnInstallModCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Mods_UninstallMod(EOS_HMods Handle, const EOS_Mods_UninstallModOptions* Options, void* ClientData, const EOS_Mods_OnUninstallModCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Mods_EnumerateMods(EOS_HMods Handle, const EOS_Mods_EnumerateModsOptions* Options, void* ClientData, const EOS_Mods_OnEnumerateModsCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Mods_CopyModInfo(EOS_HMods Handle, const EOS_Mods_CopyModInfoOptions* Options, EOS_Mods_ModInfo ** OutEnumeratedMods) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_Mods_UpdateMod(EOS_HMods Handle, const EOS_Mods_UpdateModOptions* Options, void* ClientData, const EOS_Mods_OnUpdateModCallback CompletionDelegate) {
    (void)Handle;
}

// ============ P2P ============

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_SendPacket(EOS_HP2P Handle, const EOS_P2P_SendPacketOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_GetNextReceivedPacketSize(EOS_HP2P Handle, const EOS_P2P_GetNextReceivedPacketSizeOptions* Options, uint32_t* OutPacketSizeBytes) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_ReceivePacket(EOS_HP2P Handle, const EOS_P2P_ReceivePacketOptions* Options, EOS_ProductUserId* OutPeerId, EOS_P2P_SocketId* OutSocketId, uint8_t* OutChannel, void* OutData, uint32_t* OutBytesWritten) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_P2P_AddNotifyPeerConnectionRequest(EOS_HP2P Handle, const EOS_P2P_AddNotifyPeerConnectionRequestOptions* Options, void* ClientData, EOS_P2P_OnIncomingConnectionRequestCallback ConnectionRequestHandler) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_P2P_RemoveNotifyPeerConnectionRequest(EOS_HP2P Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_P2P_AddNotifyPeerConnectionEstablished(EOS_HP2P Handle, const EOS_P2P_AddNotifyPeerConnectionEstablishedOptions* Options, void* ClientData, EOS_P2P_OnPeerConnectionEstablishedCallback ConnectionEstablishedHandler) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_P2P_RemoveNotifyPeerConnectionEstablished(EOS_HP2P Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_P2P_AddNotifyPeerConnectionInterrupted(EOS_HP2P Handle, const EOS_P2P_AddNotifyPeerConnectionInterruptedOptions* Options, void* ClientData, EOS_P2P_OnPeerConnectionInterruptedCallback ConnectionInterruptedHandler) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_P2P_RemoveNotifyPeerConnectionInterrupted(EOS_HP2P Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_P2P_AddNotifyPeerConnectionClosed(EOS_HP2P Handle, const EOS_P2P_AddNotifyPeerConnectionClosedOptions* Options, void* ClientData, EOS_P2P_OnRemoteConnectionClosedCallback ConnectionClosedHandler) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_P2P_RemoveNotifyPeerConnectionClosed(EOS_HP2P Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_AcceptConnection(EOS_HP2P Handle, const EOS_P2P_AcceptConnectionOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_CloseConnection(EOS_HP2P Handle, const EOS_P2P_CloseConnectionOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_CloseConnections(EOS_HP2P Handle, const EOS_P2P_CloseConnectionsOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_P2P_QueryNATType(EOS_HP2P Handle, const EOS_P2P_QueryNATTypeOptions* Options, void* ClientData, const EOS_P2P_OnQueryNATTypeCompleteCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_GetNATType(EOS_HP2P Handle, const EOS_P2P_GetNATTypeOptions* Options, EOS_ENATType* OutNATType) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_SetRelayControl(EOS_HP2P Handle, const EOS_P2P_SetRelayControlOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_GetRelayControl(EOS_HP2P Handle, const EOS_P2P_GetRelayControlOptions* Options, EOS_ERelayControl* OutRelayControl) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_SetPortRange(EOS_HP2P Handle, const EOS_P2P_SetPortRangeOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_GetPortRange(EOS_HP2P Handle, const EOS_P2P_GetPortRangeOptions* Options, uint16_t* OutPort, uint16_t* OutNumAdditionalPortsToTry) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_SetPacketQueueSize(EOS_HP2P Handle, const EOS_P2P_SetPacketQueueSizeOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_GetPacketQueueInfo(EOS_HP2P Handle, const EOS_P2P_GetPacketQueueInfoOptions* Options, EOS_P2P_PacketQueueInfo* OutPacketQueueInfo) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_P2P_AddNotifyIncomingPacketQueueFull(EOS_HP2P Handle, const EOS_P2P_AddNotifyIncomingPacketQueueFullOptions* Options, void* ClientData, EOS_P2P_OnIncomingPacketQueueFullCallback IncomingPacketQueueFullHandler) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_P2P_RemoveNotifyIncomingPacketQueueFull(EOS_HP2P Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_ClearPacketQueue(EOS_HP2P Handle, const EOS_P2P_ClearPacketQueueOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

// ============ PlayerDataStorage ============

EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_QueryFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_QueryFileOptions* QueryFileOptions, void* ClientData, const EOS_PlayerDataStorage_OnQueryFileCompleteCallback CompletionCallback) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_QueryFileList(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_QueryFileListOptions* QueryFileListOptions, void* ClientData, const EOS_PlayerDataStorage_OnQueryFileListCompleteCallback CompletionCallback) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorage_CopyFileMetadataByFilename(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_CopyFileMetadataByFilenameOptions* CopyFileMetadataOptions, EOS_PlayerDataStorage_FileMetadata ** OutMetadata) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorage_GetFileMetadataCount(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_GetFileMetadataCountOptions* GetFileMetadataCountOptions, int32_t* OutFileMetadataCount) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorage_CopyFileMetadataAtIndex(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_CopyFileMetadataAtIndexOptions* CopyFileMetadataOptions, EOS_PlayerDataStorage_FileMetadata ** OutMetadata) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_DuplicateFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_DuplicateFileOptions* DuplicateOptions, void* ClientData, const EOS_PlayerDataStorage_OnDuplicateFileCompleteCallback CompletionCallback) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_DeleteFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_DeleteFileOptions* DeleteOptions, void* ClientData, const EOS_PlayerDataStorage_OnDeleteFileCompleteCallback CompletionCallback) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_HPlayerDataStorageFileTransferRequest) EOS_PlayerDataStorage_ReadFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_ReadFileOptions* ReadOptions, void* ClientData, const EOS_PlayerDataStorage_OnReadFileCompleteCallback CompletionCallback) {
    (void)Handle;
    return NULL;
}

EOS_DECLARE_FUNC(EOS_HPlayerDataStorageFileTransferRequest) EOS_PlayerDataStorage_WriteFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_WriteFileOptions* WriteOptions, void* ClientData, const EOS_PlayerDataStorage_OnWriteFileCompleteCallback CompletionCallback) {
    (void)Handle;
    return NULL;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorage_DeleteCache(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_DeleteCacheOptions* Options, void* ClientData, const EOS_PlayerDataStorage_OnDeleteCacheCompleteCallback CompletionCallback) {
    (void)Handle;
    return EOS_NotConfigured;
}

// ============ PlayerDataStorageFileTransferRequest ============

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorageFileTransferRequest_GetFileRequestState(EOS_HPlayerDataStorageFileTransferRequest Handle) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorageFileTransferRequest_GetFilename(EOS_HPlayerDataStorageFileTransferRequest Handle, uint32_t FilenameStringBufferSizeBytes, char* OutStringBuffer, int32_t* OutStringLength) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorageFileTransferRequest_CancelRequest(EOS_HPlayerDataStorageFileTransferRequest Handle) {
    (void)Handle;
    return EOS_NotConfigured;
}

// ============ Presence ============

EOS_DECLARE_FUNC(void) EOS_Presence_QueryPresence(EOS_HPresence Handle, const EOS_Presence_QueryPresenceOptions* Options, void* ClientData, const EOS_Presence_OnQueryPresenceCompleteCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_Presence_HasPresence(EOS_HPresence Handle, const EOS_Presence_HasPresenceOptions* Options) {
    (void)Handle;
    return EOS_FALSE;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Presence_CopyPresence(EOS_HPresence Handle, const EOS_Presence_CopyPresenceOptions* Options, EOS_Presence_Info ** OutPresence) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Presence_CreatePresenceModification(EOS_HPresence Handle, const EOS_Presence_CreatePresenceModificationOptions* Options, EOS_HPresenceModification* OutPresenceModificationHandle) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_Presence_SetPresence(EOS_HPresence Handle, const EOS_Presence_SetPresenceOptions* Options, void* ClientData, const EOS_Presence_SetPresenceCompleteCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Presence_AddNotifyOnPresenceChanged(EOS_HPresence Handle, const EOS_Presence_AddNotifyOnPresenceChangedOptions* Options, void* ClientData, const EOS_Presence_OnPresenceChangedCallback NotificationHandler) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Presence_RemoveNotifyOnPresenceChanged(EOS_HPresence Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Presence_AddNotifyJoinGameAccepted(EOS_HPresence Handle, const EOS_Presence_AddNotifyJoinGameAcceptedOptions* Options, void* ClientData, const EOS_Presence_OnJoinGameAcceptedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Presence_RemoveNotifyJoinGameAccepted(EOS_HPresence Handle, EOS_NotificationId InId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Presence_GetJoinInfo(EOS_HPresence Handle, const EOS_Presence_GetJoinInfoOptions* Options, char* OutBuffer, int32_t* InOutBufferLength) {
    (void)Handle;
    return EOS_NotConfigured;
}

// ============ PresenceModification ============

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetStatus(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetStatusOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetTemplateId(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetTemplateIdOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetTemplateData(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetTemplateDataOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetRawRichText(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetRawRichTextOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetData(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetDataOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_DeleteData(EOS_HPresenceModification Handle, const EOS_PresenceModification_DeleteDataOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetJoinInfo(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetJoinInfoOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

// ============ ProgressionSnapshot ============

EOS_DECLARE_FUNC(EOS_EResult) EOS_ProgressionSnapshot_BeginSnapshot(EOS_HProgressionSnapshot Handle, const EOS_ProgressionSnapshot_BeginSnapshotOptions* Options, uint32_t* OutSnapshotId) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_ProgressionSnapshot_AddProgression(EOS_HProgressionSnapshot Handle, const EOS_ProgressionSnapshot_AddProgressionOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_ProgressionSnapshot_SubmitSnapshot(EOS_HProgressionSnapshot Handle, const EOS_ProgressionSnapshot_SubmitSnapshotOptions* Options, void* ClientData, const EOS_ProgressionSnapshot_OnSubmitSnapshotCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_ProgressionSnapshot_EndSnapshot(EOS_HProgressionSnapshot Handle, const EOS_ProgressionSnapshot_EndSnapshotOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_ProgressionSnapshot_DeleteSnapshot(EOS_HProgressionSnapshot Handle, const EOS_ProgressionSnapshot_DeleteSnapshotOptions* Options, void* ClientData, const EOS_ProgressionSnapshot_OnDeleteSnapshotCallback CompletionDelegate) {
    (void)Handle;
}

// ============ RTC ============

EOS_DECLARE_FUNC(EOS_HRTCAudio) EOS_RTC_GetAudioInterface(EOS_HRTC Handle) {
    (void)Handle;
    return NULL;
}

EOS_DECLARE_FUNC(EOS_HRTCData) EOS_RTC_GetDataInterface(EOS_HRTC Handle) {
    (void)Handle;
    return NULL;
}

EOS_DECLARE_FUNC(void) EOS_RTC_JoinRoom(EOS_HRTC Handle, const EOS_RTC_JoinRoomOptions* Options, void* ClientData, const EOS_RTC_OnJoinRoomCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_RTC_LeaveRoom(EOS_HRTC Handle, const EOS_RTC_LeaveRoomOptions* Options, void* ClientData, const EOS_RTC_OnLeaveRoomCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_RTC_BlockParticipant(EOS_HRTC Handle, const EOS_RTC_BlockParticipantOptions* Options, void* ClientData, const EOS_RTC_OnBlockParticipantCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTC_AddNotifyDisconnected(EOS_HRTC Handle, const EOS_RTC_AddNotifyDisconnectedOptions* Options, void* ClientData, const EOS_RTC_OnDisconnectedCallback CompletionDelegate) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_RTC_RemoveNotifyDisconnected(EOS_HRTC Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTC_AddNotifyParticipantStatusChanged(EOS_HRTC Handle, const EOS_RTC_AddNotifyParticipantStatusChangedOptions* Options, void* ClientData, const EOS_RTC_OnParticipantStatusChangedCallback CompletionDelegate) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_RTC_RemoveNotifyParticipantStatusChanged(EOS_HRTC Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTC_SetSetting(EOS_HRTC Handle, const EOS_RTC_SetSettingOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTC_SetRoomSetting(EOS_HRTC Handle, const EOS_RTC_SetRoomSettingOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTC_AddNotifyRoomStatisticsUpdated(EOS_HRTC Handle, const EOS_RTC_AddNotifyRoomStatisticsUpdatedOptions* Options, void* ClientData, const EOS_RTC_OnRoomStatisticsUpdatedCallback StatisticsUpdateHandler) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_RTC_RemoveNotifyRoomStatisticsUpdated(EOS_HRTC Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

// ============ RTCAdmin ============

EOS_DECLARE_FUNC(void) EOS_RTCAdmin_QueryJoinRoomToken(EOS_HRTCAdmin Handle, const EOS_RTCAdmin_QueryJoinRoomTokenOptions* Options, void* ClientData, const EOS_RTCAdmin_OnQueryJoinRoomTokenCompleteCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAdmin_CopyUserTokenByIndex(EOS_HRTCAdmin Handle, const EOS_RTCAdmin_CopyUserTokenByIndexOptions* Options, EOS_RTCAdmin_UserToken ** OutUserToken) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAdmin_CopyUserTokenByUserId(EOS_HRTCAdmin Handle, const EOS_RTCAdmin_CopyUserTokenByUserIdOptions* Options, EOS_RTCAdmin_UserToken ** OutUserToken) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_RTCAdmin_Kick(EOS_HRTCAdmin Handle, const EOS_RTCAdmin_KickOptions* Options, void* ClientData, const EOS_RTCAdmin_OnKickCompleteCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_RTCAdmin_SetParticipantHardMute(EOS_HRTCAdmin Handle, const EOS_RTCAdmin_SetParticipantHardMuteOptions* Options, void* ClientData, const EOS_RTCAdmin_OnSetParticipantHardMuteCompleteCallback CompletionDelegate) {
    (void)Handle;
}

// ============ RTCAudio ============

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_SendAudio(EOS_HRTCAudio Handle, const EOS_RTCAudio_SendAudioOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_UpdateSending(EOS_HRTCAudio Handle, const EOS_RTCAudio_UpdateSendingOptions* Options, void* ClientData, const EOS_RTCAudio_OnUpdateSendingCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_UpdateReceiving(EOS_HRTCAudio Handle, const EOS_RTCAudio_UpdateReceivingOptions* Options, void* ClientData, const EOS_RTCAudio_OnUpdateReceivingCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_UpdateSendingVolume(EOS_HRTCAudio Handle, const EOS_RTCAudio_UpdateSendingVolumeOptions* Options, void* ClientData, const EOS_RTCAudio_OnUpdateSendingVolumeCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_UpdateReceivingVolume(EOS_HRTCAudio Handle, const EOS_RTCAudio_UpdateReceivingVolumeOptions* Options, void* ClientData, const EOS_RTCAudio_OnUpdateReceivingVolumeCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_UpdateParticipantVolume(EOS_HRTCAudio Handle, const EOS_RTCAudio_UpdateParticipantVolumeOptions* Options, void* ClientData, const EOS_RTCAudio_OnUpdateParticipantVolumeCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTCAudio_AddNotifyParticipantUpdated(EOS_HRTCAudio Handle, const EOS_RTCAudio_AddNotifyParticipantUpdatedOptions* Options, void* ClientData, const EOS_RTCAudio_OnParticipantUpdatedCallback CompletionDelegate) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_RemoveNotifyParticipantUpdated(EOS_HRTCAudio Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTCAudio_AddNotifyAudioDevicesChanged(EOS_HRTCAudio Handle, const EOS_RTCAudio_AddNotifyAudioDevicesChangedOptions* Options, void* ClientData, const EOS_RTCAudio_OnAudioDevicesChangedCallback CompletionDelegate) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_RemoveNotifyAudioDevicesChanged(EOS_HRTCAudio Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTCAudio_AddNotifyAudioInputState(EOS_HRTCAudio Handle, const EOS_RTCAudio_AddNotifyAudioInputStateOptions* Options, void* ClientData, const EOS_RTCAudio_OnAudioInputStateCallback CompletionDelegate) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_RemoveNotifyAudioInputState(EOS_HRTCAudio Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTCAudio_AddNotifyAudioOutputState(EOS_HRTCAudio Handle, const EOS_RTCAudio_AddNotifyAudioOutputStateOptions* Options, void* ClientData, const EOS_RTCAudio_OnAudioOutputStateCallback CompletionDelegate) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_RemoveNotifyAudioOutputState(EOS_HRTCAudio Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTCAudio_AddNotifyAudioBeforeSend(EOS_HRTCAudio Handle, const EOS_RTCAudio_AddNotifyAudioBeforeSendOptions* Options, void* ClientData, const EOS_RTCAudio_OnAudioBeforeSendCallback CompletionDelegate) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_RemoveNotifyAudioBeforeSend(EOS_HRTCAudio Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTCAudio_AddNotifyAudioBeforeRender(EOS_HRTCAudio Handle, const EOS_RTCAudio_AddNotifyAudioBeforeRenderOptions* Options, void* ClientData, const EOS_RTCAudio_OnAudioBeforeRenderCallback CompletionDelegate) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_RemoveNotifyAudioBeforeRender(EOS_HRTCAudio Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_RegisterPlatformUser(EOS_HRTCAudio Handle, const EOS_RTCAudio_RegisterPlatformUserOptions* Options, void* ClientData, const EOS_RTCAudio_OnRegisterPlatformUserCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_UnregisterPlatformUser(EOS_HRTCAudio Handle, const EOS_RTCAudio_UnregisterPlatformUserOptions* Options, void* ClientData, const EOS_RTCAudio_OnUnregisterPlatformUserCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_QueryInputDevicesInformation(EOS_HRTCAudio Handle, const EOS_RTCAudio_QueryInputDevicesInformationOptions* Options, void* ClientData, const EOS_RTCAudio_OnQueryInputDevicesInformationCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(uint32_t) EOS_RTCAudio_GetInputDevicesCount(EOS_HRTCAudio Handle, const EOS_RTCAudio_GetInputDevicesCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_CopyInputDeviceInformationByIndex(EOS_HRTCAudio Handle, const EOS_RTCAudio_CopyInputDeviceInformationByIndexOptions* Options, EOS_RTCAudio_InputDeviceInformation ** OutInputDeviceInformation) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_QueryOutputDevicesInformation(EOS_HRTCAudio Handle, const EOS_RTCAudio_QueryOutputDevicesInformationOptions* Options, void* ClientData, const EOS_RTCAudio_OnQueryOutputDevicesInformationCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(uint32_t) EOS_RTCAudio_GetOutputDevicesCount(EOS_HRTCAudio Handle, const EOS_RTCAudio_GetOutputDevicesCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_CopyOutputDeviceInformationByIndex(EOS_HRTCAudio Handle, const EOS_RTCAudio_CopyOutputDeviceInformationByIndexOptions* Options, EOS_RTCAudio_OutputDeviceInformation ** OutOutputDeviceInformation) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_SetInputDeviceSettings(EOS_HRTCAudio Handle, const EOS_RTCAudio_SetInputDeviceSettingsOptions* Options, void* ClientData, const EOS_RTCAudio_OnSetInputDeviceSettingsCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_SetOutputDeviceSettings(EOS_HRTCAudio Handle, const EOS_RTCAudio_SetOutputDeviceSettingsOptions* Options, void* ClientData, const EOS_RTCAudio_OnSetOutputDeviceSettingsCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_RegisterPlatformAudioUser(EOS_HRTCAudio Handle, const EOS_RTCAudio_RegisterPlatformAudioUserOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_UnregisterPlatformAudioUser(EOS_HRTCAudio Handle, const EOS_RTCAudio_UnregisterPlatformAudioUserOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(uint32_t) EOS_RTCAudio_GetAudioInputDevicesCount(EOS_HRTCAudio Handle, const EOS_RTCAudio_GetAudioInputDevicesCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(const EOS_RTCAudio_AudioInputDeviceInfo *) EOS_RTCAudio_GetAudioInputDeviceByIndex(EOS_HRTCAudio Handle, const EOS_RTCAudio_GetAudioInputDeviceByIndexOptions* Options) {
    (void)Handle;
    return NULL;
}

EOS_DECLARE_FUNC(uint32_t) EOS_RTCAudio_GetAudioOutputDevicesCount(EOS_HRTCAudio Handle, const EOS_RTCAudio_GetAudioOutputDevicesCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(const EOS_RTCAudio_AudioOutputDeviceInfo *) EOS_RTCAudio_GetAudioOutputDeviceByIndex(EOS_HRTCAudio Handle, const EOS_RTCAudio_GetAudioOutputDeviceByIndexOptions* Options) {
    (void)Handle;
    return NULL;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_SetAudioInputSettings(EOS_HRTCAudio Handle, const EOS_RTCAudio_SetAudioInputSettingsOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_SetAudioOutputSettings(EOS_HRTCAudio Handle, const EOS_RTCAudio_SetAudioOutputSettingsOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

// ============ RTCData ============

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTCData_AddNotifyDataReceived(EOS_HRTCData Handle, const EOS_RTCData_AddNotifyDataReceivedOptions* Options, void* ClientData, const EOS_RTCData_OnDataReceivedCallback CompletionDelegate) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_RTCData_RemoveNotifyDataReceived(EOS_HRTCData Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCData_SendData(EOS_HRTCData Handle, const EOS_RTCData_SendDataOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_RTCData_UpdateSending(EOS_HRTCData Handle, const EOS_RTCData_UpdateSendingOptions* Options, void* ClientData, const EOS_RTCData_OnUpdateSendingCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_RTCData_UpdateReceiving(EOS_HRTCData Handle, const EOS_RTCData_UpdateReceivingOptions* Options, void* ClientData, const EOS_RTCData_OnUpdateReceivingCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTCData_AddNotifyParticipantUpdated(EOS_HRTCData Handle, const EOS_RTCData_AddNotifyParticipantUpdatedOptions* Options, void* ClientData, const EOS_RTCData_OnParticipantUpdatedCallback CompletionDelegate) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_RTCData_RemoveNotifyParticipantUpdated(EOS_HRTCData Handle, EOS_NotificationId NotificationId) {
    (void)Handle;
}

// ============ Reports ============

EOS_DECLARE_FUNC(void) EOS_Reports_SendPlayerBehaviorReport(EOS_HReports Handle, const EOS_Reports_SendPlayerBehaviorReportOptions* Options, void* ClientData, const EOS_Reports_OnSendPlayerBehaviorReportCompleteCallback CompletionDelegate) {
    (void)Handle;
}

// ============ Sanctions ============

EOS_DECLARE_FUNC(void) EOS_Sanctions_QueryActivePlayerSanctions(EOS_HSanctions Handle, const EOS_Sanctions_QueryActivePlayerSanctionsOptions* Options, void* ClientData, const EOS_Sanctions_OnQueryActivePlayerSanctionsCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Sanctions_GetPlayerSanctionCount(EOS_HSanctions Handle, const EOS_Sanctions_GetPlayerSanctionCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sanctions_CopyPlayerSanctionByIndex(EOS_HSanctions Handle, const EOS_Sanctions_CopyPlayerSanctionByIndexOptions* Options, EOS_Sanctions_PlayerSanction ** OutSanction) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_Sanctions_CreatePlayerSanctionAppeal(EOS_HSanctions Handle, const EOS_Sanctions_CreatePlayerSanctionAppealOptions* Options, void* ClientData, const EOS_Sanctions_CreatePlayerSanctionAppealCallback CompletionDelegate) {
    (void)Handle;
}

// ============ SessionModification ============

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetAllowedPlatformIds(EOS_HSessionModification Handle, const EOS_SessionModification_SetAllowedPlatformIdsOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

// ============ Sessions ============

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifySessionInviteRejected(EOS_HSessions Handle, const EOS_Sessions_AddNotifySessionInviteRejectedOptions* Options, void* ClientData, const EOS_Sessions_OnSessionInviteRejectedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifySessionInviteRejected(EOS_HSessions Handle, EOS_NotificationId InId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CopySessionHandleForPresence(EOS_HSessions Handle, const EOS_Sessions_CopySessionHandleForPresenceOptions* Options, EOS_HSessionDetails* OutSessionHandle) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_IsUserInSession(EOS_HSessions Handle, const EOS_Sessions_IsUserInSessionOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_DumpSessionState(EOS_HSessions Handle, const EOS_Sessions_DumpSessionStateOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifyLeaveSessionRequested(EOS_HSessions Handle, const EOS_Sessions_AddNotifyLeaveSessionRequestedOptions* Options, void* ClientData, const EOS_Sessions_OnLeaveSessionRequestedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifyLeaveSessionRequested(EOS_HSessions Handle, EOS_NotificationId InId) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifySendSessionNativeInviteRequested(EOS_HSessions Handle, const EOS_Sessions_AddNotifySendSessionNativeInviteRequestedOptions* Options, void* ClientData, const EOS_Sessions_OnSendSessionNativeInviteRequestedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifySendSessionNativeInviteRequested(EOS_HSessions Handle, EOS_NotificationId InId) {
    (void)Handle;
}

// ============ Stats ============

EOS_DECLARE_FUNC(void) EOS_Stats_IngestStat(EOS_HStats Handle, const EOS_Stats_IngestStatOptions* Options, void* ClientData, const EOS_Stats_OnIngestStatCompleteCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_Stats_QueryStats(EOS_HStats Handle, const EOS_Stats_QueryStatsOptions* Options, void* ClientData, const EOS_Stats_OnQueryStatsCompleteCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Stats_GetStatsCount(EOS_HStats Handle, const EOS_Stats_GetStatCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Stats_CopyStatByIndex(EOS_HStats Handle, const EOS_Stats_CopyStatByIndexOptions* Options, EOS_Stats_Stat ** OutStat) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Stats_CopyStatByName(EOS_HStats Handle, const EOS_Stats_CopyStatByNameOptions* Options, EOS_Stats_Stat ** OutStat) {
    (void)Handle;
    return EOS_NotConfigured;
}

// ============ TitleStorage ============

EOS_DECLARE_FUNC(void) EOS_TitleStorage_QueryFile(EOS_HTitleStorage Handle, const EOS_TitleStorage_QueryFileOptions* Options, void* ClientData, const EOS_TitleStorage_OnQueryFileCompleteCallback CompletionCallback) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_TitleStorage_QueryFileList(EOS_HTitleStorage Handle, const EOS_TitleStorage_QueryFileListOptions* Options, void* ClientData, const EOS_TitleStorage_OnQueryFileListCompleteCallback CompletionCallback) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorage_CopyFileMetadataByFilename(EOS_HTitleStorage Handle, const EOS_TitleStorage_CopyFileMetadataByFilenameOptions* Options, EOS_TitleStorage_FileMetadata ** OutMetadata) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(uint32_t) EOS_TitleStorage_GetFileMetadataCount(EOS_HTitleStorage Handle, const EOS_TitleStorage_GetFileMetadataCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorage_CopyFileMetadataAtIndex(EOS_HTitleStorage Handle, const EOS_TitleStorage_CopyFileMetadataAtIndexOptions* Options, EOS_TitleStorage_FileMetadata ** OutMetadata) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_HTitleStorageFileTransferRequest) EOS_TitleStorage_ReadFile(EOS_HTitleStorage Handle, const EOS_TitleStorage_ReadFileOptions* Options, void* ClientData, const EOS_TitleStorage_OnReadFileCompleteCallback CompletionCallback) {
    (void)Handle;
    return NULL;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorage_DeleteCache(EOS_HTitleStorage Handle, const EOS_TitleStorage_DeleteCacheOptions* Options, void* ClientData, const EOS_TitleStorage_OnDeleteCacheCompleteCallback CompletionCallback) {
    (void)Handle;
    return EOS_NotConfigured;
}

// ============ TitleStorageFileTransferRequest ============

EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorageFileTransferRequest_GetFileRequestState(EOS_HTitleStorageFileTransferRequest Handle) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorageFileTransferRequest_GetFilename(EOS_HTitleStorageFileTransferRequest Handle, uint32_t FilenameStringBufferSizeBytes, char* OutStringBuffer, int32_t* OutStringLength) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorageFileTransferRequest_CancelRequest(EOS_HTitleStorageFileTransferRequest Handle) {
    (void)Handle;
    return EOS_NotConfigured;
}

// ============ UI ============

EOS_DECLARE_FUNC(void) EOS_UI_ShowFriends(EOS_HUI Handle, const EOS_UI_ShowFriendsOptions* Options, void* ClientData, const EOS_UI_OnShowFriendsCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_UI_HideFriends(EOS_HUI Handle, const EOS_UI_HideFriendsOptions* Options, void* ClientData, const EOS_UI_OnHideFriendsCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_GetFriendsVisible(EOS_HUI Handle, const EOS_UI_GetFriendsVisibleOptions* Options) {
    (void)Handle;
    return EOS_FALSE;
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_GetFriendsExclusiveInput(EOS_HUI Handle, const EOS_UI_GetFriendsExclusiveInputOptions* Options) {
    (void)Handle;
    return EOS_FALSE;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_UI_AddNotifyDisplaySettingsUpdated(EOS_HUI Handle, const EOS_UI_AddNotifyDisplaySettingsUpdatedOptions* Options, void* ClientData, const EOS_UI_OnDisplaySettingsUpdatedCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_UI_RemoveNotifyDisplaySettingsUpdated(EOS_HUI Handle, EOS_NotificationId Id) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_SetToggleFriendsKey(EOS_HUI Handle, const EOS_UI_SetToggleFriendsKeyOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_UI_EKeyCombination) EOS_UI_GetToggleFriendsKey(EOS_HUI Handle, const EOS_UI_GetToggleFriendsKeyOptions* Options) {
    (void)Handle;
    return NULL;
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_IsValidKeyCombination(EOS_HUI Handle, EOS_UI_EKeyCombination KeyCombination) {
    (void)Handle;
    return EOS_FALSE;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_SetToggleFriendsButton(EOS_HUI Handle, const EOS_UI_SetToggleFriendsButtonOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_UI_EInputStateButtonFlags) EOS_UI_GetToggleFriendsButton(EOS_HUI Handle, const EOS_UI_GetToggleFriendsButtonOptions* Options) {
    (void)Handle;
    return NULL;
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_IsValidButtonCombination(EOS_HUI Handle, EOS_UI_EInputStateButtonFlags ButtonCombination) {
    (void)Handle;
    return EOS_FALSE;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_SetDisplayPreference(EOS_HUI Handle, const EOS_UI_SetDisplayPreferenceOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_UI_ENotificationLocation) EOS_UI_GetNotificationLocationPreference(EOS_HUI Handle) {
    (void)Handle;
    return NULL;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_AcknowledgeEventId(EOS_HUI Handle, const EOS_UI_AcknowledgeEventIdOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_ReportInputState(EOS_HUI Handle, const EOS_UI_ReportInputStateOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_PrePresent(EOS_HUI Handle, const EOS_UI_PrePresentOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_UI_ShowBlockPlayer(EOS_HUI Handle, const EOS_UI_ShowBlockPlayerOptions* Options, void* ClientData, const EOS_UI_OnShowBlockPlayerCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_UI_ShowReportPlayer(EOS_HUI Handle, const EOS_UI_ShowReportPlayerOptions* Options, void* ClientData, const EOS_UI_OnShowReportPlayerCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_PauseSocialOverlay(EOS_HUI Handle, const EOS_UI_PauseSocialOverlayOptions* Options) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_IsSocialOverlayPaused(EOS_HUI Handle, const EOS_UI_IsSocialOverlayPausedOptions* Options) {
    (void)Handle;
    return EOS_FALSE;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_UI_AddNotifyMemoryMonitor(EOS_HUI Handle, const EOS_UI_AddNotifyMemoryMonitorOptions* Options, void* ClientData, const EOS_UI_OnMemoryMonitorCallback NotificationFn) {
    (void)Handle;
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_UI_RemoveNotifyMemoryMonitor(EOS_HUI Handle, EOS_NotificationId Id) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_UI_ShowNativeProfile(EOS_HUI Handle, const EOS_UI_ShowNativeProfileOptions* Options, void* ClientData, const EOS_UI_OnShowNativeProfileCallback CompletionDelegate) {
    (void)Handle;
}

// ============ UserInfo ============

EOS_DECLARE_FUNC(void) EOS_UserInfo_QueryUserInfo(EOS_HUserInfo Handle, const EOS_UserInfo_QueryUserInfoOptions* Options, void* ClientData, const EOS_UserInfo_OnQueryUserInfoCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_UserInfo_QueryUserInfoByDisplayName(EOS_HUserInfo Handle, const EOS_UserInfo_QueryUserInfoByDisplayNameOptions* Options, void* ClientData, const EOS_UserInfo_OnQueryUserInfoByDisplayNameCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(void) EOS_UserInfo_QueryUserInfoByExternalAccount(EOS_HUserInfo Handle, const EOS_UserInfo_QueryUserInfoByExternalAccountOptions* Options, void* ClientData, const EOS_UserInfo_OnQueryUserInfoByExternalAccountCallback CompletionDelegate) {
    (void)Handle;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UserInfo_CopyUserInfo(EOS_HUserInfo Handle, const EOS_UserInfo_CopyUserInfoOptions* Options, EOS_UserInfo ** OutUserInfo) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(uint32_t) EOS_UserInfo_GetExternalUserInfoCount(EOS_HUserInfo Handle, const EOS_UserInfo_GetExternalUserInfoCountOptions* Options) {
    (void)Handle;
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UserInfo_CopyExternalUserInfoByIndex(EOS_HUserInfo Handle, const EOS_UserInfo_CopyExternalUserInfoByIndexOptions* Options, EOS_UserInfo_ExternalUserInfo ** OutExternalUserInfo) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UserInfo_CopyExternalUserInfoByAccountType(EOS_HUserInfo Handle, const EOS_UserInfo_CopyExternalUserInfoByAccountTypeOptions* Options, EOS_UserInfo_ExternalUserInfo ** OutExternalUserInfo) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UserInfo_CopyExternalUserInfoByAccountId(EOS_HUserInfo Handle, const EOS_UserInfo_CopyExternalUserInfoByAccountIdOptions* Options, EOS_UserInfo_ExternalUserInfo ** OutExternalUserInfo) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UserInfo_CopyBestDisplayName(EOS_HUserInfo Handle, const EOS_UserInfo_CopyBestDisplayNameOptions* Options, EOS_UserInfo_BestDisplayName ** OutBestDisplayName) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UserInfo_CopyBestDisplayNameWithPlatform(EOS_HUserInfo Handle, const EOS_UserInfo_CopyBestDisplayNameWithPlatformOptions* Options, EOS_UserInfo_BestDisplayName ** OutBestDisplayName) {
    (void)Handle;
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_OnlinePlatformType) EOS_UserInfo_GetLocalPlatformType(EOS_HUserInfo Handle, const EOS_UserInfo_GetLocalPlatformTypeOptions* Options) {
    (void)Handle;
    return NULL;
}

