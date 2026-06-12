/**
 * EOS-LAN Emulator - Auto-generated Stub Functions
 * Regenerated from the CURRENT EOS SDK headers by /tmp/gen_stubs.py.
 * One exported stub per EOS_DECLARE_FUNC not implemented in a real source,
 * so managed P/Invoke callers never hit EntryPointNotFoundException.
 * EOS_EResult stubs return EOS_NotConfigured; everything else returns 0/NULL.
 */
#include "eos/eos_sdk.h"
#include "eos/eos_logging.h"
#include "eos/eos_stats.h"
#include "eos/eos_leaderboards.h"
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
#include "eos/eos_rtc_data.h"
#include "eos/eos_rtc_admin.h"
#include "eos/eos_progressionsnapshot.h"
#include "eos/eos_custominvites.h"
#include "eos/eos_integratedplatform.h"
#include "eos/eos_metrics.h"

EOS_DECLARE_FUNC(void) EOS_Achievements_DefinitionV2_Release(EOS_Achievements_DefinitionV2* AchievementDefinition) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_AntiCheatServer_AddNotifyClientActionRequired(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_AddNotifyClientActionRequiredOptions* Options, void* ClientData, EOS_AntiCheatServer_OnClientActionRequiredCallback NotificationFn) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_AntiCheatServer_AddNotifyClientAuthStatusChanged(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_AddNotifyClientAuthStatusChangedOptions* Options, void* ClientData, EOS_AntiCheatServer_OnClientAuthStatusChangedCallback NotificationFn) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_AntiCheatServer_AddNotifyMessageToClient(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_AddNotifyMessageToClientOptions* Options, void* ClientData, EOS_AntiCheatServer_OnMessageToClientCallback NotificationFn) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_BeginSession(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_BeginSessionOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_EndSession(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_EndSessionOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_GetProtectMessageOutputLength(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_GetProtectMessageOutputLengthOptions* Options, uint32_t* OutBufferSizeBytes) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_LogEvent(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_LogEventOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_LogGameRoundEnd(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_LogGameRoundEndOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_LogGameRoundStart(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_LogGameRoundStartOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_LogPlayerDespawn(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_LogPlayerDespawnOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_LogPlayerRevive(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_LogPlayerReviveOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_LogPlayerSpawn(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_LogPlayerSpawnOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_LogPlayerTakeDamage(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_LogPlayerTakeDamageOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_LogPlayerTick(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_LogPlayerTickOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_LogPlayerUseAbility(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_LogPlayerUseAbilityOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_LogPlayerUseWeapon(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_LogPlayerUseWeaponOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_ProtectMessage(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_ProtectMessageOptions* Options, void* OutBuffer, uint32_t* OutBytesWritten) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_ReceiveMessageFromClient(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_ReceiveMessageFromClientOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_RegisterClient(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_RegisterClientOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_RegisterEvent(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_RegisterEventOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_AntiCheatServer_RemoveNotifyClientActionRequired(EOS_HAntiCheatServer Handle, EOS_NotificationId NotificationId) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_AntiCheatServer_RemoveNotifyClientAuthStatusChanged(EOS_HAntiCheatServer Handle, EOS_NotificationId NotificationId) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_AntiCheatServer_RemoveNotifyMessageToClient(EOS_HAntiCheatServer Handle, EOS_NotificationId NotificationId) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_SetClientDetails(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_SetClientDetailsOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_SetClientNetworkState(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_SetClientNetworkStateOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_SetGameSessionId(EOS_HAntiCheatServer Handle, const EOS_AntiCheatCommon_SetGameSessionIdOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_UnprotectMessage(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_UnprotectMessageOptions* Options, void* OutBuffer, uint32_t* OutBytesWritten) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_AntiCheatServer_UnregisterClient(EOS_HAntiCheatServer Handle, const EOS_AntiCheatServer_UnregisterClientOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_ContinuanceToken_ToString(EOS_ContinuanceToken ContinuanceToken, char* OutBuffer, int32_t* InOutBufferLength) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_AcceptRequestToJoin(EOS_HCustomInvites Handle, const EOS_CustomInvites_AcceptRequestToJoinOptions* Options, void* ClientData, const EOS_CustomInvites_OnAcceptRequestToJoinCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifyCustomInviteAccepted(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifyCustomInviteAcceptedOptions* Options, void* ClientData, const EOS_CustomInvites_OnCustomInviteAcceptedCallback NotificationFn) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifyCustomInviteReceived(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifyCustomInviteReceivedOptions* Options, void* ClientData, const EOS_CustomInvites_OnCustomInviteReceivedCallback NotificationFn) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifyCustomInviteRejected(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifyCustomInviteRejectedOptions* Options, void* ClientData, const EOS_CustomInvites_OnCustomInviteRejectedCallback NotificationFn) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifyRequestToJoinAccepted(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifyRequestToJoinAcceptedOptions* Options, void* ClientData, const EOS_CustomInvites_OnRequestToJoinAcceptedCallback NotificationFn) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifyRequestToJoinReceived(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifyRequestToJoinReceivedOptions* Options, void* ClientData, const EOS_CustomInvites_OnRequestToJoinReceivedCallback NotificationFn) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifyRequestToJoinRejected(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifyRequestToJoinRejectedOptions* Options, void* ClientData, const EOS_CustomInvites_OnRequestToJoinRejectedCallback NotificationFn) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifyRequestToJoinResponseReceived(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifyRequestToJoinResponseReceivedOptions* Options, void* ClientData, const EOS_CustomInvites_OnRequestToJoinResponseReceivedCallback NotificationFn) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifySendCustomNativeInviteRequested(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifySendCustomNativeInviteRequestedOptions* Options, void* ClientData, const EOS_CustomInvites_OnSendCustomNativeInviteRequestedCallback NotificationFn) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_CustomInvites_FinalizeInvite(EOS_HCustomInvites Handle, const EOS_CustomInvites_FinalizeInviteOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RejectRequestToJoin(EOS_HCustomInvites Handle, const EOS_CustomInvites_RejectRequestToJoinOptions* Options, void* ClientData, const EOS_CustomInvites_OnRejectRequestToJoinCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifyCustomInviteAccepted(EOS_HCustomInvites Handle, EOS_NotificationId InId) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifyCustomInviteReceived(EOS_HCustomInvites Handle, EOS_NotificationId InId) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifyCustomInviteRejected(EOS_HCustomInvites Handle, EOS_NotificationId InId) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifyRequestToJoinAccepted(EOS_HCustomInvites Handle, EOS_NotificationId InId) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifyRequestToJoinReceived(EOS_HCustomInvites Handle, EOS_NotificationId InId) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifyRequestToJoinRejected(EOS_HCustomInvites Handle, EOS_NotificationId InId) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifyRequestToJoinResponseReceived(EOS_HCustomInvites Handle, EOS_NotificationId InId) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifySendCustomNativeInviteRequested(EOS_HCustomInvites Handle, EOS_NotificationId InId) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_SendCustomInvite(EOS_HCustomInvites Handle, const EOS_CustomInvites_SendCustomInviteOptions* Options, void* ClientData, const EOS_CustomInvites_OnSendCustomInviteCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_SendRequestToJoin(EOS_HCustomInvites Handle, const EOS_CustomInvites_SendRequestToJoinOptions* Options, void* ClientData, const EOS_CustomInvites_OnSendRequestToJoinCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_CustomInvites_SetCustomInvite(EOS_HCustomInvites Handle, const EOS_CustomInvites_SetCustomInviteOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(const char*) EOS_EApplicationStatus_ToString(EOS_EApplicationStatus ApplicationStatus) {
    return (const char*)0;
}

EOS_DECLARE_FUNC(const char*) EOS_ENetworkStatus_ToString(EOS_ENetworkStatus NetworkStatus) {
    return (const char*)0;
}

EOS_DECLARE_FUNC(void) EOS_Ecom_CatalogItem_Release(EOS_Ecom_CatalogItem* CatalogItem) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_Ecom_CatalogRelease_Release(EOS_Ecom_CatalogRelease* CatalogRelease) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementById(EOS_HEcom Handle, const EOS_Ecom_CopyEntitlementByIdOptions* Options, EOS_Ecom_Entitlement ** OutEntitlement) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementByNameAndIndex(EOS_HEcom Handle, const EOS_Ecom_CopyEntitlementByNameAndIndexOptions* Options, EOS_Ecom_Entitlement ** OutEntitlement) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyItemById(EOS_HEcom Handle, const EOS_Ecom_CopyItemByIdOptions* Options, EOS_Ecom_CatalogItem ** OutItem) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyItemImageInfoByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyItemImageInfoByIndexOptions* Options, EOS_Ecom_KeyImageInfo ** OutImageInfo) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyItemReleaseByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyItemReleaseByIndexOptions* Options, EOS_Ecom_CatalogRelease ** OutRelease) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyLastRedeemedEntitlementByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyLastRedeemedEntitlementByIndexOptions* Options, char* OutRedeemedEntitlementId, int32_t* InOutRedeemedEntitlementIdLength) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferById(EOS_HEcom Handle, const EOS_Ecom_CopyOfferByIdOptions* Options, EOS_Ecom_CatalogOffer ** OutOffer) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferImageInfoByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyOfferImageInfoByIndexOptions* Options, EOS_Ecom_KeyImageInfo ** OutImageInfo) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferItemByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyOfferItemByIndexOptions* Options, EOS_Ecom_CatalogItem ** OutItem) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyTransactionById(EOS_HEcom Handle, const EOS_Ecom_CopyTransactionByIdOptions* Options, EOS_Ecom_HTransaction* OutTransaction) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyTransactionByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyTransactionByIndexOptions* Options, EOS_Ecom_HTransaction* OutTransaction) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetEntitlementsByNameCount(EOS_HEcom Handle, const EOS_Ecom_GetEntitlementsByNameCountOptions* Options) {
    return (uint32_t)0;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetItemImageInfoCount(EOS_HEcom Handle, const EOS_Ecom_GetItemImageInfoCountOptions* Options) {
    return (uint32_t)0;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetItemReleaseCount(EOS_HEcom Handle, const EOS_Ecom_GetItemReleaseCountOptions* Options) {
    return (uint32_t)0;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetLastRedeemedEntitlementsCount(EOS_HEcom Handle, const EOS_Ecom_GetLastRedeemedEntitlementsCountOptions* Options) {
    return (uint32_t)0;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferImageInfoCount(EOS_HEcom Handle, const EOS_Ecom_GetOfferImageInfoCountOptions* Options) {
    return (uint32_t)0;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferItemCount(EOS_HEcom Handle, const EOS_Ecom_GetOfferItemCountOptions* Options) {
    return (uint32_t)0;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetTransactionCount(EOS_HEcom Handle, const EOS_Ecom_GetTransactionCountOptions* Options) {
    return (uint32_t)0;
}

EOS_DECLARE_FUNC(void) EOS_Ecom_KeyImageInfo_Release(EOS_Ecom_KeyImageInfo* KeyImageInfo) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryEntitlementToken(EOS_HEcom Handle, const EOS_Ecom_QueryEntitlementTokenOptions* Options, void* ClientData, const EOS_Ecom_OnQueryEntitlementTokenCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOwnership(EOS_HEcom Handle, const EOS_Ecom_QueryOwnershipOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOwnershipBySandboxIds(EOS_HEcom Handle, const EOS_Ecom_QueryOwnershipBySandboxIdsOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipBySandboxIdsCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOwnershipToken(EOS_HEcom Handle, const EOS_Ecom_QueryOwnershipTokenOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipTokenCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_Transaction_CopyEntitlementByIndex(EOS_Ecom_HTransaction Handle, const EOS_Ecom_Transaction_CopyEntitlementByIndexOptions* Options, EOS_Ecom_Entitlement ** OutEntitlement) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_Transaction_GetEntitlementsCount(EOS_Ecom_HTransaction Handle, const EOS_Ecom_Transaction_GetEntitlementsCountOptions* Options) {
    return (uint32_t)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_Transaction_GetTransactionId(EOS_Ecom_HTransaction Handle, char* OutBuffer, int32_t* InOutBufferLength) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_Ecom_Transaction_Release(EOS_Ecom_HTransaction Transaction) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Friends_AddNotifyBlockedUsersUpdate(EOS_HFriends Handle, const EOS_Friends_AddNotifyBlockedUsersUpdateOptions* Options, void* ClientData, const EOS_Friends_OnBlockedUsersUpdateCallback BlockedUsersUpdateHandler) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_Friends_GetBlockedUserAtIndex(EOS_HFriends Handle, const EOS_Friends_GetBlockedUserAtIndexOptions* Options) {
    return (EOS_EpicAccountId)0;
}

EOS_DECLARE_FUNC(int32_t) EOS_Friends_GetBlockedUsersCount(EOS_HFriends Handle, const EOS_Friends_GetBlockedUsersCountOptions* Options) {
    return (int32_t)0;
}

EOS_DECLARE_FUNC(void) EOS_Friends_RemoveNotifyBlockedUsersUpdate(EOS_HFriends Handle, EOS_NotificationId NotificationId) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_IntegratedPlatform_AddNotifyUserLoginStatusChanged(EOS_HIntegratedPlatform Handle, const EOS_IntegratedPlatform_AddNotifyUserLoginStatusChangedOptions* Options, void* ClientData, const EOS_IntegratedPlatform_OnUserLoginStatusChangedCallback CallbackFunction) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(void) EOS_IntegratedPlatform_ClearUserPreLogoutCallback(EOS_HIntegratedPlatform Handle, const EOS_IntegratedPlatform_ClearUserPreLogoutCallbackOptions* Options) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_IntegratedPlatform_FinalizeDeferredUserLogout(EOS_HIntegratedPlatform Handle, const EOS_IntegratedPlatform_FinalizeDeferredUserLogoutOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_IntegratedPlatform_RemoveNotifyUserLoginStatusChanged(EOS_HIntegratedPlatform Handle, EOS_NotificationId NotificationId) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_IntegratedPlatform_SetUserLoginStatus(EOS_HIntegratedPlatform Handle, const EOS_IntegratedPlatform_SetUserLoginStatusOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_IntegratedPlatform_SetUserPreLogoutCallback(EOS_HIntegratedPlatform Handle, const EOS_IntegratedPlatform_SetUserPreLogoutCallbackOptions* Options, void* ClientData, EOS_IntegratedPlatform_OnUserPreLogoutCallback CallbackFunction) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_KWS_AddNotifyPermissionsUpdateReceived(EOS_HKWS Handle, const EOS_KWS_AddNotifyPermissionsUpdateReceivedOptions* Options, void* ClientData, const EOS_KWS_OnPermissionsUpdateReceivedCallback NotificationFn) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_KWS_CopyPermissionByIndex(EOS_HKWS Handle, const EOS_KWS_CopyPermissionByIndexOptions* Options, EOS_KWS_PermissionStatus ** OutPermission) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_KWS_CreateUser(EOS_HKWS Handle, const EOS_KWS_CreateUserOptions* Options, void* ClientData, const EOS_KWS_OnCreateUserCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_KWS_GetPermissionByKey(EOS_HKWS Handle, const EOS_KWS_GetPermissionByKeyOptions* Options, EOS_EKWSPermissionStatus* OutPermission) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(int32_t) EOS_KWS_GetPermissionsCount(EOS_HKWS Handle, const EOS_KWS_GetPermissionsCountOptions* Options) {
    return (int32_t)0;
}

EOS_DECLARE_FUNC(void) EOS_KWS_PermissionStatus_Release(EOS_KWS_PermissionStatus* PermissionStatus) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_KWS_QueryAgeGate(EOS_HKWS Handle, const EOS_KWS_QueryAgeGateOptions* Options, void* ClientData, const EOS_KWS_OnQueryAgeGateCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_KWS_QueryPermissions(EOS_HKWS Handle, const EOS_KWS_QueryPermissionsOptions* Options, void* ClientData, const EOS_KWS_OnQueryPermissionsCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_KWS_RemoveNotifyPermissionsUpdateReceived(EOS_HKWS Handle, EOS_NotificationId InId) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_KWS_RequestPermissions(EOS_HKWS Handle, const EOS_KWS_RequestPermissionsOptions* Options, void* ClientData, const EOS_KWS_OnRequestPermissionsCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_KWS_UpdateParentEmail(EOS_HKWS Handle, const EOS_KWS_UpdateParentEmailOptions* Options, void* ClientData, const EOS_KWS_OnUpdateParentEmailCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardDefinitionByIndex(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardDefinitionByIndexOptions* Options, EOS_Leaderboards_Definition ** OutLeaderboardDefinition) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardIdOptions* Options, EOS_Leaderboards_Definition ** OutLeaderboardDefinition) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardRecordByUserId(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardRecordByUserIdOptions* Options, EOS_Leaderboards_LeaderboardRecord ** OutLeaderboardRecord) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardUserScoreByIndex(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardUserScoreByIndexOptions* Options, EOS_Leaderboards_LeaderboardUserScore ** OutLeaderboardUserScore) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_Leaderboards_Definition_Release(EOS_Leaderboards_Definition* LeaderboardDefinition) {
    (void)0;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Leaderboards_GetLeaderboardDefinitionCount(EOS_HLeaderboards Handle, const EOS_Leaderboards_GetLeaderboardDefinitionCountOptions* Options) {
    return (uint32_t)0;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Leaderboards_GetLeaderboardUserScoreCount(EOS_HLeaderboards Handle, const EOS_Leaderboards_GetLeaderboardUserScoreCountOptions* Options) {
    return (uint32_t)0;
}

EOS_DECLARE_FUNC(void) EOS_Leaderboards_LeaderboardRecord_Release(EOS_Leaderboards_LeaderboardRecord* LeaderboardRecord) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_Leaderboards_QueryLeaderboardDefinitions(EOS_HLeaderboards Handle, const EOS_Leaderboards_QueryLeaderboardDefinitionsOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyMemberInfo(EOS_HLobbyDetails Handle, const EOS_LobbyDetails_CopyMemberInfoOptions* Options, EOS_LobbyDetails_MemberInfo ** OutLobbyDetailsMemberInfo) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_LobbyDetails_MemberInfo_Release(EOS_LobbyDetails_MemberInfo* LobbyDetailsMemberInfo) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_SetAllowedPlatformIds(EOS_HLobbyModification Handle, const EOS_LobbyModification_SetAllowedPlatformIdsOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLeaveLobbyRequested(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLeaveLobbyRequestedOptions* Options, void* ClientData, const EOS_Lobby_OnLeaveLobbyRequestedCallback NotificationFn) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyInviteRejected(EOS_HLobby Handle, const EOS_Lobby_AddNotifyLobbyInviteRejectedOptions* Options, void* ClientData, const EOS_Lobby_OnLobbyInviteRejectedCallback NotificationFn) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifySendLobbyNativeInviteRequested(EOS_HLobby Handle, const EOS_Lobby_AddNotifySendLobbyNativeInviteRequestedOptions* Options, void* ClientData, const EOS_Lobby_OnSendLobbyNativeInviteRequestedCallback NotificationFn) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Lobby_GetInviteCount(EOS_HLobby Handle, const EOS_Lobby_GetInviteCountOptions* Options) {
    return (uint32_t)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_GetInviteIdByIndex(EOS_HLobby Handle, const EOS_Lobby_GetInviteIdByIndexOptions* Options, char* OutBuffer, int32_t* InOutBufferLength) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_HardMuteMember(EOS_HLobby Handle, const EOS_Lobby_HardMuteMemberOptions* Options, void* ClientData, const EOS_Lobby_OnHardMuteMemberCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_QueryInvites(EOS_HLobby Handle, const EOS_Lobby_QueryInvitesOptions* Options, void* ClientData, const EOS_Lobby_OnQueryInvitesCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RejectInvite(EOS_HLobby Handle, const EOS_Lobby_RejectInviteOptions* Options, void* ClientData, const EOS_Lobby_OnRejectInviteCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLeaveLobbyRequested(EOS_HLobby Handle, EOS_NotificationId InId) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyInviteRejected(EOS_HLobby Handle, EOS_NotificationId InId) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifySendLobbyNativeInviteRequested(EOS_HLobby Handle, EOS_NotificationId InId) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Logging_SetCallback(EOS_LogMessageFunc Callback) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Logging_SetLogLevel(EOS_ELogCategory LogCategory, EOS_ELogLevel LogLevel) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Mods_CopyModInfo(EOS_HMods Handle, const EOS_Mods_CopyModInfoOptions* Options, EOS_Mods_ModInfo ** OutEnumeratedMods) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_Mods_EnumerateMods(EOS_HMods Handle, const EOS_Mods_EnumerateModsOptions* Options, void* ClientData, const EOS_Mods_OnEnumerateModsCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_Mods_InstallMod(EOS_HMods Handle, const EOS_Mods_InstallModOptions* Options, void* ClientData, const EOS_Mods_OnInstallModCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_Mods_ModInfo_Release(EOS_Mods_ModInfo* ModInfo) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_Mods_UninstallMod(EOS_HMods Handle, const EOS_Mods_UninstallModOptions* Options, void* ClientData, const EOS_Mods_OnUninstallModCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_Mods_UpdateMod(EOS_HMods Handle, const EOS_Mods_UpdateModOptions* Options, void* ClientData, const EOS_Mods_OnUpdateModCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_GetActiveCountryCode(EOS_HPlatform Handle, EOS_EpicAccountId LocalUserId, char* OutBuffer, int32_t* InOutBufferLength) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_GetActiveLocaleCode(EOS_HPlatform Handle, EOS_EpicAccountId LocalUserId, char* OutBuffer, int32_t* InOutBufferLength) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_HAntiCheatClient) EOS_Platform_GetAntiCheatClientInterface(EOS_HPlatform Handle) {
    return (EOS_HAntiCheatClient)0;
}

EOS_DECLARE_FUNC(EOS_HAntiCheatServer) EOS_Platform_GetAntiCheatServerInterface(EOS_HPlatform Handle) {
    return (EOS_HAntiCheatServer)0;
}

EOS_DECLARE_FUNC(EOS_HCustomInvites) EOS_Platform_GetCustomInvitesInterface(EOS_HPlatform Handle) {
    return (EOS_HCustomInvites)0;
}

EOS_DECLARE_FUNC(EOS_HIntegratedPlatform) EOS_Platform_GetIntegratedPlatformInterface(EOS_HPlatform Handle) {
    return (EOS_HIntegratedPlatform)0;
}

EOS_DECLARE_FUNC(EOS_HKWS) EOS_Platform_GetKWSInterface(EOS_HPlatform Handle) {
    return (EOS_HKWS)0;
}

EOS_DECLARE_FUNC(EOS_HMods) EOS_Platform_GetModsInterface(EOS_HPlatform Handle) {
    return (EOS_HMods)0;
}

EOS_DECLARE_FUNC(EOS_HProgressionSnapshot) EOS_Platform_GetProgressionSnapshotInterface(EOS_HPlatform Handle) {
    return (EOS_HProgressionSnapshot)0;
}

EOS_DECLARE_FUNC(EOS_HRTCAdmin) EOS_Platform_GetRTCAdminInterface(EOS_HPlatform Handle) {
    return (EOS_HRTCAdmin)0;
}

EOS_DECLARE_FUNC(EOS_HReports) EOS_Platform_GetReportsInterface(EOS_HPlatform Handle) {
    /* Non-null: UE5 OnlineServicesEOSGS FPlayerReportsEOSGS::Initialize asserts
       this handle (PlayerReportsEOSGS.cpp:25). Return the platform handle like
       the other interfaces; the Reports_* functions are stubbed. */
    return (EOS_HReports)Handle;
}

/* EOS_Platform_GetSanctionsInterface — implemented in src/sanctions.c */

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_SetOverrideCountryCode(EOS_HPlatform Handle, const char* NewCountryCode) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_SetOverrideLocaleCode(EOS_HPlatform Handle, const char* NewLocaleCode) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorageFileTransferRequest_GetFileRequestState(EOS_HPlayerDataStorageFileTransferRequest Handle) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorageFileTransferRequest_GetFilename(EOS_HPlayerDataStorageFileTransferRequest Handle, uint32_t FilenameStringBufferSizeBytes, char* OutStringBuffer, int32_t* OutStringLength) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorage_CopyFileMetadataByFilename(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_CopyFileMetadataByFilenameOptions* CopyFileMetadataOptions, EOS_PlayerDataStorage_FileMetadata ** OutMetadata) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorage_DeleteCache(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_DeleteCacheOptions* Options, void* ClientData, const EOS_PlayerDataStorage_OnDeleteCacheCompleteCallback CompletionCallback) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_DuplicateFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_DuplicateFileOptions* DuplicateOptions, void* ClientData, const EOS_PlayerDataStorage_OnDuplicateFileCompleteCallback CompletionCallback) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorage_GetFileMetadataCount(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_GetFileMetadataCountOptions* GetFileMetadataCountOptions, int32_t* OutFileMetadataCount) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_QueryFile(EOS_HPlayerDataStorage Handle, const EOS_PlayerDataStorage_QueryFileOptions* QueryFileOptions, void* ClientData, const EOS_PlayerDataStorage_OnQueryFileCompleteCallback CompletionCallback) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_DeleteData(EOS_HPresenceModification Handle, const EOS_PresenceModification_DeleteDataOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetJoinInfo(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetJoinInfoOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetTemplateData(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetTemplateDataOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetTemplateId(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetTemplateIdOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Presence_AddNotifyJoinGameAccepted(EOS_HPresence Handle, const EOS_Presence_AddNotifyJoinGameAcceptedOptions* Options, void* ClientData, const EOS_Presence_OnJoinGameAcceptedCallback NotificationFn) {
    return (EOS_NotificationId)0;
}

/* EOS_Presence_GetJoinInfo — implemented in src/social_bridge.c */

EOS_DECLARE_FUNC(void) EOS_Presence_RemoveNotifyJoinGameAccepted(EOS_HPresence Handle, EOS_NotificationId InId) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_ProgressionSnapshot_AddProgression(EOS_HProgressionSnapshot Handle, const EOS_ProgressionSnapshot_AddProgressionOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_ProgressionSnapshot_BeginSnapshot(EOS_HProgressionSnapshot Handle, const EOS_ProgressionSnapshot_BeginSnapshotOptions* Options, uint32_t* OutSnapshotId) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_ProgressionSnapshot_DeleteSnapshot(EOS_HProgressionSnapshot Handle, const EOS_ProgressionSnapshot_DeleteSnapshotOptions* Options, void* ClientData, const EOS_ProgressionSnapshot_OnDeleteSnapshotCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_ProgressionSnapshot_EndSnapshot(EOS_HProgressionSnapshot Handle, const EOS_ProgressionSnapshot_EndSnapshotOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_ProgressionSnapshot_SubmitSnapshot(EOS_HProgressionSnapshot Handle, const EOS_ProgressionSnapshot_SubmitSnapshotOptions* Options, void* ClientData, const EOS_ProgressionSnapshot_OnSubmitSnapshotCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAdmin_CopyUserTokenByIndex(EOS_HRTCAdmin Handle, const EOS_RTCAdmin_CopyUserTokenByIndexOptions* Options, EOS_RTCAdmin_UserToken ** OutUserToken) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAdmin_CopyUserTokenByUserId(EOS_HRTCAdmin Handle, const EOS_RTCAdmin_CopyUserTokenByUserIdOptions* Options, EOS_RTCAdmin_UserToken ** OutUserToken) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_RTCAdmin_Kick(EOS_HRTCAdmin Handle, const EOS_RTCAdmin_KickOptions* Options, void* ClientData, const EOS_RTCAdmin_OnKickCompleteCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_RTCAdmin_QueryJoinRoomToken(EOS_HRTCAdmin Handle, const EOS_RTCAdmin_QueryJoinRoomTokenOptions* Options, void* ClientData, const EOS_RTCAdmin_OnQueryJoinRoomTokenCompleteCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_RTCAdmin_SetParticipantHardMute(EOS_HRTCAdmin Handle, const EOS_RTCAdmin_SetParticipantHardMuteOptions* Options, void* ClientData, const EOS_RTCAdmin_OnSetParticipantHardMuteCompleteCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_RTCAdmin_UserToken_Release(EOS_RTCAdmin_UserToken* UserToken) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_InputDeviceInformation_Release(EOS_RTCAudio_InputDeviceInformation* DeviceInformation) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_RTCAudio_OutputDeviceInformation_Release(EOS_RTCAudio_OutputDeviceInformation* DeviceInformation) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTCData_AddNotifyDataReceived(EOS_HRTCData Handle, const EOS_RTCData_AddNotifyDataReceivedOptions* Options, void* ClientData, const EOS_RTCData_OnDataReceivedCallback CompletionDelegate) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTCData_AddNotifyParticipantUpdated(EOS_HRTCData Handle, const EOS_RTCData_AddNotifyParticipantUpdatedOptions* Options, void* ClientData, const EOS_RTCData_OnParticipantUpdatedCallback CompletionDelegate) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(void) EOS_RTCData_RemoveNotifyDataReceived(EOS_HRTCData Handle, EOS_NotificationId NotificationId) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_RTCData_RemoveNotifyParticipantUpdated(EOS_HRTCData Handle, EOS_NotificationId NotificationId) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCData_SendData(EOS_HRTCData Handle, const EOS_RTCData_SendDataOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_RTCData_UpdateReceiving(EOS_HRTCData Handle, const EOS_RTCData_UpdateReceivingOptions* Options, void* ClientData, const EOS_RTCData_OnUpdateReceivingCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_RTCData_UpdateSending(EOS_HRTCData Handle, const EOS_RTCData_UpdateSendingOptions* Options, void* ClientData, const EOS_RTCData_OnUpdateSendingCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTC_AddNotifyRoomStatisticsUpdated(EOS_HRTC Handle, const EOS_RTC_AddNotifyRoomStatisticsUpdatedOptions* Options, void* ClientData, const EOS_RTC_OnRoomStatisticsUpdatedCallback StatisticsUpdateHandler) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(EOS_HRTCData) EOS_RTC_GetDataInterface(EOS_HRTC Handle) {
    return (EOS_HRTCData)0;
}

EOS_DECLARE_FUNC(void) EOS_RTC_RemoveNotifyRoomStatisticsUpdated(EOS_HRTC Handle, EOS_NotificationId NotificationId) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTC_SetRoomSetting(EOS_HRTC Handle, const EOS_RTC_SetRoomSettingOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_RTC_SetSetting(EOS_HRTC Handle, const EOS_RTC_SetSettingOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_Reports_SendPlayerBehaviorReport(EOS_HReports Handle, const EOS_Reports_SendPlayerBehaviorReportOptions* Options, void* ClientData, const EOS_Reports_OnSendPlayerBehaviorReportCompleteCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sanctions_CopyPlayerSanctionByIndex(EOS_HSanctions Handle, const EOS_Sanctions_CopyPlayerSanctionByIndexOptions* Options, EOS_Sanctions_PlayerSanction ** OutSanction) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_Sanctions_CreatePlayerSanctionAppeal(EOS_HSanctions Handle, const EOS_Sanctions_CreatePlayerSanctionAppealOptions* Options, void* ClientData, const EOS_Sanctions_CreatePlayerSanctionAppealCallback CompletionDelegate) {
    (void)0;
}

/* EOS_Sanctions_GetPlayerSanctionCount — implemented in src/sanctions.c */

EOS_DECLARE_FUNC(void) EOS_Sanctions_PlayerSanction_Release(EOS_Sanctions_PlayerSanction* Sanction) {
    (void)0;
}

/* EOS_Sanctions_QueryActivePlayerSanctions — implemented in src/sanctions.c */

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetAllowedPlatformIds(EOS_HSessionModification Handle, const EOS_SessionModification_SetAllowedPlatformIdsOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifyLeaveSessionRequested(EOS_HSessions Handle, const EOS_Sessions_AddNotifyLeaveSessionRequestedOptions* Options, void* ClientData, const EOS_Sessions_OnLeaveSessionRequestedCallback NotificationFn) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifySendSessionNativeInviteRequested(EOS_HSessions Handle, const EOS_Sessions_AddNotifySendSessionNativeInviteRequestedOptions* Options, void* ClientData, const EOS_Sessions_OnSendSessionNativeInviteRequestedCallback NotificationFn) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifySessionInviteRejected(EOS_HSessions Handle, const EOS_Sessions_AddNotifySessionInviteRejectedOptions* Options, void* ClientData, const EOS_Sessions_OnSessionInviteRejectedCallback NotificationFn) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CopySessionHandleForPresence(EOS_HSessions Handle, const EOS_Sessions_CopySessionHandleForPresenceOptions* Options, EOS_HSessionDetails* OutSessionHandle) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_DumpSessionState(EOS_HSessions Handle, const EOS_Sessions_DumpSessionStateOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_IsUserInSession(EOS_HSessions Handle, const EOS_Sessions_IsUserInSessionOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifyLeaveSessionRequested(EOS_HSessions Handle, EOS_NotificationId InId) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifySendSessionNativeInviteRequested(EOS_HSessions Handle, EOS_NotificationId InId) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifySessionInviteRejected(EOS_HSessions Handle, EOS_NotificationId InId) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Stats_CopyStatByIndex(EOS_HStats Handle, const EOS_Stats_CopyStatByIndexOptions* Options, EOS_Stats_Stat ** OutStat) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Stats_GetStatsCount(EOS_HStats Handle, const EOS_Stats_GetStatCountOptions* Options) {
    return (uint32_t)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorageFileTransferRequest_CancelRequest(EOS_HTitleStorageFileTransferRequest Handle) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorageFileTransferRequest_GetFileRequestState(EOS_HTitleStorageFileTransferRequest Handle) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorageFileTransferRequest_GetFilename(EOS_HTitleStorageFileTransferRequest Handle, uint32_t FilenameStringBufferSizeBytes, char* OutStringBuffer, int32_t* OutStringLength) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorage_CopyFileMetadataByFilename(EOS_HTitleStorage Handle, const EOS_TitleStorage_CopyFileMetadataByFilenameOptions* Options, EOS_TitleStorage_FileMetadata ** OutMetadata) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(uint32_t) EOS_TitleStorage_GetFileMetadataCount(EOS_HTitleStorage Handle, const EOS_TitleStorage_GetFileMetadataCountOptions* Options) {
    return (uint32_t)0;
}

EOS_DECLARE_FUNC(void) EOS_TitleStorage_QueryFile(EOS_HTitleStorage Handle, const EOS_TitleStorage_QueryFileOptions* Options, void* ClientData, const EOS_TitleStorage_OnQueryFileCompleteCallback CompletionCallback) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_AcknowledgeEventId(EOS_HUI Handle, const EOS_UI_AcknowledgeEventIdOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_UI_AddNotifyMemoryMonitor(EOS_HUI Handle, const EOS_UI_AddNotifyMemoryMonitorOptions* Options, void* ClientData, const EOS_UI_OnMemoryMonitorCallback NotificationFn) {
    return (EOS_NotificationId)0;
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_GetFriendsExclusiveInput(EOS_HUI Handle, const EOS_UI_GetFriendsExclusiveInputOptions* Options) {
    return (EOS_Bool)0;
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_GetFriendsVisible(EOS_HUI Handle, const EOS_UI_GetFriendsVisibleOptions* Options) {
    return (EOS_Bool)0;
}

EOS_DECLARE_FUNC(EOS_UI_ENotificationLocation) EOS_UI_GetNotificationLocationPreference(EOS_HUI Handle) {
    return (EOS_UI_ENotificationLocation)0;
}

EOS_DECLARE_FUNC(EOS_UI_EInputStateButtonFlags) EOS_UI_GetToggleFriendsButton(EOS_HUI Handle, const EOS_UI_GetToggleFriendsButtonOptions* Options) {
    return (EOS_UI_EInputStateButtonFlags)0;
}

EOS_DECLARE_FUNC(EOS_UI_EKeyCombination) EOS_UI_GetToggleFriendsKey(EOS_HUI Handle, const EOS_UI_GetToggleFriendsKeyOptions* Options) {
    return (EOS_UI_EKeyCombination)0;
}

EOS_DECLARE_FUNC(void) EOS_UI_HideFriends(EOS_HUI Handle, const EOS_UI_HideFriendsOptions* Options, void* ClientData, const EOS_UI_OnHideFriendsCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_IsSocialOverlayPaused(EOS_HUI Handle, const EOS_UI_IsSocialOverlayPausedOptions* Options) {
    return (EOS_Bool)0;
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_IsValidButtonCombination(EOS_HUI Handle, EOS_UI_EInputStateButtonFlags ButtonCombination) {
    return (EOS_Bool)0;
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_IsValidKeyCombination(EOS_HUI Handle, EOS_UI_EKeyCombination KeyCombination) {
    return (EOS_Bool)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_PauseSocialOverlay(EOS_HUI Handle, const EOS_UI_PauseSocialOverlayOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_PrePresent(EOS_HUI Handle, const EOS_UI_PrePresentOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_UI_RemoveNotifyMemoryMonitor(EOS_HUI Handle, EOS_NotificationId Id) {
    (void)0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_ReportInputState(EOS_HUI Handle, const EOS_UI_ReportInputStateOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_SetDisplayPreference(EOS_HUI Handle, const EOS_UI_SetDisplayPreferenceOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_SetToggleFriendsButton(EOS_HUI Handle, const EOS_UI_SetToggleFriendsButtonOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_SetToggleFriendsKey(EOS_HUI Handle, const EOS_UI_SetToggleFriendsKeyOptions* Options) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_UI_ShowBlockPlayer(EOS_HUI Handle, const EOS_UI_ShowBlockPlayerOptions* Options, void* ClientData, const EOS_UI_OnShowBlockPlayerCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_UI_ShowNativeProfile(EOS_HUI Handle, const EOS_UI_ShowNativeProfileOptions* Options, void* ClientData, const EOS_UI_OnShowNativeProfileCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_UI_ShowReportPlayer(EOS_HUI Handle, const EOS_UI_ShowReportPlayerOptions* Options, void* ClientData, const EOS_UI_OnShowReportPlayerCallback CompletionDelegate) {
    (void)0;
}

EOS_DECLARE_FUNC(void) EOS_UserInfo_BestDisplayName_Release(EOS_UserInfo_BestDisplayName* BestDisplayName) {
    /* CopyBestDisplayName[WithPlatform] now heap-allocate; free here. Strings are
       static literals, so only the struct is freed. */
    if (BestDisplayName) free(BestDisplayName);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UserInfo_CopyBestDisplayName(EOS_HUserInfo Handle, const EOS_UserInfo_CopyBestDisplayNameOptions* Options, EOS_UserInfo_BestDisplayName ** OutBestDisplayName) {
    /* Return a display name. UE5's FAuthEOS::Login EAS path requires this; a null
       NotConfigured here fails LoginEASImpl (invalid_params) -> no valid Epic
       login -> EOS session type locked. Static strings; Release frees the struct. */
    (void)Handle;
    if (!OutBestDisplayName) return EOS_InvalidParameters;
    EOS_UserInfo_BestDisplayName* n = calloc(1, sizeof(EOS_UserInfo_BestDisplayName));
    if (!n) { *OutBestDisplayName = NULL; return EOS_UnexpectedError; }
    n->ApiVersion = EOS_USERINFO_BESTDISPLAYNAME_API_LATEST;
    n->UserId = Options ? Options->TargetUserId : NULL;
    n->DisplayName = "LAN_Player";
    n->DisplayNameSanitized = "LAN_Player";
    n->Nickname = NULL;
    n->PlatformType = 0;
    *OutBestDisplayName = n;
    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UserInfo_CopyBestDisplayNameWithPlatform(EOS_HUserInfo Handle, const EOS_UserInfo_CopyBestDisplayNameWithPlatformOptions* Options, EOS_UserInfo_BestDisplayName ** OutBestDisplayName) {
    (void)Handle;
    if (!OutBestDisplayName) return EOS_InvalidParameters;
    EOS_UserInfo_BestDisplayName* n = calloc(1, sizeof(EOS_UserInfo_BestDisplayName));
    if (!n) { *OutBestDisplayName = NULL; return EOS_UnexpectedError; }
    n->ApiVersion = EOS_USERINFO_BESTDISPLAYNAME_API_LATEST;
    n->UserId = Options ? Options->TargetUserId : NULL;
    n->DisplayName = "LAN_Player";
    n->DisplayNameSanitized = "LAN_Player";
    n->Nickname = NULL;
    n->PlatformType = Options ? Options->TargetPlatformType : 0;
    *OutBestDisplayName = n;
    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UserInfo_CopyExternalUserInfoByAccountId(EOS_HUserInfo Handle, const EOS_UserInfo_CopyExternalUserInfoByAccountIdOptions* Options, EOS_UserInfo_ExternalUserInfo ** OutExternalUserInfo) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UserInfo_CopyExternalUserInfoByAccountType(EOS_HUserInfo Handle, const EOS_UserInfo_CopyExternalUserInfoByAccountTypeOptions* Options, EOS_UserInfo_ExternalUserInfo ** OutExternalUserInfo) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UserInfo_CopyExternalUserInfoByIndex(EOS_HUserInfo Handle, const EOS_UserInfo_CopyExternalUserInfoByIndexOptions* Options, EOS_UserInfo_ExternalUserInfo ** OutExternalUserInfo) {
    return EOS_NotConfigured;
}

EOS_DECLARE_FUNC(void) EOS_UserInfo_ExternalUserInfo_Release(EOS_UserInfo_ExternalUserInfo* ExternalUserInfo) {
    (void)0;
}

EOS_DECLARE_FUNC(uint32_t) EOS_UserInfo_GetExternalUserInfoCount(EOS_HUserInfo Handle, const EOS_UserInfo_GetExternalUserInfoCountOptions* Options) {
    return (uint32_t)0;
}

EOS_DECLARE_FUNC(EOS_OnlinePlatformType) EOS_UserInfo_GetLocalPlatformType(EOS_HUserInfo Handle, const EOS_UserInfo_GetLocalPlatformTypeOptions* Options) {
    return (EOS_OnlinePlatformType)0;
}

EOS_DECLARE_FUNC(void) EOS_UserInfo_QueryUserInfoByExternalAccount(EOS_HUserInfo Handle, const EOS_UserInfo_QueryUserInfoByExternalAccountOptions* Options, void* ClientData, const EOS_UserInfo_OnQueryUserInfoByExternalAccountCallback CompletionDelegate) {
    (void)0;
}

