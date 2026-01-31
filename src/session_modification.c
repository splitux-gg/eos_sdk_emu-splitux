#include "eos/eos_sessions.h"
#include "eos/eos_sessions_types.h"
#include "internal/sessions_internal.h"
#include <stdlib.h>
#include <string.h>

// SessionModification handle functions

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetBucketId(
    EOS_HSessionModification Handle,
    const EOS_SessionModification_SetBucketIdOptions* Options
) {
    SessionModificationHandle* mod = (SessionModificationHandle*)Handle;

    if (!mod || mod->magic != 0x534D4F44) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONMODIFICATION_SETBUCKETID_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!Options->BucketId) {
        return EOS_InvalidParameters;
    }

    strncpy(mod->session.bucket_id, Options->BucketId, sizeof(mod->session.bucket_id) - 1);
    mod->session.bucket_id[sizeof(mod->session.bucket_id) - 1] = '\0';

    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetHostAddress(
    EOS_HSessionModification Handle,
    const EOS_SessionModification_SetHostAddressOptions* Options
) {
    SessionModificationHandle* mod = (SessionModificationHandle*)Handle;

    if (!mod || mod->magic != 0x534D4F44) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONMODIFICATION_SETHOSTADDRESS_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!Options->HostAddress) {
        return EOS_InvalidParameters;
    }

    strncpy(mod->session.host_address, Options->HostAddress, sizeof(mod->session.host_address) - 1);
    mod->session.host_address[sizeof(mod->session.host_address) - 1] = '\0';

    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetPermissionLevel(
    EOS_HSessionModification Handle,
    const EOS_SessionModification_SetPermissionLevelOptions* Options
) {
    SessionModificationHandle* mod = (SessionModificationHandle*)Handle;

    if (!mod || mod->magic != 0x534D4F44) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONMODIFICATION_SETPERMISSIONLEVEL_API_LATEST) {
        return EOS_InvalidParameters;
    }

    mod->session.permission_level = Options->PermissionLevel;

    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetJoinInProgressAllowed(
    EOS_HSessionModification Handle,
    const EOS_SessionModification_SetJoinInProgressAllowedOptions* Options
) {
    SessionModificationHandle* mod = (SessionModificationHandle*)Handle;

    if (!mod || mod->magic != 0x534D4F44) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONMODIFICATION_SETJOININPROGRESSALLOWED_API_LATEST) {
        return EOS_InvalidParameters;
    }

    mod->session.join_in_progress_allowed = (Options->bAllowJoinInProgress == EOS_TRUE);

    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetMaxPlayers(
    EOS_HSessionModification Handle,
    const EOS_SessionModification_SetMaxPlayersOptions* Options
) {
    SessionModificationHandle* mod = (SessionModificationHandle*)Handle;

    if (!mod || mod->magic != 0x534D4F44) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONMODIFICATION_SETMAXPLAYERS_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (Options->MaxPlayers == 0 || Options->MaxPlayers > EOS_SESSIONS_MAXREGISTEREDPLAYERS) {
        return EOS_InvalidParameters;
    }

    mod->session.max_players = Options->MaxPlayers;

    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetInvitesAllowed(
    EOS_HSessionModification Handle,
    const EOS_SessionModification_SetInvitesAllowedOptions* Options
) {
    SessionModificationHandle* mod = (SessionModificationHandle*)Handle;

    if (!mod || mod->magic != 0x534D4F44) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONMODIFICATION_SETINVITESALLOWED_API_LATEST) {
        return EOS_InvalidParameters;
    }

    mod->session.invites_allowed = (Options->bInvitesAllowed == EOS_TRUE);

    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_AddAttribute(
    EOS_HSessionModification Handle,
    const EOS_SessionModification_AddAttributeOptions* Options
) {
    SessionModificationHandle* mod = (SessionModificationHandle*)Handle;

    if (!mod || mod->magic != 0x534D4F44) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONMODIFICATION_ADDATTRIBUTE_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!Options->SessionAttribute || !Options->SessionAttribute->Key) {
        return EOS_InvalidParameters;
    }

    const EOS_Sessions_AttributeData* attr_data = Options->SessionAttribute;

    // Check if attribute already exists and update it
    int existing_index = -1;
    for (int i = 0; i < mod->session.attribute_count; i++) {
        if (strcmp(mod->session.attributes[i].key, attr_data->Key) == 0) {
            existing_index = i;
            break;
        }
    }

    SessionAttribute* attr;
    if (existing_index >= 0) {
        // Update existing attribute
        attr = &mod->session.attributes[existing_index];
    } else {
        // Add new attribute
        if (mod->session.attribute_count >= MAX_SESSION_ATTRIBUTES) {
            return EOS_LimitExceeded;
        }
        attr = &mod->session.attributes[mod->session.attribute_count++];
    }

    // Copy attribute data
    strncpy(attr->key, attr_data->Key, sizeof(attr->key) - 1);
    attr->key[sizeof(attr->key) - 1] = '\0';
    attr->type = attr_data->ValueType;
    attr->advertisement = Options->AdvertisementType;

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

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_RemoveAttribute(
    EOS_HSessionModification Handle,
    const EOS_SessionModification_RemoveAttributeOptions* Options
) {
    SessionModificationHandle* mod = (SessionModificationHandle*)Handle;

    if (!mod || mod->magic != 0x534D4F44) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONMODIFICATION_REMOVEATTRIBUTE_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!Options->Key) {
        return EOS_InvalidParameters;
    }

    // Find and remove attribute
    for (int i = 0; i < mod->session.attribute_count; i++) {
        if (strcmp(mod->session.attributes[i].key, Options->Key) == 0) {
            // Shift remaining attributes down
            memmove(&mod->session.attributes[i],
                    &mod->session.attributes[i + 1],
                    sizeof(SessionAttribute) * (mod->session.attribute_count - i - 1));
            mod->session.attribute_count--;
            return EOS_Success;
        }
    }

    return EOS_NotFound;
}

EOS_DECLARE_FUNC(void) EOS_SessionModification_Release(EOS_HSessionModification Handle) {
    SessionModificationHandle* mod = (SessionModificationHandle*)Handle;

    if (mod && mod->magic == 0x534D4F44) {
        mod->magic = 0;  // Invalidate
        free(mod);
    }
}
