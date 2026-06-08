#include "eos/eos_lobby.h"
#include "eos/eos_lobby_types.h"
#include "internal/lobby_internal.h"
#include "internal/logging.h"
#include <stdlib.h>
#include <string.h>

#define LOBBY_DETAILS_MAGIC 0x4C445448  // "LDTH"

// Helper: convert an internal Lobby into a heap-allocated EOS_LobbyDetails_Info.
// The game must release the result via EOS_LobbyDetails_Info_Release.
static EOS_LobbyDetails_Info* lobby_to_info(const Lobby* lobby) {
    if (!lobby || !lobby->valid) {
        return NULL;
    }

    EOS_LobbyDetails_Info* info = calloc(1, sizeof(EOS_LobbyDetails_Info));
    if (!info) {
        return NULL;
    }

    // Allocate and copy the strings the game expects to own until Release.
    char* lobby_id = calloc(1, strlen(lobby->lobby_id) + 1);
    char* bucket_id = calloc(1, strlen(lobby->bucket_id) + 1);

    if (!lobby_id || !bucket_id) {
        free(lobby_id);
        free(bucket_id);
        free(info);
        return NULL;
    }

    strcpy(lobby_id, lobby->lobby_id);
    strcpy(bucket_id, lobby->bucket_id);

    // AvailableSlots = max_members - member_count, clamped to >= 0.
    uint32_t available_slots = 0;
    if ((uint32_t)lobby->member_count < lobby->max_members) {
        available_slots = lobby->max_members - (uint32_t)lobby->member_count;
    }

    info->ApiVersion = EOS_LOBBYDETAILS_INFO_API_LATEST;
    info->LobbyId = lobby_id;
    info->LobbyOwnerUserId = lobby->owner_id;
    info->PermissionLevel = lobby->permission_level;
    info->AvailableSlots = available_slots;
    info->MaxMembers = lobby->max_members;
    info->bAllowInvites = lobby->allow_invites ? EOS_TRUE : EOS_FALSE;
    info->BucketId = bucket_id;
    info->bAllowHostMigration = lobby->allow_host_migration ? EOS_TRUE : EOS_FALSE;
    info->bRTCRoomEnabled = lobby->rtc_room_enabled ? EOS_TRUE : EOS_FALSE;
    info->bAllowJoinById = lobby->allow_join_by_id ? EOS_TRUE : EOS_FALSE;
    info->bRejoinAfterKickRequiresInvite = EOS_FALSE;
    info->bPresenceEnabled = lobby->presence_enabled ? EOS_TRUE : EOS_FALSE;
    info->AllowedPlatformIds = NULL;
    info->AllowedPlatformIdsCount = 0;

    return info;
}

// Helper: build a heap-allocated EOS_Lobby_Attribute from an internal LobbyAttribute.
// Mirrors session_details.c's copied-attribute discipline: the EOS_Lobby_Attribute,
// its EOS_Lobby_AttributeData, the duplicated Key, and (for strings) the duplicated
// value are all malloc'd and freed by EOS_Lobby_Attribute_Release.
static EOS_EResult lobby_attribute_to_eos(const LobbyAttribute* attr, EOS_Lobby_Attribute** out) {
    EOS_Lobby_Attribute* out_attr = calloc(1, sizeof(EOS_Lobby_Attribute));
    EOS_Lobby_AttributeData* attr_data = calloc(1, sizeof(EOS_Lobby_AttributeData));
    char* key = calloc(1, strlen(attr->key) + 1);

    if (!out_attr || !attr_data || !key) {
        free(out_attr);
        free(attr_data);
        free(key);
        return EOS_LimitExceeded;
    }

    strcpy(key, attr->key);

    attr_data->ApiVersion = EOS_LOBBY_ATTRIBUTEDATA_API_LATEST;
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

    out_attr->ApiVersion = EOS_LOBBY_ATTRIBUTE_API_LATEST;
    out_attr->Data = attr_data;
    out_attr->Visibility = attr->visibility;

    *out = out_attr;
    return EOS_Success;
}

// Helper: locate a member of the snapshot lobby by Product User ID.
static const LobbyMember* find_member(const Lobby* lobby, EOS_ProductUserId user) {
    for (int i = 0; i < lobby->member_count; i++) {
        if (lobby->members[i].valid && lobby->members[i].member_id == user) {
            return &lobby->members[i];
        }
    }
    return NULL;
}

