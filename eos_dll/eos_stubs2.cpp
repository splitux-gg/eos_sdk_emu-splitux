/*
 * Copyright (C) 2020 Nemirtingas
 * This file is part of the Nemirtingas's Epic Emulator
 *
 * Stub implementations for remaining missing EOS SDK functions
 */

#include "common_includes.h"

// Forward declarations for types not in current SDK version

// Connect Logout types (SDK 1.16+)
#ifndef EOS_CONNECT_LOGOUT_API_LATEST
#define EOS_CONNECT_LOGOUT_API_LATEST 1
EOS_STRUCT(EOS_Connect_LogoutOptions, (
    int32_t ApiVersion;
    EOS_ProductUserId LocalUserId;
));
typedef struct EOS_Connect_LogoutCallbackInfoInternal {
    EOS_EResult ResultCode;
    void* ClientData;
    EOS_ProductUserId LocalUserId;
} EOS_Connect_LogoutCallbackInfo;
typedef void (EOS_CALL* EOS_Connect_OnLogoutCallback)(const EOS_Connect_LogoutCallbackInfo* Data);
#endif

// CustomInvites Rejected types
#ifndef EOS_CUSTOMINVITES_ADDNOTIFYCUSTOMINVITEREJECTED_API_LATEST
#define EOS_CUSTOMINVITES_ADDNOTIFYCUSTOMINVITEREJECTED_API_LATEST 1
EOS_STRUCT(EOS_CustomInvites_AddNotifyCustomInviteRejectedOptions, (
    int32_t ApiVersion;
));
typedef void (EOS_CALL* EOS_CustomInvites_OnCustomInviteRejectedCallback)(const void* Data);
#endif

// CustomInvites SendRequestToJoin types
#ifndef EOS_CUSTOMINVITES_SENDREQUESTTOJOIN_API_LATEST
#define EOS_CUSTOMINVITES_SENDREQUESTTOJOIN_API_LATEST 1
EOS_STRUCT(EOS_CustomInvites_SendRequestToJoinOptions, (
    int32_t ApiVersion;
    EOS_ProductUserId LocalUserId;
    EOS_ProductUserId TargetUserId;
));
typedef struct EOS_CustomInvites_SendRequestToJoinCallbackInfoInternal {
    EOS_EResult ResultCode;
    void* ClientData;
    EOS_ProductUserId LocalUserId;
    EOS_ProductUserId TargetUserId;
} EOS_CustomInvites_SendRequestToJoinCallbackInfo;
typedef void (EOS_CALL* EOS_CustomInvites_OnSendRequestToJoinCallback)(const EOS_CustomInvites_SendRequestToJoinCallbackInfo* Data);
#endif

// CustomInvites RequestToJoin notification types
#ifndef EOS_CUSTOMINVITES_ADDNOTIFYREQUESTTOJOINRECEIVED_API_LATEST
#define EOS_CUSTOMINVITES_ADDNOTIFYREQUESTTOJOINRECEIVED_API_LATEST 1
EOS_STRUCT(EOS_CustomInvites_AddNotifyRequestToJoinReceivedOptions, (int32_t ApiVersion;));
typedef void (EOS_CALL* EOS_CustomInvites_OnRequestToJoinReceivedCallback)(const void* Data);
EOS_STRUCT(EOS_CustomInvites_AddNotifyRequestToJoinAcceptedOptions, (int32_t ApiVersion;));
typedef void (EOS_CALL* EOS_CustomInvites_OnRequestToJoinAcceptedCallback)(const void* Data);
EOS_STRUCT(EOS_CustomInvites_AddNotifyRequestToJoinRejectedOptions, (int32_t ApiVersion;));
typedef void (EOS_CALL* EOS_CustomInvites_OnRequestToJoinRejectedCallback)(const void* Data);
#endif

