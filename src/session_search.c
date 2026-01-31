#include "eos/eos_sessions.h"
#include "eos/eos_sessions_types.h"
#include "internal/sessions_internal.h"
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

// Helper function to check if a session matches a search parameter
static bool session_matches_param(const Session* session, const SearchParameter* param) {
    // Find attribute in session
    const SessionAttribute* attr = NULL;
    for (int i = 0; i < session->attribute_count; i++) {
        if (strcmp(session->attributes[i].key, param->key) == 0) {
            attr = &session->attributes[i];
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
            bool session_val = (attr->value.as_bool == EOS_TRUE);
            bool search_val = (param->value.as_bool == EOS_TRUE);
            switch (param->comparison) {
                case EOS_CO_EQUAL: return session_val == search_val;
                case EOS_CO_NOTEQUAL: return session_val != search_val;
                default: return false;
            }
        }
        case EOS_AT_INT64: {
            int64_t session_val = attr->value.as_int64;
            int64_t search_val = param->value.as_int64;
            switch (param->comparison) {
                case EOS_CO_EQUAL: return session_val == search_val;
                case EOS_CO_NOTEQUAL: return session_val != search_val;
                case EOS_CO_GREATERTHAN: return session_val > search_val;
                case EOS_CO_GREATERTHANOREQUAL: return session_val >= search_val;
                case EOS_CO_LESSTHAN: return session_val < search_val;
                case EOS_CO_LESSTHANOREQUAL: return session_val <= search_val;
                default: return false;
            }
        }
        case EOS_AT_DOUBLE: {
            double session_val = attr->value.as_double;
            double search_val = param->value.as_double;
            switch (param->comparison) {
                case EOS_CO_EQUAL: return session_val == search_val;
                case EOS_CO_NOTEQUAL: return session_val != search_val;
                case EOS_CO_GREATERTHAN: return session_val > search_val;
                case EOS_CO_GREATERTHANOREQUAL: return session_val >= search_val;
                case EOS_CO_LESSTHAN: return session_val < search_val;
                case EOS_CO_LESSTHANOREQUAL: return session_val <= search_val;
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

// SessionSearch functions

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_SetSessionId(
    EOS_HSessionSearch Handle,
    const EOS_SessionSearch_SetSessionIdOptions* Options
) {
    SessionSearchHandle* search = (SessionSearchHandle*)Handle;

    if (!search || search->magic != 0x53534348) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONSEARCH_SETSESSIONID_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!Options->SessionId) {
        return EOS_InvalidParameters;
    }

    strncpy(search->target_session_id, Options->SessionId, sizeof(search->target_session_id) - 1);
    search->target_session_id[sizeof(search->target_session_id) - 1] = '\0';

    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_SetTargetUserId(
    EOS_HSessionSearch Handle,
    const EOS_SessionSearch_SetTargetUserIdOptions* Options
) {
    SessionSearchHandle* search = (SessionSearchHandle*)Handle;

    if (!search || search->magic != 0x53534348) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONSEARCH_SETTARGETUSERID_API_LATEST) {
        return EOS_InvalidParameters;
    }

    search->target_user_id = Options->TargetUserId;

    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_SetParameter(
    EOS_HSessionSearch Handle,
    const EOS_SessionSearch_SetParameterOptions* Options
) {
    SessionSearchHandle* search = (SessionSearchHandle*)Handle;

    if (!search || search->magic != 0x53534348) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONSEARCH_SETPARAMETER_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!Options->Parameter || !Options->Parameter->Key) {
        return EOS_InvalidParameters;
    }

    if (search->param_count >= MAX_SEARCH_PARAMS) {
        return EOS_LimitExceeded;
    }

    const EOS_Sessions_AttributeData* attr_data = Options->Parameter;
    SearchParameter* param = &search->params[search->param_count++];

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

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_RemoveParameter(
    EOS_HSessionSearch Handle,
    const EOS_SessionSearch_RemoveParameterOptions* Options
) {
    SessionSearchHandle* search = (SessionSearchHandle*)Handle;

    if (!search || search->magic != 0x53534348) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONSEARCH_REMOVEPARAMETER_API_LATEST) {
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
                    sizeof(SearchParameter) * (search->param_count - i - 1));
            search->param_count--;
            return EOS_Success;
        }
    }

    return EOS_NotFound;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_SetMaxResults(
    EOS_HSessionSearch Handle,
    const EOS_SessionSearch_SetMaxResultsOptions* Options
) {
    SessionSearchHandle* search = (SessionSearchHandle*)Handle;

    if (!search || search->magic != 0x53534348) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONSEARCH_SETMAXSEARCHRESULTS_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (Options->MaxSearchResults > EOS_SESSIONS_MAX_SEARCH_RESULTS) {
        return EOS_InvalidParameters;
    }

    search->max_results = Options->MaxSearchResults;

    return EOS_Success;
}

EOS_DECLARE_FUNC(void) EOS_SessionSearch_Find(
    EOS_HSessionSearch Handle,
    const EOS_SessionSearch_FindOptions* Options,
    void* ClientData,
    const EOS_SessionSearch_OnFindCallback CompletionDelegate
) {
    SessionSearchHandle* search = (SessionSearchHandle*)Handle;

    EOS_SessionSearch_FindCallbackInfo info = {0};
    info.ResultCode = EOS_InvalidParameters;
    info.ClientData = ClientData;

    if (!search || search->magic != 0x53534348) {
        goto queue_callback;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONSEARCH_FIND_API_LATEST) {
        goto queue_callback;
    }

    SessionsState* state = search->sessions_state;
    if (!state || state->magic != 0x53455353) {
        goto queue_callback;
    }

    // Free previous results
    if (search->results) {
        free(search->results);
        search->results = NULL;
        search->result_count = 0;
    }

    // Allocate results buffer
    search->results = calloc(search->max_results, sizeof(Session));
    if (!search->results) {
        info.ResultCode = EOS_LimitExceeded;
        goto queue_callback;
    }

    // Send discovery query to trigger fresh announcements from hosts
    if (state && state->discovery) {
        discovery_send_query(state->discovery, NULL);  // Send query broadcast

        // Poll for responses (give hosts 300ms to respond)
        for (int poll_count = 0; poll_count < 3; poll_count++) {
            discovery_poll(state->discovery);

            #ifdef _WIN32
            Sleep(100);  // 100ms
            #else
            usleep(100000);  // 100ms
            #endif

            // Copy discovered sessions from discovery cache to state array
            int discovered_count = 0;
            Session* discovered = discovery_get_sessions(state->discovery, &discovered_count);
            state->discovered_session_count = (discovered_count > MAX_DISCOVERED_SESSIONS)
                ? MAX_DISCOVERED_SESSIONS : discovered_count;
            for (int j = 0; j < state->discovered_session_count; j++) {
                state->discovered_sessions[j] = discovered[j];
            }

            if (state->discovered_session_count > 0) {
                break;  // Found sessions, no need to continue polling
            }
        }
    }

    // Filter discovered sessions
    for (int i = 0; i < state->discovered_session_count && search->result_count < (int)search->max_results; i++) {
        Session* s = &state->discovered_sessions[i];

        if (!s->valid) {
            continue;
        }

        // Check session ID filter
        if (search->target_session_id[0] != '\0') {
            if (strcmp(s->session_id, search->target_session_id) != 0) {
                continue;
            }
        }

        // Check target user ID filter
        if (search->target_user_id != NULL) {
            bool user_found = false;
            for (int j = 0; j < s->registered_player_count; j++) {
                if (s->registered_players[j] == search->target_user_id) {
                    user_found = true;
                    break;
                }
            }
            if (!user_found) {
                continue;
            }
        }

        // Check parameter filters
        bool matches = true;
        for (int p = 0; p < search->param_count && matches; p++) {
            matches = session_matches_param(s, &search->params[p]);
        }

        if (matches) {
            search->results[search->result_count++] = *s;
        }
    }

    search->search_complete = true;
    info.ResultCode = (search->result_count > 0) ? EOS_Success : EOS_NotFound;

queue_callback:
    if (CompletionDelegate && state && state->platform && state->platform->callbacks) {
        callback_queue_push(state->platform->callbacks, (void*)CompletionDelegate, &info, sizeof(info));
    }
}

EOS_DECLARE_FUNC(uint32_t) EOS_SessionSearch_GetSearchResultCount(
    EOS_HSessionSearch Handle,
    const EOS_SessionSearch_GetSearchResultCountOptions* Options
) {
    SessionSearchHandle* search = (SessionSearchHandle*)Handle;

    if (!search || search->magic != 0x53534348) {
        return 0;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONSEARCH_GETSEARCHRESULTCOUNT_API_LATEST) {
        return 0;
    }

    if (!search->search_complete) {
        return 0;
    }

    return (uint32_t)search->result_count;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_CopySearchResultByIndex(
    EOS_HSessionSearch Handle,
    const EOS_SessionSearch_CopySearchResultByIndexOptions* Options,
    EOS_HSessionDetails* OutSessionHandle
) {
    SessionSearchHandle* search = (SessionSearchHandle*)Handle;

    if (!search || search->magic != 0x53534348) {
        return EOS_InvalidParameters;
    }

    if (!Options || Options->ApiVersion != EOS_SESSIONSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST) {
        return EOS_InvalidParameters;
    }

    if (!OutSessionHandle) {
        return EOS_InvalidParameters;
    }

    if (!search->search_complete) {
        return EOS_InvalidState;
    }

    if (Options->SessionIndex >= (uint32_t)search->result_count) {
        return EOS_NotFound;
    }

    // Create session details handle
    SessionDetailsHandle* details = calloc(1, sizeof(SessionDetailsHandle));
    if (!details) {
        return EOS_LimitExceeded;
    }

    details->magic = 0x53445448;
    details->session = search->results[Options->SessionIndex];

    *OutSessionHandle = (EOS_HSessionDetails)details;
    return EOS_Success;
}

EOS_DECLARE_FUNC(void) EOS_SessionSearch_Release(EOS_HSessionSearch Handle) {
    SessionSearchHandle* search = (SessionSearchHandle*)Handle;

    if (search && search->magic == 0x53534348) {
        if (search->results) {
            free(search->results);
        }
        search->magic = 0;  // Invalidate
        free(search);
    }
}
