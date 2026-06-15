/**
 * EOS-LAN Emulator - Palworld-specific Stubs
 * Minimal stubs for functions Palworld requires but we don't implement
 */

#include "eos/eos_sdk.h"
#include "eos/eos_logging.h"
#include "internal/logging.h"
#include "internal/auth_internal.h"
#include <string.h>

// Suppress unused parameter warnings
#define UNUSED(x) (void)(x)

// ============ Achievements ============
EOS_DECLARE_FUNC(void) EOS_Achievements_QueryDefinitions(EOS_HAchievements Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(uint32_t) EOS_Achievements_GetAchievementDefinitionCount(EOS_HAchievements Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return 0; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyAchievementDefinitionByIndex(EOS_HAchievements Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(void) EOS_Achievements_QueryPlayerAchievements(EOS_HAchievements Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(uint32_t) EOS_Achievements_GetPlayerAchievementCount(EOS_HAchievements Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return 0; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyPlayerAchievementByIndex(EOS_HAchievements Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(void) EOS_Achievements_Definition_Release(void* Def) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Def); }
EOS_DECLARE_FUNC(void) EOS_Achievements_PlayerAchievement_Release(EOS_Achievements_PlayerAchievement* Ach) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Ach); }
// V2 / newer Achievements funcs (EOS SDK 1.16) the stub generator predated.
// MUST be exported or UE5 OnlineServicesEOSGS delay-load crashes 0xc06d007f in
// FAchievementsEOSGS::Initialize (it calls AddNotifyAchievementsUnlockedV2).
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Achievements_AddNotifyAchievementsUnlockedV2(EOS_HAchievements Handle, const void* Options, void* ClientData, void* NotificationFn) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(NotificationFn); return (EOS_NotificationId)1; }
EOS_DECLARE_FUNC(void) EOS_Achievements_RemoveNotifyAchievementsUnlocked(EOS_HAchievements Handle, EOS_NotificationId InId) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(InId); }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Achievements_CopyAchievementDefinitionV2ByIndex(EOS_HAchievements Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(void) EOS_Achievements_UnlockAchievements(EOS_HAchievements Handle, const void* Options, void* ClientData, void* CompletionDelegate) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(CompletionDelegate); }

// ============ ByteArray/Version ============
EOS_DECLARE_FUNC(EOS_EResult) EOS_ByteArray_ToString(const uint8_t* ByteArray, uint32_t Length, char* OutBuffer, uint32_t* InOutBufferLength) {
    UNUSED(ByteArray); UNUSED(Length);
    if (OutBuffer && InOutBufferLength && *InOutBufferLength > 0) { OutBuffer[0] = '\0'; }
    return EOS_Success;
}
EOS_DECLARE_FUNC(const char*) EOS_GetVersion(void) { EOS_LOG_INFO("EOS_GetVersion called - returning 1.16.0"); return "1.16.0"; }

