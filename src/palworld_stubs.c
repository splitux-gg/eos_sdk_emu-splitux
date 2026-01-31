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
EOS_DECLARE_FUNC(void) EOS_Achievements_PlayerAchievement_Release(void* Ach) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Ach); }

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
    // Return our global auth account if we have one
    extern AuthState g_auth_state;
    if (g_auth_state.logged_in) {
        return (EOS_EpicAccountId)&g_auth_state.account_id;
    }
    return NULL;
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
    // DEBUG: Accept NULL Id and return our account_id anyway (game might pass NULL)
    if (OutBuffer && InOutBufferLength && *InOutBufferLength > 32 && g_auth_state.logged_in) {
        strncpy(OutBuffer, g_auth_state.account_id.id_string, *InOutBufferLength - 1);
        OutBuffer[*InOutBufferLength - 1] = '\0';
        EOS_LOG_INFO("EpicAccountId_ToString(Id=%p) -> %s", (void*)Id, OutBuffer);
        return EOS_Success;
    }
    if (OutBuffer && InOutBufferLength && *InOutBufferLength > 0) { OutBuffer[0] = '\0'; }
    EOS_LOG_WARN("EpicAccountId_ToString failed - buffer or logged_in issue");
    return EOS_InvalidParameters;
}

// ============ EResult ============
EOS_DECLARE_FUNC(EOS_Bool) EOS_EResult_IsOperationComplete(EOS_EResult Result) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Result); return EOS_TRUE; }