// LobbyDetails functions

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_LobbyDetails_GetLobbyOwner(
    EOS_HLobbyDetails Handle,
    const EOS_LobbyDetails_GetLobbyOwnerOptions* Options
) {
    LobbyDetailsHandle* details = (LobbyDetailsHandle*)Handle;

    if (!details || details->magic != LOBBY_DETAILS_MAGIC) {
        return NULL;
    }

    if (!Options) {
        return NULL;
    }

    return details->lobby.owner_id;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyInfo(
    EOS_HLobbyDetails Handle,
    const EOS_LobbyDetails_CopyInfoOptions* Options,
    EOS_LobbyDetails_Info** OutLobbyDetailsInfo
) {
    LobbyDetailsHandle* details = (LobbyDetailsHandle*)Handle;

    if (!details || details->magic != LOBBY_DETAILS_MAGIC) {
        return EOS_InvalidParameters;
    }

    if (!Options) {
        return EOS_InvalidParameters;
    }

    if (!OutLobbyDetailsInfo) {
        return EOS_InvalidParameters;
    }

    EOS_LobbyDetails_Info* info = lobby_to_info(&details->lobby);
    if (!info) {
        return EOS_LimitExceeded;
    }

    *OutLobbyDetailsInfo = info;
    return EOS_Success;
}

EOS_DECLARE_FUNC(uint32_t) EOS_LobbyDetails_GetAttributeCount(
    EOS_HLobbyDetails Handle,
    const EOS_LobbyDetails_GetAttributeCountOptions* Options
) {
    LobbyDetailsHandle* details = (LobbyDetailsHandle*)Handle;

    if (!details || details->magic != LOBBY_DETAILS_MAGIC) {
        return 0;
    }

    if (!Options) {
        return 0;
    }

    return (uint32_t)details->lobby.attribute_count;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyAttributeByIndex(
    EOS_HLobbyDetails Handle,
    const EOS_LobbyDetails_CopyAttributeByIndexOptions* Options,
    EOS_Lobby_Attribute** OutAttribute
) {
    LobbyDetailsHandle* details = (LobbyDetailsHandle*)Handle;

    if (!details || details->magic != LOBBY_DETAILS_MAGIC) {
        return EOS_InvalidParameters;
    }

    if (!Options) {
        return EOS_InvalidParameters;
    }

    if (!OutAttribute) {
        return EOS_InvalidParameters;
    }

    if (Options->AttrIndex >= (uint32_t)details->lobby.attribute_count) {
        return EOS_NotFound;
    }

    const LobbyAttribute* attr = &details->lobby.attributes[Options->AttrIndex];
    return lobby_attribute_to_eos(attr, OutAttribute);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyAttributeByKey(
    EOS_HLobbyDetails Handle,
    const EOS_LobbyDetails_CopyAttributeByKeyOptions* Options,
    EOS_Lobby_Attribute** OutAttribute
) {
    LobbyDetailsHandle* details = (LobbyDetailsHandle*)Handle;

    if (!details || details->magic != LOBBY_DETAILS_MAGIC) {
        return EOS_InvalidParameters;
    }

    if (!Options) {
        return EOS_InvalidParameters;
    }

    if (!Options->AttrKey || !OutAttribute) {
        return EOS_InvalidParameters;
    }

    for (int i = 0; i < details->lobby.attribute_count; i++) {
        if (strcmp(details->lobby.attributes[i].key, Options->AttrKey) == 0) {
            return lobby_attribute_to_eos(&details->lobby.attributes[i], OutAttribute);
        }
    }

    return EOS_NotFound;
}

EOS_DECLARE_FUNC(uint32_t) EOS_LobbyDetails_GetMemberCount(
    EOS_HLobbyDetails Handle,
    const EOS_LobbyDetails_GetMemberCountOptions* Options
) {
    LobbyDetailsHandle* details = (LobbyDetailsHandle*)Handle;

    if (!details || details->magic != LOBBY_DETAILS_MAGIC) {
        return 0;
    }

    if (!Options) {
        return 0;
    }

    return (uint32_t)details->lobby.member_count;
}

EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_LobbyDetails_GetMemberByIndex(
    EOS_HLobbyDetails Handle,
    const EOS_LobbyDetails_GetMemberByIndexOptions* Options
) {
    LobbyDetailsHandle* details = (LobbyDetailsHandle*)Handle;

    if (!details || details->magic != LOBBY_DETAILS_MAGIC) {
        return NULL;
    }

    if (!Options) {
        return NULL;
    }

    if (Options->MemberIndex >= (uint32_t)details->lobby.member_count) {
        return NULL;
    }

    EOS_ProductUserId m = details->lobby.members[Options->MemberIndex].member_id;
    EOS_LOG_DEBUG("LobbyDetails_GetMemberByIndex(%u) -> %s (member_count=%d)",
                 Options->MemberIndex, m ? "valid" : "NULL", details->lobby.member_count);
    return m;
}

EOS_DECLARE_FUNC(uint32_t) EOS_LobbyDetails_GetMemberAttributeCount(
    EOS_HLobbyDetails Handle,
    const EOS_LobbyDetails_GetMemberAttributeCountOptions* Options
) {
    LobbyDetailsHandle* details = (LobbyDetailsHandle*)Handle;

    if (!details || details->magic != LOBBY_DETAILS_MAGIC) {
        return 0;
    }

    if (!Options) {
        return 0;
    }

    const LobbyMember* member = find_member(&details->lobby, Options->TargetUserId);
    if (!member) {
        return 0;
    }

    return (uint32_t)member->attribute_count;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyMemberAttributeByIndex(
    EOS_HLobbyDetails Handle,
    const EOS_LobbyDetails_CopyMemberAttributeByIndexOptions* Options,
    EOS_Lobby_Attribute** OutAttribute
) {
    LobbyDetailsHandle* details = (LobbyDetailsHandle*)Handle;

    if (!details || details->magic != LOBBY_DETAILS_MAGIC) {
        return EOS_InvalidParameters;
    }

    if (!Options) {
        return EOS_InvalidParameters;
    }

    if (!OutAttribute) {
        return EOS_InvalidParameters;
    }

    const LobbyMember* member = find_member(&details->lobby, Options->TargetUserId);
    if (!member) {
        return EOS_NotFound;
    }

    if (Options->AttrIndex >= (uint32_t)member->attribute_count) {
        return EOS_NotFound;
    }

    return lobby_attribute_to_eos(&member->attributes[Options->AttrIndex], OutAttribute);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyDetails_CopyMemberAttributeByKey(
    EOS_HLobbyDetails Handle,
    const EOS_LobbyDetails_CopyMemberAttributeByKeyOptions* Options,
    EOS_Lobby_Attribute** OutAttribute
) {
    LobbyDetailsHandle* details = (LobbyDetailsHandle*)Handle;

    if (!details || details->magic != LOBBY_DETAILS_MAGIC) {
        return EOS_InvalidParameters;
    }

    if (!Options) {
        return EOS_InvalidParameters;
    }

    if (!Options->AttrKey || !OutAttribute) {
        return EOS_InvalidParameters;
    }

    const LobbyMember* member = find_member(&details->lobby, Options->TargetUserId);
    if (!member) {
        return EOS_NotFound;
    }

    for (int i = 0; i < member->attribute_count; i++) {
        if (strcmp(member->attributes[i].key, Options->AttrKey) == 0) {
            return lobby_attribute_to_eos(&member->attributes[i], OutAttribute);
        }
    }

    return EOS_NotFound;
}

EOS_DECLARE_FUNC(void) EOS_LobbyDetails_Release(EOS_HLobbyDetails LobbyHandle) {
    LobbyDetailsHandle* details = (LobbyDetailsHandle*)LobbyHandle;

    if (details && details->magic == LOBBY_DETAILS_MAGIC) {
        details->magic = 0;  // Invalidate
        free(details);
    }
}

EOS_DECLARE_FUNC(void) EOS_LobbyDetails_Info_Release(EOS_LobbyDetails_Info* LobbyDetailsInfo) {
    if (!LobbyDetailsInfo) {
        return;
    }

    if (LobbyDetailsInfo->LobbyId) {
        free((void*)LobbyDetailsInfo->LobbyId);
    }
    if (LobbyDetailsInfo->BucketId) {
        free((void*)LobbyDetailsInfo->BucketId);
    }
    if (LobbyDetailsInfo->AllowedPlatformIds) {
        free((void*)LobbyDetailsInfo->AllowedPlatformIds);
    }

    free(LobbyDetailsInfo);
}

EOS_DECLARE_FUNC(void) EOS_Lobby_Attribute_Release(EOS_Lobby_Attribute* LobbyAttribute) {
    if (!LobbyAttribute) {
        return;
    }

    if (LobbyAttribute->Data) {
        if (LobbyAttribute->Data->Key) {
            free((void*)LobbyAttribute->Data->Key);
        }
        if (LobbyAttribute->Data->ValueType == EOS_AT_STRING && LobbyAttribute->Data->Value.AsUtf8) {
            free((void*)LobbyAttribute->Data->Value.AsUtf8);
        }
        free(LobbyAttribute->Data);
    }

    free(LobbyAttribute);
}