// CustomInvites Accept/Reject RequestToJoin types
#ifndef EOS_CUSTOMINVITES_ACCEPTREQUESTTOJOIN_API_LATEST
#define EOS_CUSTOMINVITES_ACCEPTREQUESTTOJOIN_API_LATEST 1
EOS_STRUCT(EOS_CustomInvites_AcceptRequestToJoinOptions, (
    int32_t ApiVersion;
    EOS_ProductUserId LocalUserId;
    EOS_ProductUserId TargetUserId;
));
typedef struct EOS_CustomInvites_AcceptRequestToJoinCallbackInfoInternal {
    EOS_EResult ResultCode;
    void* ClientData;
    EOS_ProductUserId LocalUserId;
    EOS_ProductUserId TargetUserId;
} EOS_CustomInvites_AcceptRequestToJoinCallbackInfo;
typedef void (EOS_CALL* EOS_CustomInvites_OnAcceptRequestToJoinCallback)(const EOS_CustomInvites_AcceptRequestToJoinCallbackInfo* Data);
#endif

#ifndef EOS_CUSTOMINVITES_REJECTREQUESTTOJOIN_API_LATEST
#define EOS_CUSTOMINVITES_REJECTREQUESTTOJOIN_API_LATEST 1
EOS_STRUCT(EOS_CustomInvites_RejectRequestToJoinOptions, (
    int32_t ApiVersion;
    EOS_ProductUserId LocalUserId;
    EOS_ProductUserId TargetUserId;
));
typedef struct EOS_CustomInvites_RejectRequestToJoinCallbackInfoInternal {
    EOS_EResult ResultCode;
    void* ClientData;
    EOS_ProductUserId LocalUserId;
    EOS_ProductUserId TargetUserId;
} EOS_CustomInvites_RejectRequestToJoinCallbackInfo;
typedef void (EOS_CALL* EOS_CustomInvites_OnRejectRequestToJoinCallback)(const EOS_CustomInvites_RejectRequestToJoinCallbackInfo* Data);
#endif

// CustomInvites additional types
#ifndef EOS_CUSTOMINVITES_ADDNOTIFYREQUESTTOJOINRESPONSERECEIVEDOPTIONS_API_LATEST
#define EOS_CUSTOMINVITES_ADDNOTIFYREQUESTTOJOINRESPONSERECEIVEDOPTIONS_API_LATEST 1
EOS_STRUCT(EOS_CustomInvites_AddNotifyRequestToJoinResponseReceivedOptions, (int32_t ApiVersion;));
typedef void (EOS_CALL* EOS_CustomInvites_OnRequestToJoinResponseReceivedCallback)(const void* Data);
EOS_STRUCT(EOS_CustomInvites_AddNotifySendCustomNativeInviteRequestedOptions, (int32_t ApiVersion;));
typedef void (EOS_CALL* EOS_CustomInvites_OnSendCustomNativeInviteRequestedCallback)(const void* Data);
#endif

// Friends types
#ifndef EOS_FRIENDS_ADDNOTIFYBLOCKEDUSERSUPDATE_API_LATEST
#define EOS_FRIENDS_ADDNOTIFYBLOCKEDUSERSUPDATE_API_LATEST 1
EOS_STRUCT(EOS_Friends_AddNotifyBlockedUsersUpdateOptions, (int32_t ApiVersion;));
typedef void (EOS_CALL* EOS_Friends_OnBlockedUsersUpdateCallback)(const void* Data);
EOS_STRUCT(EOS_Friends_GetBlockedUsersCountOptions, (int32_t ApiVersion; EOS_EpicAccountId LocalUserId;));
EOS_STRUCT(EOS_Friends_GetBlockedUserAtIndexOptions, (int32_t ApiVersion; EOS_EpicAccountId LocalUserId; int32_t Index;));
#endif

// Ecom QueryEntitlementToken types
#ifndef EOS_ECOM_QUERYENTITLEMENTTOKEN_API_LATEST
#define EOS_ECOM_QUERYENTITLEMENTTOKEN_API_LATEST 1
EOS_STRUCT(EOS_Ecom_QueryEntitlementTokenOptions, (
    int32_t ApiVersion;
    EOS_EpicAccountId LocalUserId;
    const EOS_Ecom_EntitlementId* EntitlementIds;
    uint32_t EntitlementIdCount;
));
typedef struct EOS_Ecom_QueryEntitlementTokenCallbackInfoInternal {
    EOS_EResult ResultCode;
    void* ClientData;
    EOS_EpicAccountId LocalUserId;
    const char* EntitlementToken;
} EOS_Ecom_QueryEntitlementTokenCallbackInfo;
typedef void (EOS_CALL* EOS_Ecom_OnQueryEntitlementTokenCallback)(const EOS_Ecom_QueryEntitlementTokenCallbackInfo* Data);
#endif