// ============ EpicAccountId ============
EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_EpicAccountId_FromString(const char* String) {
    EOS_LOG_INFO("EpicAccountId_FromString('%s')", String ? String : "NULL");
    extern AuthState g_auth_state;

    // A well-formed EpicAccountId is exactly EPIC_ACCOUNT_ID_LENGTH hex chars. Real
    // EOS returns an INVALID id for anything else, and we MUST too: Steam+EOS games
    // bridging external accounts call FromString on a *Steam ID* string (e.g.
    // "76561198000000001", 17 chars). Handing back the LOCAL user there associates a
    // remote host's puid with the joiner's epic, violating UE's one-puid<->one-epic
    // registry invariant (FUniqueNetIdEOSRegistry asserts InEpicAccountId ==
    // FoundEpicAccountId -> hard crash). NULL makes EpicAccountId_IsValid() return
    // false, so the caller treats the id as absent (PUID-only) instead of mismatched.
    if (!String || strlen(String) != EPIC_ACCOUNT_ID_LENGTH) {
        return NULL;
    }

    // The local user's own id -> the canonical local handle (stable pointer).
    if (g_auth_state.account_id.magic == EAID_MAGIC &&
        strncmp(String, g_auth_state.account_id.id_string, EPIC_ACCOUNT_ID_LENGTH) == 0) {
        return (EOS_EpicAccountId)&g_auth_state.account_id;
    }

    // Remote (peer/host) id. INTERN it so repeated FromString of the same epic
    // returns the SAME handle — real EOS interns account ids and the game compares
    // them. The previous stub returned the LOCAL account for EVERY string, which
    // collapsed the host's identity onto the joiner and left the host user
    // unresolvable during a join (UserKeyHandle:-1 / E011).
    static EOS_EpicAccountIdDetails g_interned_epics[128];
    static int g_interned_epic_count = 0;
    for (int i = 0; i < g_interned_epic_count; i++) {
        if (strncmp(g_interned_epics[i].id_string, String, EPIC_ACCOUNT_ID_LENGTH) == 0) {
            return (EOS_EpicAccountId)&g_interned_epics[i];
        }
    }
    if (g_interned_epic_count >= (int)(sizeof(g_interned_epics) / sizeof(g_interned_epics[0]))) {
        EOS_LOG_WARN("EpicAccountId_FromString: intern table full, returning NULL");
        return NULL;
    }
    EOS_EpicAccountIdDetails* id = &g_interned_epics[g_interned_epic_count++];
    id->magic = EAID_MAGIC;
    strncpy(id->id_string, String, EPIC_ACCOUNT_ID_LENGTH);
    id->id_string[EPIC_ACCOUNT_ID_LENGTH] = '\0';
    EOS_LOG_INFO("EpicAccountId_FromString: interned new remote id %s", id->id_string);
    return (EOS_EpicAccountId)id;
}
EOS_DECLARE_FUNC(EOS_Bool) EOS_EpicAccountId_IsValid(EOS_EpicAccountId Id) {
    extern AuthState g_auth_state;

    // Log all the details to understand what the game is passing
    EOS_LOG_INFO(">>> EpicAccountId_IsValid(%p) called", (void*)Id);
    EOS_LOG_INFO("    logged_in=%d, our_account_ptr=%p, our_account_id=%s",
                 g_auth_state.logged_in,
                 (void*)&g_auth_state.account_id,
                 g_auth_state.account_id.id_string);

    if (Id == NULL) {
        EOS_LOG_INFO("    -> Result: FALSE (NULL pointer)");
        return EOS_FALSE;
    }

    // Try to examine what the game passed us
    EOS_EpicAccountIdDetails* details = (EOS_EpicAccountIdDetails*)Id;
    EOS_LOG_INFO("    passed magic=0x%08X (expected 0x%08X)", details->magic, EAID_MAGIC);

    if (details->magic == EAID_MAGIC) {
        EOS_LOG_INFO("    passed id_string=%s", details->id_string);
    } else {
        // Try to dump first 16 bytes as hex in case it's a different format
        unsigned char* bytes = (unsigned char*)Id;
        EOS_LOG_INFO("    raw bytes: %02X %02X %02X %02X %02X %02X %02X %02X",
                     bytes[0], bytes[1], bytes[2], bytes[3],
                     bytes[4], bytes[5], bytes[6], bytes[7]);
    }

    // Check if this is OUR account ID (pointer comparison)
    if (Id == (EOS_EpicAccountId)&g_auth_state.account_id) {
        EOS_LOG_INFO("    -> Result: TRUE (matches our account_id pointer)");
        return EOS_TRUE;
    }

    // Check if valid by magic number
    if (details->magic == EAID_MAGIC && g_auth_state.logged_in) {
        EOS_LOG_INFO("    -> Result: TRUE (valid magic, logged in)");
        return EOS_TRUE;
    }

    EOS_LOG_INFO("    -> Result: FALSE (magic mismatch or not logged in)");
    return EOS_FALSE;
}
EOS_DECLARE_FUNC(EOS_EResult) EOS_EpicAccountId_ToString(EOS_EpicAccountId Id, char* OutBuffer, int32_t* InOutBufferLength) {
    extern AuthState g_auth_state;
    if (!OutBuffer || !InOutBufferLength) return EOS_InvalidParameters;

    // Stringify the ACTUAL id passed in — this is the whole point: a peer/host id
    // must render as ITS OWN string, not the local user's. The old stub always
    // emitted g_auth_state.account_id, so every remote id printed as the local
    // user -> host identity collapsed onto the joiner (UserKeyHandle:-1).
    const char* src = NULL;
    EOS_EpicAccountIdDetails* d = (EOS_EpicAccountIdDetails*)Id;
    if (d && d->magic == EAID_MAGIC) {
        src = d->id_string;
    } else if (!Id && g_auth_state.logged_in) {
        src = g_auth_state.account_id.id_string;  // back-compat: NULL -> local user
    }
    if (!src) {
        if (*InOutBufferLength > 0) OutBuffer[0] = '\0';
        EOS_LOG_WARN("EpicAccountId_ToString: invalid Id %p", (void*)Id);
        return EOS_InvalidUser;
    }

    int32_t required = EPIC_ACCOUNT_ID_LENGTH + 1;
    if (*InOutBufferLength < required) {
        *InOutBufferLength = required;
        return EOS_LimitExceeded;
    }
    strcpy(OutBuffer, src);
    *InOutBufferLength = required;
    EOS_LOG_INFO("EpicAccountId_ToString(Id=%p) -> %s", (void*)Id, OutBuffer);
    return EOS_Success;
}

