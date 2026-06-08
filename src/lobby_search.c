#include "eos/eos_lobby.h"
#include "eos/eos_lobby_types.h"
#include "internal/lobby_internal.h"
#include "internal/callbacks.h"
#include "internal/lan_discovery.h"
#include "internal/logging.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// Helper function to check if a lobby matches a search parameter
static bool lobby_matches_param(const Lobby* lobby, const LobbySearchParameter* param) {
    // Find attribute in lobby
    const LobbyAttribute* attr = NULL;
    for (int i = 0; i < lobby->attribute_count; i++) {
        if (strcmp(lobby->attributes[i].key, param->key) == 0) {
            attr = &lobby->attributes[i];
            break;
        }
    }

    if (!attr) {
        return false;  // Attribute not found
    }

    if (attr->type != param->type) {
        return false;  // Type mismatch
    }

    // Perform comparison based on type
    switch (param->type) {
        case EOS_AT_BOOLEAN: {
            bool lobby_val = (attr->value.as_bool == EOS_TRUE);
            bool search_val = (param->value.as_bool == EOS_TRUE);
            switch (param->comparison) {
                case EOS_CO_EQUAL: return lobby_val == search_val;
                case EOS_CO_NOTEQUAL: return lobby_val != search_val;
                default: return false;
            }
        }
        case EOS_AT_INT64: {
            int64_t lobby_val = attr->value.as_int64;
            int64_t search_val = param->value.as_int64;
            switch (param->comparison) {
                case EOS_CO_EQUAL: return lobby_val == search_val;
                case EOS_CO_NOTEQUAL: return lobby_val != search_val;
                case EOS_CO_GREATERTHAN: return lobby_val > search_val;
                case EOS_CO_GREATERTHANOREQUAL: return lobby_val >= search_val;
                case EOS_CO_LESSTHAN: return lobby_val < search_val;
                case EOS_CO_LESSTHANOREQUAL: return lobby_val <= search_val;
                default: return false;
            }
        }
        case EOS_AT_DOUBLE: {
            double lobby_val = attr->value.as_double;
            double search_val = param->value.as_double;
            switch (param->comparison) {
                case EOS_CO_EQUAL: return lobby_val == search_val;
                case EOS_CO_NOTEQUAL: return lobby_val != search_val;
                case EOS_CO_GREATERTHAN: return lobby_val > search_val;
                case EOS_CO_GREATERTHANOREQUAL: return lobby_val >= search_val;
                case EOS_CO_LESSTHAN: return lobby_val < search_val;
                case EOS_CO_LESSTHANOREQUAL: return lobby_val <= search_val;
                default: return false;
            }
        }
        case EOS_AT_STRING: {
            int cmp = strcmp(attr->value.as_string, param->value.as_string);
            switch (param->comparison) {
                case EOS_CO_EQUAL: return cmp == 0;
                case EOS_CO_NOTEQUAL: return cmp != 0;
                case EOS_CO_CONTAINS: return strstr(attr->value.as_string, param->value.as_string) != NULL;
                // EOS_CO_NOTCONTAINS not in SDK, skip
                default: return false;
            }
        }
        default:
            return false;
    }
}