// ============ Ecom ============
EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOffers(EOS_HEcom Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferCount(EOS_HEcom Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return 0; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferByIndex(EOS_HEcom Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(void) EOS_Ecom_CatalogOffer_Release(void* Offer) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Offer); }
EOS_DECLARE_FUNC(void) EOS_Ecom_QueryEntitlements(EOS_HEcom Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetEntitlementsCount(EOS_HEcom Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return 0; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementByIndex(EOS_HEcom Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(void) EOS_Ecom_Entitlement_Release(void* Ent) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Ent); }
EOS_DECLARE_FUNC(void) EOS_Ecom_Checkout(EOS_HEcom Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(void) EOS_Ecom_RedeemEntitlements(EOS_HEcom Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }

// ============ Friends ============
EOS_DECLARE_FUNC(void) EOS_Friends_QueryFriends(EOS_HFriends Handle, const void* Options, void* ClientData, void* Callback) {
    EOS_LOG_INFO(">>> EOS_Friends_QueryFriends CALLED - firing callback with success");
    typedef void (*QueryFriendsCallback)(const void*);
    if (Callback) {
        // Create a minimal callback info - just ResultCode and ClientData
        struct {
            EOS_EResult ResultCode;
            void* ClientData;
            EOS_EpicAccountId LocalUserId;
        } info = {0};
        info.ResultCode = EOS_Success;
        info.ClientData = ClientData;
        info.LocalUserId = NULL;  // No local user for now
        ((QueryFriendsCallback)Callback)(&info);
        EOS_LOG_INFO("    QueryFriends callback fired");
    }
}
EOS_DECLARE_FUNC(int32_t) EOS_Friends_GetFriendsCount(EOS_HFriends Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return 0; }
EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_Friends_GetFriendAtIndex(EOS_HFriends Handle, const void* Options, int32_t Index) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(Index); return NULL; }
EOS_DECLARE_FUNC(int32_t) EOS_Friends_GetStatus(EOS_HFriends Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return 0; }
EOS_DECLARE_FUNC(void) EOS_Friends_SendInvite(EOS_HFriends Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(void) EOS_Friends_AcceptInvite(EOS_HFriends Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(void) EOS_Friends_RejectInvite(EOS_HFriends Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Friends_AddNotifyFriendsUpdate(EOS_HFriends Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_INVALID_NOTIFICATIONID; }
EOS_DECLARE_FUNC(void) EOS_Friends_RemoveNotifyFriendsUpdate(EOS_HFriends Handle, EOS_NotificationId Id) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Id); }

// ============ Leaderboards ============
EOS_DECLARE_FUNC(void) EOS_Leaderboards_QueryLeaderboardRanks(EOS_HLeaderboards Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(uint32_t) EOS_Leaderboards_GetLeaderboardRecordCount(EOS_HLeaderboards Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return 0; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardRecordByIndex(EOS_HLeaderboards Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(void) EOS_Leaderboards_QueryLeaderboardUserScores(EOS_HLeaderboards Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardUserScoreByUserId(EOS_HLeaderboards Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(void) EOS_Leaderboards_LeaderboardUserScore_Release(void* Score) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Score); }

// ============ Lobby ============
EOS_DECLARE_FUNC(void) EOS_Lobby_CreateLobby(EOS_HLobby Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(void) EOS_Lobby_JoinLobby(EOS_HLobby Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(void) EOS_Lobby_LeaveLobby(EOS_HLobby Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(void) EOS_Lobby_UpdateLobby(EOS_HLobby Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(void) EOS_Lobby_SendInvite(EOS_HLobby Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(void) EOS_Lobby_KickMember(EOS_HLobby Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_UpdateLobbyModification(EOS_HLobby Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_CopyLobbyDetailsHandle(EOS_HLobby Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_CopyLobbyDetailsHandleByInviteId(EOS_HLobby Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_CopyLobbyDetailsHandleByUiEventId(EOS_HLobby Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_CreateLobbySearch(EOS_HLobby Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_GetRTCRoomName(EOS_HLobby Handle, const void* Options, char* Out, uint32_t* Len) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(Out); UNUSED(Len); return EOS_NotFound; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Lobby_IsRTCRoomConnected(EOS_HLobby Handle, const void* Options, EOS_Bool* Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = EOS_FALSE; return EOS_Success; }
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyUpdateReceived(EOS_HLobby Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_INVALID_NOTIFICATIONID; }
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyMemberUpdateReceived(EOS_HLobby Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_INVALID_NOTIFICATIONID; }
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyMemberStatusReceived(EOS_HLobby Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_INVALID_NOTIFICATIONID; }
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyJoinLobbyAccepted(EOS_HLobby Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_INVALID_NOTIFICATIONID; }
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyLobbyInviteAccepted(EOS_HLobby Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_INVALID_NOTIFICATIONID; }
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Lobby_AddNotifyRTCRoomConnectionChanged(EOS_HLobby Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_INVALID_NOTIFICATIONID; }
EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyUpdateReceived(EOS_HLobby Handle, EOS_NotificationId Id) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Id); }
EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyMemberUpdateReceived(EOS_HLobby Handle, EOS_NotificationId Id) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Id); }
EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyMemberStatusReceived(EOS_HLobby Handle, EOS_NotificationId Id) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Id); }
EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyJoinLobbyAccepted(EOS_HLobby Handle, EOS_NotificationId Id) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Id); }
EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyLobbyInviteAccepted(EOS_HLobby Handle, EOS_NotificationId Id) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Id); }
EOS_DECLARE_FUNC(void) EOS_Lobby_RemoveNotifyRTCRoomConnectionChanged(EOS_HLobby Handle, EOS_NotificationId Id) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Id); }
EOS_DECLARE_FUNC(void) EOS_Lobby_Attribute_Release(void* Attr) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Attr); }

// ============ LobbyModification ============
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_SetPermissionLevel(void* Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return EOS_Success; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_SetMaxMembers(void* Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return EOS_Success; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_AddAttribute(void* Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return EOS_Success; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_AddMemberAttribute(void* Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return EOS_Success; }
EOS_DECLARE_FUNC(void) EOS_LobbyModification_Release(void* Handle) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); }

// ============ LobbyDetails ============
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyInfo(void* Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(uint32_t) EOS_LobbyDetails_GetAttributeCount(void* Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return 0; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyAttributeByIndex(void* Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(uint32_t) EOS_LobbyDetails_GetMemberCount(void* Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return 0; }
EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_LobbyDetails_GetMemberByIndex(void* Handle, const void* Options, uint32_t Index) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(Index); return NULL; }
EOS_DECLARE_FUNC(uint32_t) EOS_LobbyDetails_GetMemberAttributeCount(void* Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return 0; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyMemberAttributeByIndex(void* Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(void) EOS_LobbyDetails_Release(void* Handle) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); }
EOS_DECLARE_FUNC(void) EOS_LobbyDetails_Info_Release(void* Info) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Info); }

// ============ LobbySearch ============
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetLobbyId(void* Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return EOS_Success; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetTargetUserId(void* Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return EOS_Success; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetParameter(void* Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return EOS_Success; }
EOS_DECLARE_FUNC(void) EOS_LobbySearch_Find(void* Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(uint32_t) EOS_LobbySearch_GetSearchResultCount(void* Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return 0; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_CopySearchResultByIndex(void* Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(void) EOS_LobbySearch_Release(void* Handle) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); }

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
EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_GetDesktopCrossplayStatus(EOS_HPlatform Handle, const void* Options, void* Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(Out); return EOS_NotConfigured; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_GetOverrideCountryCode(EOS_HPlatform Handle, char* Out, int32_t* Len) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Out); UNUSED(Len); return EOS_NotFound; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Platform_GetOverrideLocaleCode(EOS_HPlatform Handle, char* Out, int32_t* Len) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Out); UNUSED(Len); return EOS_NotFound; }

// ============ PlayerDataStorage ============
EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_QueryFileList(EOS_HPlayerDataStorage Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorage_CopyFileMetadataAtIndex(EOS_HPlayerDataStorage Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(void*) EOS_PlayerDataStorage_ReadFile(EOS_HPlayerDataStorage Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return NULL; }
EOS_DECLARE_FUNC(void*) EOS_PlayerDataStorage_WriteFile(EOS_HPlayerDataStorage Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return NULL; }
EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_DeleteFile(EOS_HPlayerDataStorage Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_FileMetadata_Release(void* Meta) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Meta); }
EOS_DECLARE_FUNC(EOS_EResult) EOS_PlayerDataStorageFileTransferRequest_CancelRequest(void* Handle) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); return EOS_Success; }
EOS_DECLARE_FUNC(void) EOS_PlayerDataStorageFileTransferRequest_Release(void* Handle) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); }

// ============ Presence ============
EOS_DECLARE_FUNC(void) EOS_Presence_QueryPresence(EOS_HPresence Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(EOS_Bool) EOS_Presence_HasPresence(EOS_HPresence Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return EOS_FALSE; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Presence_CopyPresence(EOS_HPresence Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_Presence_CreatePresenceModification(EOS_HPresence Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(void) EOS_Presence_SetPresence(EOS_HPresence Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Presence_AddNotifyOnPresenceChanged(EOS_HPresence Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_INVALID_NOTIFICATIONID; }
EOS_DECLARE_FUNC(void) EOS_Presence_RemoveNotifyOnPresenceChanged(EOS_HPresence Handle, EOS_NotificationId Id) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Id); }
EOS_DECLARE_FUNC(void) EOS_Presence_Info_Release(void* Info) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Info); }
EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetStatus(void* Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return EOS_Success; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetRawRichText(void* Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return EOS_Success; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetData(void* Handle, const void* Options) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); return EOS_Success; }
EOS_DECLARE_FUNC(void) EOS_PresenceModification_Release(void* Handle) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); }

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
EOS_DECLARE_FUNC(void) EOS_Stats_Stat_Release(void* Stat) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Stat); }