// ============ EResult ============
EOS_DECLARE_FUNC(EOS_Bool) EOS_EResult_IsOperationComplete(EOS_EResult Result) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Result); return EOS_TRUE; }

// ============ Ecom ============
EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOffers(EOS_HEcom Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferCount(EOS_HEcom Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return 0; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferByIndex(EOS_HEcom Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(void) EOS_Ecom_CatalogOffer_Release(EOS_Ecom_CatalogOffer* Offer) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Offer); }
EOS_DECLARE_FUNC(void) EOS_Ecom_QueryEntitlements(EOS_HEcom Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetEntitlementsCount(EOS_HEcom Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return 0; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementByIndex(EOS_HEcom Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(void) EOS_Ecom_Entitlement_Release(EOS_Ecom_Entitlement* Ent) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Ent); }
EOS_DECLARE_FUNC(void) EOS_Ecom_Checkout(EOS_HEcom Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(void) EOS_Ecom_RedeemEntitlements(EOS_HEcom Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }

// ============ Friends ============
// QueryFriends / GetFriendsCount / GetFriendAtIndex / GetStatus are now
// implemented in src/social_bridge.c (they expose LAN-discovered peers as
// friends so games join via the friends UI).
EOS_DECLARE_FUNC(void) EOS_Friends_SendInvite(EOS_HFriends Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(void) EOS_Friends_AcceptInvite(EOS_HFriends Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(void) EOS_Friends_RejectInvite(EOS_HFriends Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
// EOS_Friends_AddNotifyFriendsUpdate / RemoveNotifyFriendsUpdate implemented in src/social_bridge.c

// ============ Leaderboards ============
EOS_DECLARE_FUNC(void) EOS_Leaderboards_QueryLeaderboardRanks(EOS_HLeaderboards Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(uint32_t) EOS_Leaderboards_GetLeaderboardRecordCount(EOS_HLeaderboards Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return 0; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardRecordByIndex(EOS_HLeaderboards Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(void) EOS_Leaderboards_QueryLeaderboardUserScores(EOS_HLeaderboards Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardUserScoreByUserId(EOS_HLeaderboards Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(void) EOS_Leaderboards_LeaderboardUserScore_Release(EOS_Leaderboards_LeaderboardUserScore* Score) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Score); }

// ============ Lobby ============
// NOTE: the real Lobby interface now lives in lobby.c / lobby_modification.c /
// lobby_search.c / lobby_details.c. Only EOS_Lobby_SendInvite remains a stub
// (no LAN invite system; Palworld co-op uses search + join, not invites).
EOS_DECLARE_FUNC(void) EOS_Lobby_SendInvite(EOS_HLobby Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }

// ============ Metrics ============
EOS_DECLARE_FUNC(EOS_EResult) EOS_Metrics_BeginPlayerSession(EOS_HMetrics Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return EOS_Success; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Metrics_EndPlayerSession(EOS_HMetrics Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return EOS_Success; }

// ============ Platform Interface Getters ============
EOS_DECLARE_FUNC(EOS_HAchievements) EOS_Platform_GetAchievementsInterface(EOS_HPlatform Handle) { EOS_LOG_WARN("STUB called: %s", __func__); return (void*)Handle; }
EOS_DECLARE_FUNC(EOS_HLeaderboards) EOS_Platform_GetLeaderboardsInterface(EOS_HPlatform Handle) { EOS_LOG_WARN("STUB called: %s", __func__); return (void*)Handle; }
EOS_DECLARE_FUNC(EOS_HStats) EOS_Platform_GetStatsInterface(EOS_HPlatform Handle) { EOS_LOG_WARN("STUB called: %s", __func__); return (void*)Handle; }
EOS_DECLARE_FUNC(EOS_HPlayerDataStorage) EOS_Platform_GetPlayerDataStorageInterface(EOS_HPlatform Handle) { EOS_LOG_WARN("STUB called: %s", __func__); return (void*)Handle; }
EOS_DECLARE_FUNC(EOS_HTitleStorage) EOS_Platform_GetTitleStorageInterface(EOS_HPlatform Handle) { EOS_LOG_WARN("STUB called: %s", __func__); return (void*)Handle; }
EOS_DECLARE_FUNC(EOS_HRTC) EOS_Platform_GetRTCInterface(EOS_HPlatform Handle) { EOS_LOG_WARN("STUB called: %s", __func__); return (void*)Handle; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_GetDesktopCrossplayStatus(EOS_HPlatform Handle, const void* Options, EOS_Platform_DesktopCrossplayStatusInfo* OutDesktopCrossplayStatusInfo) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(OutDesktopCrossplayStatusInfo); return EOS_NotConfigured; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_GetOverrideCountryCode(EOS_HPlatform Handle, char* Out, int32_t* Len) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Out); UNUSED(Len); return EOS_NotFound; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_GetOverrideLocaleCode(EOS_HPlatform Handle, char* Out, int32_t* Len) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Out); UNUSED(Len); return EOS_NotFound; }

// ============ PlayerDataStorage ============
EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_QueryFileList(EOS_HPlayerDataStorage Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorage_CopyFileMetadataAtIndex(EOS_HPlayerDataStorage Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(void*) EOS_PlayerDataStorage_ReadFile(EOS_HPlayerDataStorage Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return NULL; }
EOS_DECLARE_FUNC(void*) EOS_PlayerDataStorage_WriteFile(EOS_HPlayerDataStorage Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return NULL; }
EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_DeleteFile(EOS_HPlayerDataStorage Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_FileMetadata_Release(EOS_PlayerDataStorage_FileMetadata* Meta) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Meta); }
EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorageFileTransferRequest_CancelRequest(EOS_HPlayerDataStorageFileTransferRequest Handle) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); return EOS_Success; }
EOS_DECLARE_FUNC(void) EOS_PlayerDataStorageFileTransferRequest_Release(EOS_HPlayerDataStorageFileTransferRequest Handle) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); }

// ============ Presence ============
// QueryPresence / HasPresence / CopyPresence / GetJoinInfo / Info_Release and
// the full PresenceModification surface (CreatePresenceModification /
// SetPresence / SetStatus / SetRawRichText / SetData / Release) are
// implemented in src/social_bridge.c (peer presence + join-info bridging).

// ============ RTC ============
EOS_DECLARE_FUNC(void*) EOS_RTC_GetAudioInterface(EOS_HRTC Handle) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); return NULL; }
EOS_DECLARE_FUNC(void) EOS_RTC_JoinRoom(EOS_HRTC Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(void) EOS_RTC_LeaveRoom(EOS_HRTC Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(void) EOS_RTC_BlockParticipant(EOS_HRTC Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTC_AddNotifyDisconnected(EOS_HRTC Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_INVALID_NOTIFICATIONID; }
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTC_AddNotifyParticipantStatusChanged(EOS_HRTC Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_INVALID_NOTIFICATIONID; }
EOS_DECLARE_FUNC(void) EOS_RTC_RemoveNotifyDisconnected(EOS_HRTC Handle, EOS_NotificationId Id) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Id); }
EOS_DECLARE_FUNC(void) EOS_RTC_RemoveNotifyParticipantStatusChanged(EOS_HRTC Handle, EOS_NotificationId Id) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Id); }

// ============ RTCAudio (use void* for handle since EOS_HRTC_Audio not defined) ============
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTCAudio_AddNotifyParticipantUpdated(void* Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_INVALID_NOTIFICATIONID; }
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTCAudio_AddNotifyAudioDevicesChanged(void* Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_INVALID_NOTIFICATIONID; }
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTCAudio_AddNotifyAudioInputState(void* Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_INVALID_NOTIFICATIONID; }
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTCAudio_AddNotifyAudioBeforeSend(void* Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_INVALID_NOTIFICATIONID; }
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_RTCAudio_AddNotifyAudioBeforeRender(void* Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_INVALID_NOTIFICATIONID; }
EOS_DECLARE_FUNC(void) EOS_RTCAudio_RemoveNotifyParticipantUpdated(void* Handle, EOS_NotificationId Id) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Id); }
EOS_DECLARE_FUNC(void) EOS_RTCAudio_RemoveNotifyAudioDevicesChanged(void* Handle, EOS_NotificationId Id) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Id); }
EOS_DECLARE_FUNC(void) EOS_RTCAudio_RemoveNotifyAudioInputState(void* Handle, EOS_NotificationId Id) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Id); }
EOS_DECLARE_FUNC(void) EOS_RTCAudio_RemoveNotifyAudioBeforeSend(void* Handle, EOS_NotificationId Id) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Id); }
EOS_DECLARE_FUNC(void) EOS_RTCAudio_RemoveNotifyAudioBeforeRender(void* Handle, EOS_NotificationId Id) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Id); }
EOS_DECLARE_FUNC(uint32_t) EOS_RTCAudio_GetAudioInputDevicesCount(void* Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return 0; }
EOS_DECLARE_FUNC(uint32_t) EOS_RTCAudio_GetAudioOutputDevicesCount(void* Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return 0; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_GetAudioInputDeviceByIndex(void* Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_GetAudioOutputDeviceByIndex(void* Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_SetAudioInputSettings(void* Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return EOS_Success; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_SetAudioOutputSettings(void* Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return EOS_Success; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_UpdateSending(void* Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_Success; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_UpdateReceiving(void* Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_Success; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_RTCAudio_UpdateParticipantVolume(void* Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_Success; }

// ============ Stats ============
EOS_DECLARE_FUNC(void) EOS_Stats_IngestStat(EOS_HStats Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(void) EOS_Stats_QueryStats(EOS_HStats Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Stats_CopyStatByName(EOS_HStats Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(void) EOS_Stats_Stat_Release(EOS_Stats_Stat* Stat) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Stat); }

// ============ TitleStorage ============
EOS_DECLARE_FUNC(void) EOS_TitleStorage_QueryFileList(EOS_HTitleStorage Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorage_CopyFileMetadataAtIndex(EOS_HTitleStorage Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(void*) EOS_TitleStorage_ReadFile(EOS_HTitleStorage Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return NULL; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorage_DeleteCache(EOS_HTitleStorage Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_Success; }
EOS_DECLARE_FUNC(void) EOS_TitleStorage_FileMetadata_Release(EOS_TitleStorage_FileMetadata* Meta) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Meta); }
EOS_DECLARE_FUNC(void) EOS_TitleStorageFileTransferRequest_Release(EOS_HTitleStorageFileTransferRequest Handle) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); }

// ============ UI ============
EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_ShowFriends(EOS_HUI Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_NotConfigured; }
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_UI_AddNotifyDisplaySettingsUpdated(EOS_HUI Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_INVALID_NOTIFICATIONID; }
EOS_DECLARE_FUNC(void) EOS_UI_RemoveNotifyDisplaySettingsUpdated(EOS_HUI Handle, EOS_NotificationId Id) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Id); }

// ============ UserInfo ============
// QueryUserInfo / QueryUserInfoByDisplayName / CopyUserInfo / Release are
// implemented in src/social_bridge.c (completing callbacks + per-target
// display names from user beacons).