// Ecom types
#ifndef EOS_ECOM_QUERYOWNERSHIPBYSANDBOXIDS_API_LATEST
#define EOS_ECOM_QUERYOWNERSHIPBYSANDBOXIDS_API_LATEST 1
EOS_STRUCT(EOS_Ecom_QueryOwnershipBySandboxIdsOptions, (
    int32_t ApiVersion;
    EOS_EpicAccountId LocalUserId;
    const char** SandboxIds;
    uint32_t SandboxIdsCount;
));
typedef struct EOS_Ecom_QueryOwnershipBySandboxIdsCallbackInfoInternal {
    EOS_EResult ResultCode;
    void* ClientData;
    EOS_EpicAccountId LocalUserId;
} EOS_Ecom_QueryOwnershipBySandboxIdsCallbackInfo;
typedef void (EOS_CALL* EOS_Ecom_OnQueryOwnershipBySandboxIdsCallback)(const EOS_Ecom_QueryOwnershipBySandboxIdsCallbackInfo* Data);
#endif

#ifndef EOS_ECOM_GETLASTREDEEMENTITLEMENTSCOUNT_API_LATEST
#define EOS_ECOM_GETLASTREDEEMENTITLEMENTSCOUNT_API_LATEST 1
EOS_STRUCT(EOS_Ecom_GetLastRedeemedEntitlementsCountOptions, (
    int32_t ApiVersion;
    EOS_EpicAccountId LocalUserId;
));
#endif

#ifndef EOS_ECOM_COPYLASTREDEEMENTITLEMENTBYINDEX_API_LATEST
#define EOS_ECOM_COPYLASTREDEEMENTITLEMENTBYINDEX_API_LATEST 1
EOS_STRUCT(EOS_Ecom_CopyLastRedeemedEntitlementByIndexOptions, (
    int32_t ApiVersion;
    EOS_EpicAccountId LocalUserId;
    uint32_t RedeemedEntitlementIndex;
));
#endif

// ============================================================================
// Auth Additional Stubs
// ============================================================================

