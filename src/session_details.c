#include "eos/eos_sessions.h"
#include "eos/eos_sessions_types.h"
#include "internal/sessions_internal.h"
#include <stdlib.h>
#include <string.h>

// Helper function to convert internal Session to EOS_SessionDetails_Info
static EOS_SessionDetails_Info* session_to_info(const Session* session) {
    if (!session || !session->valid) {
        return NULL;
    }

    EOS_SessionDetails_Info* info = calloc(1, sizeof(EOS_SessionDetails_Info));
    if (!info) {
        return NULL;
    }

    EOS_SessionDetails_Settings* settings = calloc(1, sizeof(EOS_SessionDetails_Settings));
    if (!settings) {
        free(info);
        return NULL;
    }

    // Allocate and copy session ID
    char* session_id = calloc(1, strlen(session->session_id) + 1);
    char* host_address = calloc(1, strlen(session->host_address) + 1);
    char* bucket_id = calloc(1, strlen(session->bucket_id) + 1);

    if (!session_id || !host_address || !bucket_id) {
        free(session_id);
        free(host_address);
        free(bucket_id);
        free(settings);
        free(info);
        return NULL;
    }

    strcpy(session_id, session->session_id);
    strcpy(host_address, session->host_address);
    strcpy(bucket_id, session->bucket_id);

    // Fill settings
    settings->ApiVersion = EOS_SESSIONDETAILS_SETTINGS_API_LATEST;
    settings->BucketId = bucket_id;
    settings->NumPublicConnections = session->max_players;
    settings->bAllowJoinInProgress = session->join_in_progress_allowed ? EOS_TRUE : EOS_FALSE;
    settings->PermissionLevel = session->permission_level;
    settings->bInvitesAllowed = session->invites_allowed ? EOS_TRUE : EOS_FALSE;
    settings->bSanctionsEnabled = session->sanctions_enabled ? EOS_TRUE : EOS_FALSE;
    settings->AllowedPlatformIds = NULL;
    settings->AllowedPlatformIdsCount = 0;

    // Fill info
    info->ApiVersion = EOS_SESSIONDETAILS_INFO_API_LATEST;
    info->SessionId = session_id;
    info->HostAddress = host_address;
    info->NumOpenPublicConnections = session->max_players - session->registered_player_count;
    info->Settings = settings;
    info->OwnerUserId = session->owner_id;
    info->OwnerServerClientId = NULL;

    return info;
}

// SessionDetails functions

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionDetails_CopyInfo(
    EOS_HSessionDetails Handle,
    const EOS_SessionDetails_CopyInfoOptions* Options,
    EOS_SessionDetails_Info** OutSessionInfo
) {
    SessionDetailsHandle* details = (SessionDetailsHandle*)Handle;

    if (!details || details->magic != 0x53445448) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONDETAILS_COPYINFO_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!OutSessionInfo) {
        return EOS_InvalidParameters;
    }

    EOS_SessionDetails_Info* info = session_to_info(&details->session);
    if (!info) {
        return EOS_LimitExceeded;
    }

    *OutSessionInfo = info;
    return EOS_Success;
}