// LobbySearch functions

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetLobbyId(
    EOS_HLobbySearch Handle,
    const EOS_LobbySearch_SetLobbyIdOptions* Options
) {
    LobbySearchHandle* search = (LobbySearchHandle*)Handle;

    if (!search || search->magic != 0x4C534348) {
        return EOS_InvalidParameters;
    }

    if (!Options) {
        return EOS_InvalidParameters;
    }

    if (!Options->LobbyId) {
        return EOS_InvalidParameters;
    }

    strncpy(search->target_lobby_id, Options->LobbyId, sizeof(search->target_lobby_id) - 1);
    search->target_lobby_id[sizeof(search->target_lobby_id) - 1] = '\0';

    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetTargetUserId(
    EOS_HLobbySearch Handle,
    const EOS_LobbySearch_SetTargetUserIdOptions* Options
) {
    LobbySearchHandle* search = (LobbySearchHandle*)Handle;

    if (!search || search->magic != 0x4C534348) {
        return EOS_InvalidParameters;
    }

    if (!Options) {
        return EOS_InvalidParameters;
    }

    if (!Options->TargetUserId) {
        return EOS_InvalidParameters;
    }

    search->target_user_id = Options->TargetUserId;

    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetParameter(
    EOS_HLobbySearch Handle,
    const EOS_LobbySearch_SetParameterOptions* Options
) {
    LobbySearchHandle* search = (LobbySearchHandle*)Handle;

    if (!search || search->magic != 0x4C534348) {
        return EOS_InvalidParameters;
    }

    if (!Options) {
        return EOS_InvalidParameters;
    }

    if (!Options->Parameter || !Options->Parameter->Key) {
        return EOS_InvalidParameters;
    }

    if (search->param_count >= MAX_LOBBY_SEARCH_PARAMS) {
        return EOS_LimitExceeded;
    }

    const EOS_Lobby_AttributeData* attr_data = Options->Parameter;
    LobbySearchParameter* param = &search->params[search->param_count++];

    strncpy(param->key, attr_data->Key, sizeof(param->key) - 1);
    param->key[sizeof(param->key) - 1] = '\0';
    param->type = attr_data->ValueType;
    param->comparison = Options->ComparisonOp;

    switch (attr_data->ValueType) {
        case EOS_AT_BOOLEAN:
            param->value.as_bool = attr_data->Value.AsBool;
            break;
        case EOS_AT_INT64:
            param->value.as_int64 = attr_data->Value.AsInt64;
            break;
        case EOS_AT_DOUBLE:
            param->value.as_double = attr_data->Value.AsDouble;
            break;
        case EOS_AT_STRING:
            if (attr_data->Value.AsUtf8) {
                strncpy(param->value.as_string, attr_data->Value.AsUtf8, sizeof(param->value.as_string) - 1);
                param->value.as_string[sizeof(param->value.as_string) - 1] = '\0';
            } else {
                param->value.as_string[0] = '\0';
            }
            break;
        default:
            search->param_count--;  // Rollback
            return EOS_InvalidParameters;
    }

    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_RemoveParameter(
    EOS_HLobbySearch Handle,
    const EOS_LobbySearch_RemoveParameterOptions* Options
) {
    LobbySearchHandle* search = (LobbySearchHandle*)Handle;

    if (!search || search->magic != 0x4C534348) {
        return EOS_InvalidParameters;
    }

    if (!Options) {
        return EOS_InvalidParameters;
    }

    if (!Options->Key) {
        return EOS_InvalidParameters;
    }

    // Find and remove parameter
    for (int i = 0; i < search->param_count; i++) {
        if (strcmp(search->params[i].key, Options->Key) == 0 &&
            search->params[i].comparison == Options->ComparisonOp) {
            // Shift remaining parameters down
            memmove(&search->params[i],
                    &search->params[i + 1],
                    sizeof(LobbySearchParameter) * (search->param_count - i - 1));
            search->param_count--;
            return EOS_Success;
        }
    }

    return EOS_NotFound;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_SetMaxResults(
    EOS_HLobbySearch Handle,
    const EOS_LobbySearch_SetMaxResultsOptions* Options
) {
    LobbySearchHandle* search = (LobbySearchHandle*)Handle;

    if (!search || search->magic != 0x4C534348) {
        return EOS_InvalidParameters;
    }

    if (!Options) {
        return EOS_InvalidParameters;
    }

    if (Options->MaxResults == 0 || Options->MaxResults > EOS_LOBBY_MAX_SEARCH_RESULTS) {
        return EOS_InvalidParameters;
    }

    search->max_results = Options->MaxResults;

    return EOS_Success;
}

EOS_DECLARE_FUNC(void) EOS_LobbySearch_Find(
    EOS_HLobbySearch Handle,
    const EOS_LobbySearch_FindOptions* Options,
    void* ClientData,
    const EOS_LobbySearch_OnFindCallback CompletionDelegate
) {
    LobbySearchHandle* search = (LobbySearchHandle*)Handle;
    LobbyState* state = NULL;

    EOS_LobbySearch_FindCallbackInfo info = {0};
    info.ResultCode = EOS_InvalidParameters;
    info.ClientData = ClientData;

    if (!search || search->magic != 0x4C534348) {
        goto queue_callback;
    }

    if (!Options) {
        goto queue_callback;
    }

    state = search->lobby_state;
    if (!state || state->magic != 0x4C4F4259) {
        goto queue_callback;
    }

    // Free previous results
    if (search->results) {
        free(search->results);
        search->results = NULL;
        search->result_count = 0;
    }

    // Allocate results buffer
    search->results = calloc(search->max_results, sizeof(Lobby));
    if (!search->results) {
        info.ResultCode = EOS_LimitExceeded;
        goto queue_callback;
    }

    // Send discovery query to trigger fresh lobby announcements from hosts
    if (state->discovery) {
        discovery_send_query(state->discovery, NULL);  // Send query broadcast

        // Poll for responses (give hosts 300ms to respond). lobby_tick (in
        // lobby.c) owns the announcement->discovered_lobbies conversion; here we
        // just drive the network and read the refreshed state array.
        for (int poll_count = 0; poll_count < 3; poll_count++) {
            discovery_poll(state->discovery);

            #ifdef _WIN32
            Sleep(100);  // 100ms
            #else
            usleep(100000);  // 100ms
            #endif

            if (state->discovered_lobby_count > 0) {
                break;  // Found lobbies, no need to continue polling
            }
        }
    }

    // Dump the search criteria (verbose - DEBUG only; invaluable when bringing
    // up a new game's search params, but noise in a normal co-op run).
    EOS_LOG_DEBUG("LobbySearch_Find: criteria max_results=%d target_lobby_id='%s' target_user_id=%s param_count=%d ; discovered_lobby_count=%d",
                 (int)search->max_results, search->target_lobby_id,
                 search->target_user_id ? "(set)" : "(null)",
                 search->param_count, state->discovered_lobby_count);
    for (int p = 0; p < search->param_count; p++) {
        EOS_LOG_DEBUG("LobbySearch_Find:   param[%d] key='%s' type=%d comparison=%d",
                     p, search->params[p].key, (int)search->params[p].type,
                     (int)search->params[p].comparison);
    }

    // Filter discovered lobbies
    for (int i = 0; i < state->discovered_lobby_count && search->result_count < (int)search->max_results; i++) {
        Lobby* l = &state->discovered_lobbies[i];

        if (!l->valid) {
            continue;
        }

        EOS_LOG_INFO("LobbySearch_Find:   candidate[%d] id='%s' owner='%s' member_count=%d attribute_count=%d",
                     i, l->lobby_id, l->owner_id_string, l->member_count, l->attribute_count);
        EOS_LOG_DEBUG("LobbySearch_Find:     cap: max_members=%u member_count=%d",
                     l->max_members, l->member_count);
        for (int a = 0; a < l->attribute_count; a++) {
            const LobbyAttribute* la = &l->attributes[a];
            if (la->type == EOS_AT_INT64) {
                EOS_LOG_DEBUG("LobbySearch_Find:     attr '%s' = %lld (int64)",
                             la->key, (long long)la->value.as_int64);
            } else if (la->type == EOS_AT_BOOLEAN) {
                EOS_LOG_DEBUG("LobbySearch_Find:     attr '%s' = %d (bool)",
                             la->key, (int)la->value.as_bool);
            } else if (la->type == EOS_AT_STRING) {
                EOS_LOG_DEBUG("LobbySearch_Find:     attr '%s' = '%s' (str)",
                             la->key, la->value.as_string);
            }
        }

        // Check lobby ID filter
        if (search->target_lobby_id[0] != '\0') {
            if (strcmp(l->lobby_id, search->target_lobby_id) != 0) {
                EOS_LOG_INFO("LobbySearch_Find:     REJECT lobby_id mismatch (want '%s')", search->target_lobby_id);
                continue;
            }
        }

        // Check target user ID filter (user must be a registered member)
        if (search->target_user_id != NULL) {
            bool user_found = false;
            for (int j = 0; j < l->member_count; j++) {
                if (l->members[j].valid && l->members[j].member_id == search->target_user_id) {
                    user_found = true;
                    break;
                }
            }
            if (!user_found) {
                EOS_LOG_INFO("LobbySearch_Find:     REJECT target_user_id not found among %d members (members[] empty on wire-discovered lobbies)", l->member_count);
                continue;
            }
        }

        // Check parameter filters (implicit AND)
        bool matches = true;
        for (int p = 0; p < search->param_count && matches; p++) {
            matches = lobby_matches_param(l, &search->params[p]);
            if (!matches) {
                EOS_LOG_INFO("LobbySearch_Find:     REJECT param[%d] key='%s' no match (lobby has %d attrs)",
                             p, search->params[p].key, l->attribute_count);
            }
        }

        if (matches) {
            EOS_LOG_INFO("LobbySearch_Find:     ACCEPT candidate[%d]", i);
            search->results[search->result_count++] = *l;
        }
    }

    search->search_complete = true;
    info.ResultCode = (search->result_count > 0) ? EOS_Success : EOS_NotFound;

    EOS_LOG_INFO("Lobby search complete: %d result(s)", search->result_count);

queue_callback:
    if (CompletionDelegate && state && state->platform && state->platform->callbacks) {
        callback_queue_push(state->platform->callbacks, (void*)CompletionDelegate, &info, sizeof(info));
    }
}

EOS_DECLARE_FUNC(uint32_t) EOS_LobbySearch_GetSearchResultCount(
    EOS_HLobbySearch Handle,
    const EOS_LobbySearch_GetSearchResultCountOptions* Options
) {
    LobbySearchHandle* search = (LobbySearchHandle*)Handle;

    if (!search || search->magic != 0x4C534348) {
        return 0;
    }

    if (!Options) {
        return 0;
    }

    if (!search->search_complete) {
        return 0;
    }

    return (uint32_t)search->result_count;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_LobbySearch_CopySearchResultByIndex(
    EOS_HLobbySearch Handle,
    const EOS_LobbySearch_CopySearchResultByIndexOptions* Options,
    EOS_HLobbyDetails* OutLobbyDetailsHandle
) {
    LobbySearchHandle* search = (LobbySearchHandle*)Handle;

    if (!search || search->magic != 0x4C534348) {
        return EOS_InvalidParameters;
    }

    if (!Options) {
        return EOS_InvalidParameters;
    }

    if (!OutLobbyDetailsHandle) {
        return EOS_InvalidParameters;
    }

    if (!search->search_complete) {
        return EOS_InvalidState;
    }

    if (Options->LobbyIndex >= (uint32_t)search->result_count) {
        return EOS_NotFound;
    }

    // Create lobby details handle (snapshot of the lobby for the game to read)
    LobbyDetailsHandle* details = calloc(1, sizeof(LobbyDetailsHandle));
    if (!details) {
        return EOS_LimitExceeded;
    }

    details->magic = 0x4C445448;
    details->lobby = search->results[Options->LobbyIndex];  // flat struct, deep copy
    details->local_user = NULL;  // search results are not tied to a local member

    *OutLobbyDetailsHandle = (EOS_HLobbyDetails)details;
    return EOS_Success;
}

EOS_DECLARE_FUNC(void) EOS_LobbySearch_Release(EOS_HLobbySearch Handle) {
    LobbySearchHandle* search = (LobbySearchHandle*)Handle;

    if (search && search->magic == 0x4C534348) {
        if (search->results) {
            free(search->results);
        }
        search->magic = 0;  // Invalidate
        free(search);
    }
}