// ============ TitleStorage ============
EOS_DECLARE_FUNC(void) EOS_TitleStorage_QueryFileList(EOS_HTitleStorage Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorage_CopyFileMetadataAtIndex(EOS_HTitleStorage Handle, const void* Options, void** Out) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); if(Out) *Out = NULL; return EOS_NotFound; }
EOS_DECLARE_FUNC(void*) EOS_TitleStorage_ReadFile(EOS_HTitleStorage Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return NULL; }
EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorage_DeleteCache(EOS_HTitleStorage Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_Success; }
EOS_DECLARE_FUNC(void) EOS_TitleStorage_FileMetadata_Release(void* Meta) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Meta); }
EOS_DECLARE_FUNC(void) EOS_TitleStorageFileTransferRequest_Release(void* Handle) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); }

// ============ UI ============
EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_ShowFriends(EOS_HUI Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_NotConfigured; }
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_UI_AddNotifyDisplaySettingsUpdated(EOS_HUI Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); return EOS_INVALID_NOTIFICATIONID; }
EOS_DECLARE_FUNC(void) EOS_UI_RemoveNotifyDisplaySettingsUpdated(EOS_HUI Handle, EOS_NotificationId Id) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Id); }

// ============ UserInfo ============
// Static user info struct for LAN player
static struct {
    int32_t ApiVersion;
    EOS_EpicAccountId UserId;
    const char* Country;
    const char* DisplayName;
    const char* PreferredLanguage;
    const char* Nickname;
    const char* DisplayNameSanitized;
} g_lan_user_info = {0};

EOS_DECLARE_FUNC(void) EOS_UserInfo_QueryUserInfo(EOS_HUserInfo Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(void) EOS_UserInfo_QueryUserInfoByDisplayName(EOS_HUserInfo Handle, const void* Options, void* ClientData, void* Callback) { EOS_LOG_WARN("STUB called: %s", __func__); UNUSED(Handle); UNUSED(Options); UNUSED(ClientData); UNUSED(Callback); }
EOS_DECLARE_FUNC(EOS_EResult) EOS_UserInfo_CopyUserInfo(EOS_HUserInfo Handle, const void* Options, void** Out) {
    extern AuthState g_auth_state;
    EOS_LOG_INFO(">>> EOS_UserInfo_CopyUserInfo CALLED");
    if (Out && g_auth_state.logged_in) {
        // Initialize the static user info
        g_lan_user_info.ApiVersion = 3;
        g_lan_user_info.UserId = (EOS_EpicAccountId)&g_auth_state.account_id;
        g_lan_user_info.Country = "US";
        g_lan_user_info.DisplayName = "LAN_Player";
        g_lan_user_info.PreferredLanguage = "en";
        g_lan_user_info.Nickname = "LAN_Player";
        g_lan_user_info.DisplayNameSanitized = "LAN_Player";
        *Out = &g_lan_user_info;
        EOS_LOG_INFO("    Returning user info for %s", g_lan_user_info.DisplayName);
        return EOS_Success;
    }
    EOS_LOG_WARN("    CopyUserInfo failed - Out=%p, logged_in=%d", Out, g_auth_state.logged_in);
    if(Out) *Out = NULL;
    return EOS_NotFound;
}
EOS_DECLARE_FUNC(void) EOS_UserInfo_Release(void* Info) {
    EOS_LOG_INFO(">>> EOS_UserInfo_Release CALLED");
    // Static struct, nothing to free
}