EOS_DECLARE_FUNC(uint32_t) EOS_SessionDetails_GetSessionAttributeCount(
    EOS_HSessionDetails Handle,
    const EOS_SessionDetails_GetSessionAttributeCountOptions* Options
) {
    SessionDetailsHandle* details = (SessionDetailsHandle*)Handle;

    if (!details || details->magic != 0x53445448) {
        return 0;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONDETAILS_GETSESSIONATTRIBUTECOUNT_API_LATEST) {
        return 0;
    }

    return (uint32_t)details->session.attribute_count;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionDetails_CopySessionAttributeByIndex(
    EOS_HSessionDetails Handle,
    const EOS_SessionDetails_CopySessionAttributeByIndexOptions* Options,
    EOS_SessionDetails_Attribute** OutSessionAttribute
) {
    SessionDetailsHandle* details = (SessionDetailsHandle*)Handle;

    if (!details || details->magic != 0x53445448) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONDETAILS_COPYSESSIONATTRIBUTEBYINDEX_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!OutSessionAttribute) {
        return EOS_InvalidParameters;
    }

    if (Options->AttrIndex >= (uint32_t)details->session.attribute_count) {
        return EOS_NotFound;
    }

    const SessionAttribute* attr = &details->session.attributes[Options->AttrIndex];

    // Allocate structures
    EOS_SessionDetails_Attribute* out_attr = calloc(1, sizeof(EOS_SessionDetails_Attribute));
    EOS_Sessions_AttributeData* attr_data = calloc(1, sizeof(EOS_Sessions_AttributeData));
    char* key = calloc(1, strlen(attr->key) + 1);

    if (!out_attr || !attr_data || !key) {
        free(out_attr);
        free(attr_data);
        free(key);
        return EOS_LimitExceeded;
    }

    strcpy(key, attr->key);

    // Fill attribute data
    attr_data->ApiVersion = EOS_SESSIONS_ATTRIBUTEDATA_API_LATEST;
    attr_data->Key = key;
    attr_data->ValueType = attr->type;

    switch (attr->type) {
        case EOS_AT_BOOLEAN:
            attr_data->Value.AsBool = attr->value.as_bool;
            break;
        case EOS_AT_INT64:
            attr_data->Value.AsInt64 = attr->value.as_int64;
            break;
        case EOS_AT_DOUBLE:
            attr_data->Value.AsDouble = attr->value.as_double;
            break;
        case EOS_AT_STRING: {
            char* str_val = calloc(1, strlen(attr->value.as_string) + 1);
            if (!str_val) {
                free(out_attr);
                free(attr_data);
                free(key);
                return EOS_LimitExceeded;
            }
            strcpy(str_val, attr->value.as_string);
            attr_data->Value.AsUtf8 = str_val;
            break;
        }
    }

    // Fill output attribute
    out_attr->ApiVersion = EOS_SESSIONDETAILS_ATTRIBUTE_API_LATEST;
    out_attr->Data = attr_data;
    out_attr->AdvertisementType = attr->advertisement;

    *OutSessionAttribute = out_attr;
    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionDetails_CopySessionAttributeByKey(
    EOS_HSessionDetails Handle,
    const EOS_SessionDetails_CopySessionAttributeByKeyOptions* Options,
    EOS_SessionDetails_Attribute** OutSessionAttribute
) {
    SessionDetailsHandle* details = (SessionDetailsHandle*)Handle;

    if (!details || details->magic != 0x53445448) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONDETAILS_COPYSESSIONATTRIBUTEBYKEY_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!Options->AttrKey || !OutSessionAttribute) {
        return EOS_InvalidParameters;
    }

    // Find attribute by key
    for (int i = 0; i < details->session.attribute_count; i++) {
        if (strcmp(details->session.attributes[i].key, Options->AttrKey) == 0) {
            EOS_SessionDetails_CopySessionAttributeByIndexOptions index_opts = {
                .ApiVersion = EOS_SESSIONDETAILS_COPYSESSIONATTRIBUTEBYINDEX_API_LATEST,
                .AttrIndex = (uint32_t)i
            };
            return EOS_SessionDetails_CopySessionAttributeByIndex(Handle, &index_opts, OutSessionAttribute);
        }
    }

    return EOS_NotFound;
}

EOS_DECLARE_FUNC(void) EOS_SessionDetails_Release(EOS_HSessionDetails SessionHandle) {
    SessionDetailsHandle* details = (SessionDetailsHandle*)SessionHandle;

    if (details && details->magic == 0x53445448) {
        details->magic = 0;  // Invalidate
        free(details);
    }
}

EOS_DECLARE_FUNC(void) EOS_SessionDetails_Info_Release(EOS_SessionDetails_Info* SessionInfo) {
    if (!SessionInfo) {
        return;
    }

    // Free allocated strings
    if (SessionInfo->SessionId) {
        free((void*)SessionInfo->SessionId);
    }
    if (SessionInfo->HostAddress) {
        free((void*)SessionInfo->HostAddress);
    }
    if (SessionInfo->OwnerServerClientId) {
        free((void*)SessionInfo->OwnerServerClientId);
    }

    // Free settings
    if (SessionInfo->Settings) {
        if (SessionInfo->Settings->BucketId) {
            free((void*)SessionInfo->Settings->BucketId);
        }
        if (SessionInfo->Settings->AllowedPlatformIds) {
            free((void*)SessionInfo->Settings->AllowedPlatformIds);
        }
        free((void*)SessionInfo->Settings);
    }

    free(SessionInfo);
}

