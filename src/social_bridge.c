/**
 * EOS-LAN Emulator - Social bridge (Friends + Presence)
 *
 * Bridges LAN-discovered sessions into the Friends and Presence interfaces so
 * that games whose "Join Game" UI lists *friends' joinable games* (e.g.
 * Satisfactory) can see and join a peer hosting on the same LAN.
 *
 * The discovery layer (sessions.c) already populates
 * platform->sessions->discovered_sessions with peers' sessions (each carrying
 * the owner's ProductUserId + host address). Here we expose each such peer as:
 *   - a Friend (synthetic EpicAccountId derived from the peer's owner id),
 *   - with Online Presence carrying JoinInfo (the session id) so the game
 *     treats the friend as joinable.
 *
 * Identity note: discovered sessions are keyed by ProductUserId (Connect world),
 * but Friends/Presence use EpicAccountId (Auth world). We synthesize a stable
 * EpicAccountId per peer by reusing the peer's 32-hex owner id string; the id is
 * opaque to the game and only needs to round-trip consistently across
 * GetFriendAtIndex -> CopyPresence/GetJoinInfo.
 *
 * The Friends/Presence handles are the platform handle (see
 * EOS_Platform_Get*Interface), so we cast Handle back to PlatformState*.
 */

#include "eos/eos_sdk.h"
#include "eos/eos_friends.h"
#include "eos/eos_presence.h"
#include "internal/platform_internal.h"
#include "internal/sessions_internal.h"
#include "internal/connect_internal.h"
#include "internal/auth_internal.h"
#include "internal/callbacks.h"
#include "internal/logging.h"
#include <string.h>
#include <stdlib.h>

// Synthetic friend registry, rebuilt from discovered sessions on demand.
typedef struct {
    EOS_EpicAccountIdDetails epic;          // synthetic EpicAccountId for the peer
    char session_id[SESSION_ID_LEN + 1];    // the peer's session id (join target)
    char host_address[HOST_ADDRESS_LEN];    // "IP:port"
    int valid;
} PeerFriend;

static PeerFriend g_peers[MAX_DISCOVERED_SESSIONS];
static int g_peer_count = 0;

// Local user's product id string (to exclude our own discovered session).
static const char* local_product_id_string(PlatformState* p) {
    if (!p || !p->connect) return NULL;
    for (int i = 0; i < p->connect->user_count; i++) {
        if (p->connect->users[i].in_use) {
            return p->connect->users[i].user_id.id_string;
        }
    }
    return NULL;
}

// Rebuild the peer registry from the sessions discovery cache.
static void refresh_peers(PlatformState* p) {
    g_peer_count = 0;
    if (!p || !p->sessions) return;
    SessionsState* s = p->sessions;
    const char* self = local_product_id_string(p);
    for (int i = 0; i < s->discovered_session_count && g_peer_count < MAX_DISCOVERED_SESSIONS; i++) {
        Session* ds = &s->discovered_sessions[i];
        if (self && strncmp(ds->owner_id_string, self, OWNER_ID_STRING_LEN) == 0) {
            continue;  // skip our own session
        }
        PeerFriend* pf = &g_peers[g_peer_count];
        pf->epic.magic = EAID_MAGIC;
        // Synthetic EpicAccountId: reuse the peer's 32-hex owner id string.
        strncpy(pf->epic.id_string, ds->owner_id_string, EPIC_ACCOUNT_ID_LENGTH);
        pf->epic.id_string[EPIC_ACCOUNT_ID_LENGTH] = '\0';
        strncpy(pf->session_id, ds->session_id, SESSION_ID_LEN);
        pf->session_id[SESSION_ID_LEN] = '\0';
        strncpy(pf->host_address, ds->host_address, HOST_ADDRESS_LEN - 1);
        pf->host_address[HOST_ADDRESS_LEN - 1] = '\0';
        pf->valid = 1;
        g_peer_count++;
    }
    EOS_LOG_INFO("social_bridge: %d peer friend(s) from discovery", g_peer_count);
}

static PeerFriend* find_peer_by_epic(EOS_EpicAccountId id) {
    if (!id) return NULL;
    EOS_EpicAccountIdDetails* d = (EOS_EpicAccountIdDetails*)id;
    for (int i = 0; i < g_peer_count; i++) {
        if (g_peers[i].valid && strncmp(g_peers[i].epic.id_string, d->id_string, EPIC_ACCOUNT_ID_LENGTH) == 0) {
            return &g_peers[i];
        }
    }
    return NULL;
}

// ===================== Push notifications =====================
// Games (e.g. Satisfactory) query the friends list ONCE when the Join menu
// opens and cache it; re-opening doesn't re-query. So returning peers on demand
// isn't enough — when a peer is discovered AFTER that first query, we must FIRE
// the registered friends-update + presence-changed notifications so the game
// refreshes and the host appears. social_bridge_tick() (called from
// EOS_Platform_Tick) detects newly-discovered peers and fires those callbacks.

