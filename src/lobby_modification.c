#include "eos/eos_lobby.h"
#include "eos/eos_lobby_types.h"
#include "internal/lobby_internal.h"
#include <stdlib.h>
#include <string.h>

// LobbyModification handle functions
//
// These are the staged-edit setters consumed by CreateLobby/UpdateLobby.
// They mutate the handle's `staged` Lobby (lobby-level fields + attributes[])
// or the handle's member_attributes[] (staged local-member attributes).
//
// Magic: 0x4C4D4F44 == "LMOD"

// Copy an EOS_Lobby_AttributeData (+ visibility) into a LobbyAttribute.
// Returns EOS_Success or EOS_InvalidParameters on an unknown value type.
static EOS_EResult lobby_copy_attribute_data(
    LobbyAttribute* attr,
    const EOS_Lobby_AttributeData* attr_data,
    EOS_ELobbyAttributeVisibility visibility
) {
    strncpy(attr->key, attr_data->Key, sizeof(attr->key) - 1);
    attr->key[sizeof(attr->key) - 1] = '\0';
    attr->type = attr_data->ValueType;
    attr->visibility = visibility;

    switch (attr_data->ValueType) {
        case EOS_AT_BOOLEAN:
            attr->value.as_bool = attr_data->Value.AsBool;
            break;
        case EOS_AT_INT64:
            attr->value.as_int64 = attr_data->Value.AsInt64;
            break;
        case EOS_AT_DOUBLE:
            attr->value.as_double = attr_data->Value.AsDouble;
            break;
        case EOS_AT_STRING:
            if (attr_data->Value.AsUtf8) {
                strncpy(attr->value.as_string, attr_data->Value.AsUtf8, sizeof(attr->value.as_string) - 1);
                attr->value.as_string[sizeof(attr->value.as_string) - 1] = '\0';
            } else {
                attr->value.as_string[0] = '\0';
            }
            break;
        default:
            return EOS_InvalidParameters;
    }

    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_SetBucketId(
    EOS_HLobbyModification Handle,
    const EOS_LobbyModification_SetBucketIdOptions* Options
) {
    LobbyModificationHandle* mod = (LobbyModificationHandle*)Handle;

    if (!mod || mod->magic != 0x4C4D4F44) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_LOBBYMODIFICATION_SETBUCKETID_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!Options->BucketId) {
        return EOS_InvalidParameters;
    }

    strncpy(mod->staged.bucket_id, Options->BucketId, sizeof(mod->staged.bucket_id) - 1);
    mod->staged.bucket_id[sizeof(mod->staged.bucket_id) - 1] = '\0';

    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_SetPermissionLevel(
    EOS_HLobbyModification Handle,
    const EOS_LobbyModification_SetPermissionLevelOptions* Options
) {
    LobbyModificationHandle* mod = (LobbyModificationHandle*)Handle;

    if (!mod || mod->magic != 0x4C4D4F44) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_LOBBYMODIFICATION_SETPERMISSIONLEVEL_API_LATEST) {
        return EOS_InvalidParameters;
    }

    mod->staged.permission_level = Options->PermissionLevel;

    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_SetMaxMembers(
    EOS_HLobbyModification Handle,
    const EOS_LobbyModification_SetMaxMembersOptions* Options
) {
    LobbyModificationHandle* mod = (LobbyModificationHandle*)Handle;

    if (!mod || mod->magic != 0x4C4D4F44) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_LOBBYMODIFICATION_SETMAXMEMBERS_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (Options->MaxMembers == 0 || Options->MaxMembers > MAX_LOBBY_MEMBERS) {
        return EOS_InvalidParameters;
    }

    mod->staged.max_members = Options->MaxMembers;

    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_SetInvitesAllowed(
    EOS_HLobbyModification Handle,
    const EOS_LobbyModification_SetInvitesAllowedOptions* Options
) {
    LobbyModificationHandle* mod = (LobbyModificationHandle*)Handle;

    if (!mod || mod->magic != 0x4C4D4F44) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_LOBBYMODIFICATION_SETINVITESALLOWED_API_LATEST) {
        return EOS_InvalidParameters;
    }

    mod->staged.allow_invites = (Options->bInvitesAllowed == EOS_TRUE);

    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_AddAttribute(
    EOS_HLobbyModification Handle,
    const EOS_LobbyModification_AddAttributeOptions* Options
) {
    LobbyModificationHandle* mod = (LobbyModificationHandle*)Handle;

    if (!mod || mod->magic != 0x4C4D4F44) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_LOBBYMODIFICATION_ADDATTRIBUTE_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!Options->Attribute || !Options->Attribute->Key) {
        return EOS_InvalidParameters;
    }

    const EOS_Lobby_AttributeData* attr_data = Options->Attribute;

    // Check if attribute already exists and update it
    int existing_index = -1;
    for (int i = 0; i < mod->staged.attribute_count; i++) {
        if (strcmp(mod->staged.attributes[i].key, attr_data->Key) == 0) {
            existing_index = i;
            break;
        }
    }

    LobbyAttribute* attr;
    if (existing_index >= 0) {
        // Update existing attribute (dedupe by key)
        attr = &mod->staged.attributes[existing_index];
    } else {
        // Add new attribute
        if (mod->staged.attribute_count >= MAX_LOBBY_ATTRIBUTES) {
            return EOS_LimitExceeded;
        }
        attr = &mod->staged.attributes[mod->staged.attribute_count++];
    }

    return lobby_copy_attribute_data(attr, attr_data, Options->Visibility);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_RemoveAttribute(
    EOS_HLobbyModification Handle,
    const EOS_LobbyModification_RemoveAttributeOptions* Options
) {
    LobbyModificationHandle* mod = (LobbyModificationHandle*)Handle;

    if (!mod || mod->magic != 0x4C4D4F44) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_LOBBYMODIFICATION_REMOVEATTRIBUTE_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!Options->Key) {
        return EOS_InvalidParameters;
    }

    // Find and remove attribute
    for (int i = 0; i < mod->staged.attribute_count; i++) {
        if (strcmp(mod->staged.attributes[i].key, Options->Key) == 0) {
            // Shift remaining attributes down
            memmove(&mod->staged.attributes[i],
                    &mod->staged.attributes[i + 1],
                    sizeof(LobbyAttribute) * (mod->staged.attribute_count - i - 1));
            mod->staged.attribute_count--;
            return EOS_Success;
        }
    }

    return EOS_NotFound;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_AddMemberAttribute(
    EOS_HLobbyModification Handle,
    const EOS_LobbyModification_AddMemberAttributeOptions* Options
) {
    LobbyModificationHandle* mod = (LobbyModificationHandle*)Handle;

    if (!mod || mod->magic != 0x4C4D4F44) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_LOBBYMODIFICATION_ADDMEMBERATTRIBUTE_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!Options->Attribute || !Options->Attribute->Key) {
        return EOS_InvalidParameters;
    }

    const EOS_Lobby_AttributeData* attr_data = Options->Attribute;

    // Check if member attribute already exists and update it
    int existing_index = -1;
    for (int i = 0; i < mod->member_attribute_count; i++) {
        if (strcmp(mod->member_attributes[i].key, attr_data->Key) == 0) {
            existing_index = i;
            break;
        }
    }

    LobbyAttribute* attr;
    if (existing_index >= 0) {
        // Update existing member attribute (dedupe by key)
        attr = &mod->member_attributes[existing_index];
    } else {
        // Add new member attribute
        if (mod->member_attribute_count >= MAX_MEMBER_ATTRIBUTES) {
            return EOS_LimitExceeded;
        }
        attr = &mod->member_attributes[mod->member_attribute_count++];
    }

    return lobby_copy_attribute_data(attr, attr_data, Options->Visibility);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbyModification_RemoveMemberAttribute(
    EOS_HLobbyModification Handle,
    const EOS_LobbyModification_RemoveMemberAttributeOptions* Options
) {
    LobbyModificationHandle* mod = (LobbyModificationHandle*)Handle;

    if (!mod || mod->magic != 0x4C4D4F44) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_LOBBYMODIFICATION_REMOVEMEMBERATTRIBUTE_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!Options->Key) {
        return EOS_InvalidParameters;
    }

    // Find and remove member attribute
    for (int i = 0; i < mod->member_attribute_count; i++) {
        if (strcmp(mod->member_attributes[i].key, Options->Key) == 0) {
            // Shift remaining member attributes down
            memmove(&mod->member_attributes[i],
                    &mod->member_attributes[i + 1],
                    sizeof(LobbyAttribute) * (mod->member_attribute_count - i - 1));
            mod->member_attribute_count--;
            return EOS_Success;
        }
    }

    return EOS_NotFound;
}

EOS_DECLARE_FUNC(void) EOS_LobbyModification_Release(EOS_HLobbyModification Handle) {
    LobbyModificationHandle* mod = (LobbyModificationHandle*)Handle;

    if (mod && mod->magic == 0x4C4D4F44) {
        mod->magic = 0;  // Invalidate
        free(mod);
    }
}