EOS_DECLARE_FUNC(void) EOS_SessionDetails_Attribute_Release(EOS_SessionDetails_Attribute* SessionAttribute) {
    if (!SessionAttribute) {
        return;
    }

    if (SessionAttribute->Data) {
        if (SessionAttribute->Data->Key) {
            free((void*)SessionAttribute->Data->Key);
        }
        if (SessionAttribute->Data->ValueType == EOS_AT_STRING && SessionAttribute->Data->Value.AsUtf8) {
            free((void*)SessionAttribute->Data->Value.AsUtf8);
        }
        free(SessionAttribute->Data);
    }

    free(SessionAttribute);
}

// ActiveSession functions

EOS_DECLARE_FUNC(EOS_EResult) EOS_ActiveSession_CopyInfo(
    EOS_HActiveSession Handle,
    const EOS_ActiveSession_CopyInfoOptions* Options,
    EOS_ActiveSession_Info** OutActiveSessionInfo
) {
    ActiveSessionHandle* active = (ActiveSessionHandle*)Handle;

    if (!active || active->magic != 0x41435448) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_ACTIVESESSION_COPYINFO_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!OutActiveSessionInfo) {
        return EOS_InvalidParameters;
    }

    if (!active->session || !active->session->valid) {
        return EOS_InvalidState;
    }

    // Allocate info structure
    EOS_ActiveSession_Info* info = calloc(1, sizeof(EOS_ActiveSession_Info));
    if (!info) {
        return EOS_LimitExceeded;
    }

    // Allocate session name copy
    char* session_name = calloc(1, strlen(active->session->session_name) + 1);
    if (!session_name) {
        free(info);
        return EOS_LimitExceeded;
    }
    strcpy(session_name, active->session->session_name);

    // Get session details
    EOS_SessionDetails_Info* session_details = session_to_info(active->session);
    if (!session_details) {
        free(session_name);
        free(info);
        return EOS_LimitExceeded;
    }

    info->ApiVersion = EOS_ACTIVESESSION_INFO_API_LATEST;
    info->SessionName = session_name;
    info->LocalUserId = active->session->owner_id;
    info->State = active->session->state;
    info->SessionDetails = session_details;

    *OutActiveSessionInfo = info;
    return EOS_Success;
}

EOS_DECLARE_FUNC(uint32_t) EOS_ActiveSession_GetRegisteredPlayerCount(
    EOS_HActiveSession Handle,
    const EOS_ActiveSession_GetRegisteredPlayerCountOptions* Options
) {
    ActiveSessionHandle* active = (ActiveSessionHandle*)Handle;

    if (!active || active->magic != 0x41435448) {
        return 0;
    }

    if (!Options || Options->ApiVersion != EOS_ACTIVESESSION_GETREGISTEREDPLAYERCOUNT_API_LATEST) {
        return 0;
    }

    if (!active->session) {
        return 0;
    }

    return (uint32_t)active->session->registered_player_count;
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_ActiveSession_GetRegisteredPlayerByIndex(
    EOS_HActiveSession Handle,
    const EOS_ActiveSession_GetRegisteredPlayerByIndexOptions* Options
) {
    ActiveSessionHandle* active = (ActiveSessionHandle*)Handle;

    if (!active || active->magic != 0x41435448) {
        return NULL;
    }

    if (!Options || Options->ApiVersion != EOS_ACTIVESESSION_GETREGISTEREDPLAYERBYINDEX_API_LATEST) {
        return NULL;
    }

    if (!active->session) {
        return NULL;
    }

    if (Options->PlayerIndex >= (uint32_t)active->session->registered_player_count) {
        return NULL;
    }

    return active->session->registered_players[Options->PlayerIndex];
}

EOS_DECLARE_FUNC(void) EOS_ActiveSession_Release(EOS_HActiveSession Handle) {
    ActiveSessionHandle* active = (ActiveSessionHandle*)Handle;

    if (active && active->magic == 0x41435448) {
        active->magic = 0;  // Invalidate
        free(active);
    }
}

EOS_DECLARE_FUNC(void) EOS_ActiveSession_Info_Release(EOS_ActiveSession_Info* ActiveSessionInfo) {
    if (!ActiveSessionInfo) {
        return;
    }

    if (ActiveSessionInfo->SessionName) {
        free((void*)ActiveSessionInfo->SessionName);
    }

    if (ActiveSessionInfo->SessionDetails) {
        EOS_SessionDetails_Info_Release((EOS_SessionDetails_Info*)ActiveSessionInfo->SessionDetails);
    }

    free(ActiveSessionInfo);
}