static EOS_Friends_OnFriendsUpdateCallback g_friends_cb = NULL;
static void* g_friends_cd = NULL;
static EOS_Presence_OnPresenceChangedCallback g_presence_cb = NULL;
static void* g_presence_cd = NULL;

static char g_notified[MAX_DISCOVERED_SESSIONS][EPIC_ACCOUNT_ID_LENGTH + 1];
static int g_notified_count = 0;

static int already_notified(const char* epic_id) {
    for (int i = 0; i < g_notified_count; i++) {
        if (strncmp(g_notified[i], epic_id, EPIC_ACCOUNT_ID_LENGTH) == 0) return 1;
    }
    return 0;
}

void social_bridge_tick(PlatformState* p) {
    if (!p || !p->callbacks) return;
    if (!g_friends_cb && !g_presence_cb) return;  // nobody listening yet
    refresh_peers(p);
    for (int i = 0; i < g_peer_count; i++) {
        if (already_notified(g_peers[i].epic.id_string)) continue;
        EOS_EpicAccountId local = (EOS_EpicAccountId)&g_auth_state.account_id;
        EOS_EpicAccountId peer = (EOS_EpicAccountId)&g_peers[i].epic;
        if (g_friends_cb) {
            EOS_Friends_OnFriendsUpdateInfo info = {0};
            info.ClientData = g_friends_cd;
            info.LocalUserId = local;
            info.TargetUserId = peer;
            info.PreviousStatus = EOS_FS_NotFriends;
            info.CurrentStatus = EOS_FS_Friends;
            callback_queue_push(p->callbacks, (void*)g_friends_cb, &info, sizeof(info));
        }
        if (g_presence_cb) {
            EOS_Presence_PresenceChangedCallbackInfo pinfo = {0};
            pinfo.ClientData = g_presence_cd;
            pinfo.LocalUserId = local;
            pinfo.PresenceUserId = peer;
            callback_queue_push(p->callbacks, (void*)g_presence_cb, &pinfo, sizeof(pinfo));
        }
        if (g_notified_count < MAX_DISCOVERED_SESSIONS) {
            strncpy(g_notified[g_notified_count], g_peers[i].epic.id_string, EPIC_ACCOUNT_ID_LENGTH);
            g_notified[g_notified_count][EPIC_ACCOUNT_ID_LENGTH] = '\0';
            g_notified_count++;
        }
        EOS_LOG_INFO("social_bridge: fired friends/presence update for new peer");
    }
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Friends_AddNotifyFriendsUpdate(EOS_HFriends Handle, const EOS_Friends_AddNotifyFriendsUpdateOptions* Options, void* ClientData, const EOS_Friends_OnFriendsUpdateCallback FriendsUpdateHandler) {
    (void)Handle; (void)Options;
    g_friends_cb = FriendsUpdateHandler;
    g_friends_cd = ClientData;
    g_notified_count = 0;  // re-arm so existing peers get announced
    EOS_LOG_INFO(">>> EOS_Friends_AddNotifyFriendsUpdate registered");
    return (EOS_NotificationId)0x1001;
}

EOS_DECLARE_FUNC(void) EOS_Friends_RemoveNotifyFriendsUpdate(EOS_HFriends Handle, EOS_NotificationId NotificationId) {
    (void)Handle; (void)NotificationId;
    g_friends_cb = NULL; g_friends_cd = NULL;
}

EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Presence_AddNotifyOnPresenceChanged(EOS_HPresence Handle, const EOS_Presence_AddNotifyOnPresenceChangedOptions* Options, void* ClientData, const EOS_Presence_OnPresenceChangedCallback NotificationHandler) {
    (void)Handle; (void)Options;
    g_presence_cb = NotificationHandler;
    g_presence_cd = ClientData;
    g_notified_count = 0;
    EOS_LOG_INFO(">>> EOS_Presence_AddNotifyOnPresenceChanged registered");
    return (EOS_NotificationId)0x1002;
}

EOS_DECLARE_FUNC(void) EOS_Presence_RemoveNotifyOnPresenceChanged(EOS_HPresence Handle, EOS_NotificationId NotificationId) {
    (void)Handle; (void)NotificationId;
    g_presence_cb = NULL; g_presence_cd = NULL;
}

// ===================== Friends =====================

EOS_DECLARE_FUNC(void) EOS_Friends_QueryFriends(EOS_HFriends Handle, const EOS_Friends_QueryFriendsOptions* Options, void* ClientData, const EOS_Friends_OnQueryFriendsCallback CompletionDelegate) {
    EOS_LOG_INFO(">>> EOS_Friends_QueryFriends CALLED");
    PlatformState* p = (PlatformState*)Handle;
    refresh_peers(p);
    EOS_Friends_QueryFriendsCallbackInfo info = {0};
    info.ResultCode = EOS_Success;
    info.ClientData = ClientData;
    info.LocalUserId = Options ? Options->LocalUserId : NULL;
    if (CompletionDelegate && p && p->callbacks) {
        callback_queue_push(p->callbacks, (void*)CompletionDelegate, &info, sizeof(info));
    }
}

EOS_DECLARE_FUNC(int32_t) EOS_Friends_GetFriendsCount(EOS_HFriends Handle, const EOS_Friends_GetFriendsCountOptions* Options) {
    (void)Options;
    PlatformState* p = (PlatformState*)Handle;
    refresh_peers(p);
    EOS_LOG_INFO(">>> EOS_Friends_GetFriendsCount -> %d", g_peer_count);
    return g_peer_count;
}

EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_Friends_GetFriendAtIndex(EOS_HFriends Handle, const EOS_Friends_GetFriendAtIndexOptions* Options) {
    (void)Handle;
    if (!Options || Options->Index < 0 || Options->Index >= g_peer_count) {
        EOS_LOG_WARN("GetFriendAtIndex: index out of range");
        return NULL;
    }
    return (EOS_EpicAccountId)&g_peers[Options->Index].epic;
}

EOS_DECLARE_FUNC(EOS_EFriendsStatus) EOS_Friends_GetStatus(EOS_HFriends Handle, const EOS_Friends_GetStatusOptions* Options) {
    (void)Handle; (void)Options;
    // Every discovered peer is treated as an accepted friend so it shows up.
    return EOS_FS_Friends;
}

// ===================== Presence =====================

EOS_DECLARE_FUNC(void) EOS_Presence_QueryPresence(EOS_HPresence Handle, const EOS_Presence_QueryPresenceOptions* Options, void* ClientData, const EOS_Presence_OnQueryPresenceCompleteCallback CompletionDelegate) {
    EOS_LOG_INFO(">>> EOS_Presence_QueryPresence CALLED");
    PlatformState* p = (PlatformState*)Handle;
    refresh_peers(p);
    EOS_Presence_QueryPresenceCallbackInfo info = {0};
    info.ResultCode = EOS_Success;
    info.ClientData = ClientData;
    info.LocalUserId = Options ? Options->LocalUserId : NULL;
    info.TargetUserId = Options ? Options->TargetUserId : NULL;
    if (CompletionDelegate && p && p->callbacks) {
        callback_queue_push(p->callbacks, (void*)CompletionDelegate, &info, sizeof(info));
    }
}

EOS_DECLARE_FUNC(EOS_Bool) EOS_Presence_HasPresence(EOS_HPresence Handle, const EOS_Presence_HasPresenceOptions* Options) {
    (void)Handle;
    EOS_EpicAccountId target = Options ? Options->TargetUserId : NULL;
    return find_peer_by_epic(target) ? EOS_TRUE : EOS_FALSE;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Presence_CopyPresence(EOS_HPresence Handle, const EOS_Presence_CopyPresenceOptions* Options, EOS_Presence_Info** OutPresence) {
    (void)Handle;
    if (!OutPresence) return EOS_InvalidParameters;
    *OutPresence = NULL;
    EOS_EpicAccountId target = Options ? Options->TargetUserId : NULL;
    PeerFriend* pf = find_peer_by_epic(target);
    if (!pf) {
        return EOS_NotFound;
    }
    EOS_Presence_Info* info = calloc(1, sizeof(EOS_Presence_Info));
    if (!info) return EOS_UnexpectedError;
    info->ApiVersion = EOS_PRESENCE_INFO_API_LATEST;
    info->Status = EOS_PS_Online;
    info->UserId = target;
    info->ProductId = "Satisfactory";
    info->ProductVersion = "1.0";
    info->Platform = "EOSLAN";
    info->RichText = "Hosting a game";
    info->RecordsCount = 0;
    info->Records = NULL;
    info->ProductName = "Satisfactory";
    info->IntegratedPlatform = "";
    *OutPresence = info;
    EOS_LOG_INFO(">>> EOS_Presence_CopyPresence: returning Online presence for peer");
    return EOS_Success;
}

EOS_DECLARE_FUNC(void) EOS_Presence_Info_Release(EOS_Presence_Info* PresenceInfo) {
    if (PresenceInfo) free(PresenceInfo);
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Presence_GetJoinInfo(EOS_HPresence Handle, const EOS_Presence_GetJoinInfoOptions* Options, char* OutBuffer, int32_t* InOutBufferLength) {
    (void)Handle;
    if (!OutBuffer || !InOutBufferLength) return EOS_InvalidParameters;
    EOS_EpicAccountId target = Options ? Options->TargetUserId : NULL;
    PeerFriend* pf = find_peer_by_epic(target);
    if (!pf) {
        *InOutBufferLength = 0;
        return EOS_NotFound;
    }
    // Join info = the peer's session id; the joiner resolves+joins this session.
    int need = (int)strlen(pf->session_id) + 1;
    if (*InOutBufferLength < need) {
        *InOutBufferLength = need;
        return EOS_LimitExceeded;
    }
    strcpy(OutBuffer, pf->session_id);
    *InOutBufferLength = need;
    EOS_LOG_INFO(">>> EOS_Presence_GetJoinInfo: '%s'", pf->session_id);
    return EOS_Success;
}