EOS_DECLARE_FUNC(void) EOS_Auth_QueryIdToken(EOS_HAuth Handle, const EOS_Auth_QueryIdTokenOptions* Options, void* ClientData, const EOS_Auth_OnQueryIdTokenCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_Auth_QueryIdTokenCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Auth_CopyIdToken(EOS_HAuth Handle, const EOS_Auth_CopyIdTokenOptions* Options, EOS_Auth_IdToken** OutIdToken)
{
    return EOS_EResult::EOS_NotFound;
}

EOS_DECLARE_FUNC(void) EOS_Auth_IdToken_Release(EOS_Auth_IdToken* IdToken)
{
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Auth_VerifyIdToken(EOS_HAuth Handle, const EOS_Auth_VerifyIdTokenOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(uint32_t) EOS_Auth_GetMergedAccountsCount(EOS_HAuth Handle, EOS_EpicAccountId LocalUserId)
{
    return 0;
}

EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_Auth_GetMergedAccountByIndex(EOS_HAuth Handle, EOS_EpicAccountId LocalUserId, uint32_t Index)
{
    return nullptr;
}

EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_Auth_GetSelectedAccountId(EOS_HAuth Handle, EOS_EpicAccountId LocalUserId)
{
    return LocalUserId;
}

// ============================================================================
// Connect Additional Stubs
// ============================================================================

EOS_DECLARE_FUNC(EOS_EResult) EOS_Connect_CopyIdToken(EOS_HConnect Handle, const EOS_Connect_CopyIdTokenOptions* Options, EOS_Connect_IdToken** OutIdToken)
{
    return EOS_EResult::EOS_NotFound;
}

EOS_DECLARE_FUNC(void) EOS_Connect_IdToken_Release(EOS_Connect_IdToken* IdToken)
{
}

// EOS_Connect_VerifyIdToken is already declared in SDK with different signature (void with callback)

EOS_DECLARE_FUNC(void) EOS_Connect_Logout(EOS_HConnect Handle, const EOS_Connect_LogoutOptions* Options, void* ClientData, const EOS_Connect_OnLogoutCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_Connect_LogoutCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

// ============================================================================
// CustomInvites Stubs
// ============================================================================

EOS_DECLARE_FUNC(EOS_EResult) EOS_CustomInvites_SetCustomInvite(EOS_HCustomInvites Handle, const EOS_CustomInvites_SetCustomInviteOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_SendCustomInvite(EOS_HCustomInvites Handle, const EOS_CustomInvites_SendCustomInviteOptions* Options, void* ClientData, const EOS_CustomInvites_OnSendCustomInviteCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_CustomInvites_SendCustomInviteCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifyCustomInviteReceived(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifyCustomInviteReceivedOptions* Options, void* ClientData, const EOS_CustomInvites_OnCustomInviteReceivedCallback NotificationFn)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifyCustomInviteReceived(EOS_HCustomInvites Handle, EOS_NotificationId InId)
{
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifyCustomInviteAccepted(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifyCustomInviteAcceptedOptions* Options, void* ClientData, const EOS_CustomInvites_OnCustomInviteAcceptedCallback NotificationFn)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifyCustomInviteAccepted(EOS_HCustomInvites Handle, EOS_NotificationId InId)
{
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifyCustomInviteRejected(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifyCustomInviteRejectedOptions* Options, void* ClientData, const EOS_CustomInvites_OnCustomInviteRejectedCallback NotificationFn)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifyCustomInviteRejected(EOS_HCustomInvites Handle, EOS_NotificationId InId)
{
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_FinalizeInvite(EOS_HCustomInvites Handle, const EOS_CustomInvites_FinalizeInviteOptions* Options)
{
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_SendRequestToJoin(EOS_HCustomInvites Handle, const EOS_CustomInvites_SendRequestToJoinOptions* Options, void* ClientData, const EOS_CustomInvites_OnSendRequestToJoinCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_CustomInvites_SendRequestToJoinCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifyRequestToJoinReceived(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifyRequestToJoinReceivedOptions* Options, void* ClientData, const EOS_CustomInvites_OnRequestToJoinReceivedCallback NotificationFn)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifyRequestToJoinReceived(EOS_HCustomInvites Handle, EOS_NotificationId InId)
{
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifyRequestToJoinAccepted(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifyRequestToJoinAcceptedOptions* Options, void* ClientData, const EOS_CustomInvites_OnRequestToJoinAcceptedCallback NotificationFn)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifyRequestToJoinAccepted(EOS_HCustomInvites Handle, EOS_NotificationId InId)
{
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifyRequestToJoinRejected(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifyRequestToJoinRejectedOptions* Options, void* ClientData, const EOS_CustomInvites_OnRequestToJoinRejectedCallback NotificationFn)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifyRequestToJoinRejected(EOS_HCustomInvites Handle, EOS_NotificationId InId)
{
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_AcceptRequestToJoin(EOS_HCustomInvites Handle, const EOS_CustomInvites_AcceptRequestToJoinOptions* Options, void* ClientData, const EOS_CustomInvites_OnAcceptRequestToJoinCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_CustomInvites_AcceptRequestToJoinCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RejectRequestToJoin(EOS_HCustomInvites Handle, const EOS_CustomInvites_RejectRequestToJoinOptions* Options, void* ClientData, const EOS_CustomInvites_OnRejectRequestToJoinCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_CustomInvites_RejectRequestToJoinCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifyRequestToJoinResponseReceived(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifyRequestToJoinResponseReceivedOptions* Options, void* ClientData, const EOS_CustomInvites_OnRequestToJoinResponseReceivedCallback NotificationFn)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifyRequestToJoinResponseReceived(EOS_HCustomInvites Handle, EOS_NotificationId InId)
{
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_CustomInvites_AddNotifySendCustomNativeInviteRequested(EOS_HCustomInvites Handle, const EOS_CustomInvites_AddNotifySendCustomNativeInviteRequestedOptions* Options, void* ClientData, const EOS_CustomInvites_OnSendCustomNativeInviteRequestedCallback NotificationFn)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_CustomInvites_RemoveNotifySendCustomNativeInviteRequested(EOS_HCustomInvites Handle, EOS_NotificationId InId)
{
}

// ============================================================================
// KWS Stubs
// ============================================================================

EOS_DECLARE_FUNC(void) EOS_KWS_QueryAgeGate(EOS_HKWS Handle, const EOS_KWS_QueryAgeGateOptions* Options, void* ClientData, const EOS_KWS_OnQueryAgeGateCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_KWS_QueryAgeGateCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_KWS_CreateUser(EOS_HKWS Handle, const EOS_KWS_CreateUserOptions* Options, void* ClientData, const EOS_KWS_OnCreateUserCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_KWS_CreateUserCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(int32_t) EOS_KWS_GetPermissionsCount(EOS_HKWS Handle, const EOS_KWS_GetPermissionsCountOptions* Options)
{
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_KWS_CopyPermissionByIndex(EOS_HKWS Handle, const EOS_KWS_CopyPermissionByIndexOptions* Options, EOS_KWS_PermissionStatus** OutPermission)
{
    return EOS_EResult::EOS_NotFound;
}

EOS_DECLARE_FUNC(EOS_EKWSPermissionStatus) EOS_KWS_GetPermissionByKey(EOS_HKWS Handle, const EOS_KWS_GetPermissionByKeyOptions* Options)
{
    return EOS_EKWSPermissionStatus::EOS_KPS_GRANTED;
}

EOS_DECLARE_FUNC(void) EOS_KWS_PermissionStatus_Release(EOS_KWS_PermissionStatus* PermissionStatus)
{
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_KWS_AddNotifyPermissionsUpdateReceived(EOS_HKWS Handle, const EOS_KWS_AddNotifyPermissionsUpdateReceivedOptions* Options, void* ClientData, const EOS_KWS_OnPermissionsUpdateReceivedCallback NotificationFn)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_KWS_RemoveNotifyPermissionsUpdateReceived(EOS_HKWS Handle, EOS_NotificationId InId)
{
}

EOS_DECLARE_FUNC(void) EOS_KWS_QueryPermissions(EOS_HKWS Handle, const EOS_KWS_QueryPermissionsOptions* Options, void* ClientData, const EOS_KWS_OnQueryPermissionsCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_KWS_QueryPermissionsCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_KWS_RequestPermissions(EOS_HKWS Handle, const EOS_KWS_RequestPermissionsOptions* Options, void* ClientData, const EOS_KWS_OnRequestPermissionsCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_KWS_RequestPermissionsCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_KWS_UpdateParentEmail(EOS_HKWS Handle, const EOS_KWS_UpdateParentEmailOptions* Options, void* ClientData, const EOS_KWS_OnUpdateParentEmailCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_KWS_UpdateParentEmailCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

// ============================================================================
// Friends Additional Stubs
// ============================================================================

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Friends_AddNotifyBlockedUsersUpdate(EOS_HFriends Handle, const EOS_Friends_AddNotifyBlockedUsersUpdateOptions* Options, void* ClientData, const EOS_Friends_OnBlockedUsersUpdateCallback NotificationFn)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_Friends_RemoveNotifyBlockedUsersUpdate(EOS_HFriends Handle, EOS_NotificationId InId)
{
}

EOS_DECLARE_FUNC(int32_t) EOS_Friends_GetBlockedUsersCount(EOS_HFriends Handle, const EOS_Friends_GetBlockedUsersCountOptions* Options)
{
    return 0;
}

EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_Friends_GetBlockedUserAtIndex(EOS_HFriends Handle, const EOS_Friends_GetBlockedUserAtIndexOptions* Options)
{
    return nullptr;
}

// ============================================================================
// Ecom Additional Stubs
// ============================================================================

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryEntitlementToken(EOS_HEcom Handle, const EOS_Ecom_QueryEntitlementTokenOptions* Options, void* ClientData, const EOS_Ecom_OnQueryEntitlementTokenCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_Ecom_QueryEntitlementTokenCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOwnershipBySandboxIds(EOS_HEcom Handle, const EOS_Ecom_QueryOwnershipBySandboxIdsOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipBySandboxIdsCallback CompletionDelegate)
{
    if (CompletionDelegate)
    {
        EOS_Ecom_QueryOwnershipBySandboxIdsCallbackInfo info = {};
        info.ResultCode = EOS_EResult::EOS_Success;
        info.ClientData = ClientData;
        CompletionDelegate(&info);
    }
}

EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetLastRedeemedEntitlementsCount(EOS_HEcom Handle, const EOS_Ecom_GetLastRedeemedEntitlementsCountOptions* Options)
{
    return 0;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyLastRedeemedEntitlementByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyLastRedeemedEntitlementByIndexOptions* Options, char* OutRedeemedEntitlementId, int32_t* InOutRedeemedEntitlementIdLength)
{
    return EOS_EResult::EOS_NotFound;
}

// ============================================================================
// IntegratedPlatform Stubs
// ============================================================================

EOS_DECLARE_FUNC(EOS_EResult) EOS_IntegratedPlatform_CreateIntegratedPlatformOptionsContainer(const EOS_IntegratedPlatform_CreateIntegratedPlatformOptionsContainerOptions* Options, EOS_HIntegratedPlatformOptionsContainer* OutIntegratedPlatformOptionsContainer)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(void) EOS_IntegratedPlatformOptionsContainer_Release(EOS_HIntegratedPlatformOptionsContainer IntegratedPlatformOptionsContainer)
{
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_IntegratedPlatformOptionsContainer_Add(EOS_HIntegratedPlatformOptionsContainer Handle, const EOS_IntegratedPlatformOptionsContainer_AddOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_IntegratedPlatform_SetUserLoginStatus(EOS_HIntegratedPlatform Handle, const EOS_IntegratedPlatform_SetUserLoginStatusOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_IntegratedPlatform_AddNotifyUserLoginStatusChanged(EOS_HIntegratedPlatform Handle, const EOS_IntegratedPlatform_AddNotifyUserLoginStatusChangedOptions* Options, void* ClientData, const EOS_IntegratedPlatform_OnUserLoginStatusChangedCallback NotificationFn)
{
    return EOS_INVALID_NOTIFICATIONID;
}

EOS_DECLARE_FUNC(void) EOS_IntegratedPlatform_RemoveNotifyUserLoginStatusChanged(EOS_HIntegratedPlatform Handle, EOS_NotificationId InId)
{
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_IntegratedPlatform_SetUserPreLogoutCallback(EOS_HIntegratedPlatform Handle, const EOS_IntegratedPlatform_SetUserPreLogoutCallbackOptions* Options, void* ClientData, const EOS_IntegratedPlatform_OnUserPreLogoutCallback CallbackFn)
{
    return EOS_EResult::EOS_Success;
}

EOS_DECLARE_FUNC(void) EOS_IntegratedPlatform_ClearUserPreLogoutCallback(EOS_HIntegratedPlatform Handle, const EOS_IntegratedPlatform_ClearUserPreLogoutCallbackOptions* Options)
{
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_IntegratedPlatform_FinalizeDeferredUserLogout(EOS_HIntegratedPlatform Handle, const EOS_IntegratedPlatform_FinalizeDeferredUserLogoutOptions* Options)
{
    return EOS_EResult::EOS_Success;
}

// ============================================================================
// Misc Stubs
// ============================================================================

EOS_DECLARE_FUNC(void) EOS_BeginScopeEvent(const char* EventName)
{
}

EOS_DECLARE_FUNC(void) EOS_EndScopeEvent()
{
}

EOS_DECLARE_FUNC(const char*) EOS_ENetworkStatus_ToString(EOS_ENetworkStatus Status)
{
    return "Online";
}
