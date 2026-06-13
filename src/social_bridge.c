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
#include "eos/eos_userinfo.h"
#include "internal/platform_internal.h"
#include "internal/sessions_internal.h"
#include "internal/lan_discovery.h"
#include "internal/connect_internal.h"
#include "internal/auth_internal.h"
#include "internal/callbacks.h"
#include "internal/logging.h"
#include "lan_common.h"
#include <string.h>
#include <stdlib.h>

// Synthetic friend registry, rebuilt from discovered sessions + user beacons.
typedef struct {
    EOS_EpicAccountIdDetails epic;          // peer's EpicAccountId (real from beacon, else synthetic = puid)
    char puid[OWNER_ID_STRING_LEN];         // peer's ProductUserId string (dedupe key across sources)
    char session_id[SESSION_ID_LEN + 1];    // the peer's session id (join target; may be empty)
    char host_address[HOST_ADDRESS_LEN];    // "IP:port"
    char join_info[PRESENCE_JOININFO_LEN];  // host's REAL SetJoinInfo string (from announce/beacon)
    char display_name[PEER_DISPLAY_NAME_LEN];
    PresenceRecord records[MAX_PRESENCE_RECORDS];  // host's presence data records
    int record_count;
    int valid;
} PeerFriend;

// ===================== Local presence (published by the host) =====================
// Games set their presence via CreatePresenceModification -> SetJoinInfo/SetData
// -> SetPresence. We must capture the REAL join-info string (the game defines
// its own format and parses it back on the joiner!) and relay it over the LAN
// announce. Fabricating it (e.g. raw session id) breaks the joiner's parser.

typedef struct {
    uint32_t magic;  // 0x504D4F44 = "PMOD"
    char join_info[PRESENCE_JOININFO_LEN];
    int has_join_info;
    char rich_text[PRESENCE_VALUE_LEN];
    int has_rich_text;
    EOS_Presence_EStatus status;
    int has_status;
    PresenceRecord records[MAX_PRESENCE_RECORDS];   // keys to set/update (MERGED on apply)
    int record_count;
    int has_records;
    char deleted_keys[MAX_PRESENCE_RECORDS][PRESENCE_KEY_LEN];  // keys to remove on apply
    int deleted_count;
} PresenceModHandle;

typedef struct {
    char join_info[PRESENCE_JOININFO_LEN];
    char rich_text[PRESENCE_VALUE_LEN];
    PresenceRecord records[MAX_PRESENCE_RECORDS];
    int record_count;
} LocalPresence;

static LocalPresence g_local_presence;

// Accessors for sessions.c (stamps local presence onto the LAN announce).
const char* social_bridge_local_join_info(void) {
    return g_local_presence.join_info;
}

const PresenceRecord* social_bridge_local_records(int* out_count) {
    if (out_count) *out_count = g_local_presence.record_count;
    return g_local_presence.records;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_Presence_CreatePresenceModification(EOS_HPresence Handle, const EOS_Presence_CreatePresenceModificationOptions* Options, EOS_HPresenceModification* OutPresenceModificationHandle) {
    (void)Handle; (void)Options;
    if (!OutPresenceModificationHandle) return EOS_InvalidParameters;
    PresenceModHandle* mod = calloc(1, sizeof(PresenceModHandle));
    if (!mod) return EOS_UnexpectedError;
    mod->magic = 0x504D4F44;
    *OutPresenceModificationHandle = (EOS_HPresenceModification)mod;
    EOS_LOG_INFO(">>> EOS_Presence_CreatePresenceModification -> handle");
    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetJoinInfo(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetJoinInfoOptions* Options) {
    PresenceModHandle* mod = (PresenceModHandle*)Handle;
    if (!mod || mod->magic != 0x504D4F44) return EOS_InvalidParameters;
    if (Options && Options->JoinInfo) {
        strncpy(mod->join_info, Options->JoinInfo, sizeof(mod->join_info) - 1);
        mod->join_info[sizeof(mod->join_info) - 1] = '\0';
    } else {
        mod->join_info[0] = '\0';  // NULL JoinInfo = clear
    }
    mod->has_join_info = 1;
    EOS_LOG_INFO(">>> EOS_PresenceModification_SetJoinInfo: '%s'", mod->join_info);
    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetStatus(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetStatusOptions* Options) {
    PresenceModHandle* mod = (PresenceModHandle*)Handle;
    if (!mod || mod->magic != 0x504D4F44) return EOS_InvalidParameters;
    if (Options) { mod->status = Options->Status; mod->has_status = 1; }
    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetRawRichText(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetRawRichTextOptions* Options) {
    PresenceModHandle* mod = (PresenceModHandle*)Handle;
    if (!mod || mod->magic != 0x504D4F44) return EOS_InvalidParameters;
    if (Options && Options->RichText) {
        strncpy(mod->rich_text, Options->RichText, sizeof(mod->rich_text) - 1);
        mod->rich_text[sizeof(mod->rich_text) - 1] = '\0';
        mod->has_rich_text = 1;
        EOS_LOG_INFO(">>> EOS_PresenceModification_SetRawRichText: '%s'", mod->rich_text);
    }
    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_SetData(EOS_HPresenceModification Handle, const EOS_PresenceModification_SetDataOptions* Options) {
    PresenceModHandle* mod = (PresenceModHandle*)Handle;
    if (!mod || mod->magic != 0x504D4F44) return EOS_InvalidParameters;
    if (!Options || !Options->Records) return EOS_InvalidParameters;
    for (int i = 0; i < Options->RecordsCount; i++) {
        const EOS_Presence_DataRecord* r = &Options->Records[i];
        if (!r->Key) continue;
        EOS_LOG_INFO(">>> EOS_PresenceModification_SetData: '%s' = '%s'", r->Key, r->Value ? r->Value : "(null)");
        // Update existing key or append.
        int slot = -1;
        for (int j = 0; j < mod->record_count; j++) {
            if (strncmp(mod->records[j].key, r->Key, PRESENCE_KEY_LEN) == 0) { slot = j; break; }
        }
        if (slot < 0) {
            if (mod->record_count >= MAX_PRESENCE_RECORDS) {
                EOS_LOG_WARN("SetData: record limit (%d) reached, dropping '%s'", MAX_PRESENCE_RECORDS, r->Key);
                continue;
            }
            slot = mod->record_count++;
        }
        strncpy(mod->records[slot].key, r->Key, PRESENCE_KEY_LEN - 1);
        mod->records[slot].key[PRESENCE_KEY_LEN - 1] = '\0';
        strncpy(mod->records[slot].value, r->Value ? r->Value : "", PRESENCE_VALUE_LEN - 1);
        mod->records[slot].value[PRESENCE_VALUE_LEN - 1] = '\0';
    }
    mod->has_records = 1;
    return EOS_Success;
}

// Record which keys SetData on the modification handle did NOT touch are to be
// removed; here DeleteData explicitly marks keys for removal on apply.
static void mod_mark_deleted(PresenceModHandle* mod, const char* key) {
    if (mod->deleted_count < MAX_PRESENCE_RECORDS) {
        strncpy(mod->deleted_keys[mod->deleted_count], key, PRESENCE_KEY_LEN - 1);
        mod->deleted_keys[mod->deleted_count][PRESENCE_KEY_LEN - 1] = '\0';
        mod->deleted_count++;
    }
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_PresenceModification_DeleteData(EOS_HPresenceModification Handle, const EOS_PresenceModification_DeleteDataOptions* Options) {
    PresenceModHandle* mod = (PresenceModHandle*)Handle;
    if (!mod || mod->magic != 0x504D4F44) return EOS_InvalidParameters;
    if (!Options || !Options->Records) return EOS_InvalidParameters;
    for (int i = 0; i < Options->RecordsCount; i++) {
        const char* key = Options->Records[i].Key;
        if (!key) continue;
        mod_mark_deleted(mod, key);
        for (int j = 0; j < mod->record_count; j++) {
            if (strncmp(mod->records[j].key, key, PRESENCE_KEY_LEN) == 0) {
                mod->records[j] = mod->records[mod->record_count - 1];
                mod->record_count--;
                break;
            }
        }
    }
    mod->has_records = 1;
    return EOS_Success;
}

// Merge one key into the local presence record set (update existing / append).
static void local_presence_set(const char* key, const char* value) {
    for (int i = 0; i < g_local_presence.record_count; i++) {
        if (strncmp(g_local_presence.records[i].key, key, PRESENCE_KEY_LEN) == 0) {
            strncpy(g_local_presence.records[i].value, value, PRESENCE_VALUE_LEN - 1);
            g_local_presence.records[i].value[PRESENCE_VALUE_LEN - 1] = '\0';
            return;
        }
    }
    if (g_local_presence.record_count >= MAX_PRESENCE_RECORDS) return;
    PresenceRecord* nr = &g_local_presence.records[g_local_presence.record_count++];
    strncpy(nr->key, key, PRESENCE_KEY_LEN - 1); nr->key[PRESENCE_KEY_LEN - 1] = '\0';
    strncpy(nr->value, value, PRESENCE_VALUE_LEN - 1); nr->value[PRESENCE_VALUE_LEN - 1] = '\0';
}

static void local_presence_delete(const char* key) {
    for (int i = 0; i < g_local_presence.record_count; i++) {
        if (strncmp(g_local_presence.records[i].key, key, PRESENCE_KEY_LEN) == 0) {
            g_local_presence.records[i] = g_local_presence.records[g_local_presence.record_count - 1];
            g_local_presence.record_count--;
            return;
        }
    }
}

EOS_DECLARE_FUNC(void) EOS_Presence_SetPresence(EOS_HPresence Handle, const EOS_Presence_SetPresenceOptions* Options, void* ClientData, const EOS_Presence_SetPresenceCompleteCallback CompletionDelegate) {
    PlatformState* p = (PlatformState*)Handle;
    EOS_Presence_SetPresenceCallbackInfo info = {0};
    info.ResultCode = EOS_InvalidParameters;
    info.RichPresenceResultCode = EOS_Success;
    info.ClientData = ClientData;
    info.LocalUserId = Options ? Options->LocalUserId : NULL;

    PresenceModHandle* mod = Options ? (PresenceModHandle*)Options->PresenceModificationHandle : NULL;
    if (mod && mod->magic == 0x504D4F44) {
        if (mod->has_join_info) {
            strncpy(g_local_presence.join_info, mod->join_info, sizeof(g_local_presence.join_info) - 1);
            g_local_presence.join_info[sizeof(g_local_presence.join_info) - 1] = '\0';
        }
        if (mod->has_rich_text) {
            strncpy(g_local_presence.rich_text, mod->rich_text, sizeof(g_local_presence.rich_text) - 1);
            g_local_presence.rich_text[sizeof(g_local_presence.rich_text) - 1] = '\0';
        }
        // MERGE the modification's records into existing presence (real EOS
        // semantics): SetData updates/adds named keys, DeleteData removes named
        // keys, and any key NOT mentioned PERSISTS. The old code replaced the
        // whole set, dropping e.g. Presence_Joinability_Custom on partial
        // updates so the joiner saw the host flip to non-joinable mid-session.
        for (int i = 0; i < mod->deleted_count; i++) {
            local_presence_delete(mod->deleted_keys[i]);
        }
        for (int i = 0; i < mod->record_count; i++) {
            local_presence_set(mod->records[i].key, mod->records[i].value);
        }
        info.ResultCode = EOS_Success;
        EOS_LOG_INFO(">>> EOS_Presence_SetPresence merged: join_info='%s', %d record(s) total",
                     g_local_presence.join_info, g_local_presence.record_count);
    } else {
        EOS_LOG_WARN("EOS_Presence_SetPresence: bad modification handle");
    }

    if (CompletionDelegate && p && p->callbacks) {
        callback_queue_push(p->callbacks, (void*)CompletionDelegate, &info, sizeof(info));
    }
}

EOS_DECLARE_FUNC(void) EOS_PresenceModification_Release(EOS_HPresenceModification Handle) {
    PresenceModHandle* mod = (PresenceModHandle*)Handle;
    if (mod && mod->magic == 0x504D4F44) {
        mod->magic = 0;
        free(mod);
    }
}

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

static PeerFriend* find_or_add_peer(const char* epic_id_str) {
    for (int i = 0; i < g_peer_count; i++) {
        if (strncmp(g_peers[i].epic.id_string, epic_id_str, EPIC_ACCOUNT_ID_LENGTH) == 0) {
            return &g_peers[i];
        }
    }
    if (g_peer_count >= MAX_DISCOVERED_SESSIONS) return NULL;
    PeerFriend* pf = &g_peers[g_peer_count++];
    memset(pf, 0, sizeof(*pf));
    pf->epic.magic = EAID_MAGIC;
    strncpy(pf->epic.id_string, epic_id_str, EPIC_ACCOUNT_ID_LENGTH);
    pf->epic.id_string[EPIC_ACCOUNT_ID_LENGTH] = '\0';
    pf->valid = 1;
    return pf;
}

// Rebuild the peer registry from user beacons + the sessions discovery cache.
// Beacons announce a user's existence/presence even when they host no EOS
// session (e.g. a Steam/Goldberg session) — required for the host to show up
// in friends-driven join UIs at all.
static void refresh_peers(PlatformState* p) {
    g_peer_count = 0;
    if (!p || !p->sessions) return;
    SessionsState* s = p->sessions;
    const char* self = local_product_id_string(p);
    const char* self_epic = g_auth_state.account_id.id_string;

    // User beacons first: identity, display name, live presence.
    if (s->discovery) {
        int user_count = 0;
        UserBeacon* users = discovery_get_users(s->discovery, &user_count);
        for (int i = 0; i < user_count; i++) {
            UserBeacon* ub = &users[i];
            if (!ub->valid) continue;
            if (self_epic && strncmp(ub->epic_id, self_epic, EPIC_ACCOUNT_ID_LENGTH) == 0) continue;
            if (self && strncmp(ub->puid, self, OWNER_ID_STRING_LEN) == 0) continue;
            PeerFriend* pf = find_or_add_peer(ub->epic_id);
            if (!pf) break;
            strncpy(pf->puid, ub->puid, sizeof(pf->puid) - 1);
            pf->puid[sizeof(pf->puid) - 1] = '\0';
            strncpy(pf->display_name, ub->display_name, sizeof(pf->display_name) - 1);
            pf->display_name[sizeof(pf->display_name) - 1] = '\0';
            strncpy(pf->join_info, ub->join_info, sizeof(pf->join_info) - 1);
            pf->join_info[sizeof(pf->join_info) - 1] = '\0';
            memcpy(pf->records, ub->records, sizeof(pf->records));
            pf->record_count = ub->record_count;
        }
    }

    // Discovered EOS sessions: add session id + host address (and presence
    // relay as fallback when no beacon was seen for the owner).
    for (int i = 0; i < s->discovered_session_count; i++) {
        Session* ds = &s->discovered_sessions[i];
        if (self && strncmp(ds->owner_id_string, self, OWNER_ID_STRING_LEN) == 0) {
            continue;  // skip our own session
        }
        // Dedupe against beacon-derived peers by ProductUserId; only fall back
        // to a synthetic EpicAccountId (= the puid string) when no beacon seen.
        PeerFriend* pf = NULL;
        for (int j = 0; j < g_peer_count; j++) {
            if (strncmp(g_peers[j].puid, ds->owner_id_string, OWNER_ID_STRING_LEN) == 0) {
                pf = &g_peers[j];
                break;
            }
        }
        if (!pf) pf = find_or_add_peer(ds->owner_id_string);
        if (!pf) break;
        if (pf->puid[0] == '\0') {
            strncpy(pf->puid, ds->owner_id_string, sizeof(pf->puid) - 1);
            pf->puid[sizeof(pf->puid) - 1] = '\0';
        }
        strncpy(pf->session_id, ds->session_id, SESSION_ID_LEN);
        pf->session_id[SESSION_ID_LEN] = '\0';
        strncpy(pf->host_address, ds->host_address, HOST_ADDRESS_LEN - 1);
        pf->host_address[HOST_ADDRESS_LEN - 1] = '\0';
        if (pf->join_info[0] == '\0') {
            strncpy(pf->join_info, ds->join_info, sizeof(pf->join_info) - 1);
            pf->join_info[sizeof(pf->join_info) - 1] = '\0';
        }
        if (pf->record_count == 0) {
            memcpy(pf->records, ds->presence_records, sizeof(pf->records));
            pf->record_count = ds->presence_record_count;
        }
    }

    // refresh_peers runs every Platform_Tick once notifications are registered;
    // log only on change or the file grows by megabytes per minute.
    static int last_logged_count = -1;
    if (g_peer_count != last_logged_count) {
        EOS_LOG_INFO("social_bridge: %d peer friend(s) from discovery", g_peer_count);
        last_logged_count = g_peer_count;
    }
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

// Map an external (Epic) account id string -> the matching ProductUserId.
// Epic account ids (auth) and ProductUserIds (connect) are DISTINCT in EOS; the
// game resolves a user's identity by this mapping during an EOS join. For the
// local user it's our own Connect id; for a peer it's the puid carried in the
// peer's LAN beacon (epic_id+puid). Returning the right puid (matching the
// session owner) clears the join's UserKeyHandle:-1 / E011.
EOS_ProductUserId social_bridge_resolve_puid(PlatformState* p, const char* epic_id) {
    if (!p || !epic_id) return NULL;
    // Local user (the joiner resolving itself).
    if (g_auth_state.logged_in &&
        strncmp(epic_id, g_auth_state.account_id.id_string, EPIC_ACCOUNT_ID_LENGTH) == 0) {
        const char* puid = local_product_id_string(p);
        if (puid) {
            return EOS_ProductUserId_FromString(puid);
        }
    }
    // Discovered peer (the host) — puid relayed in its beacon.
    refresh_peers(p);
    for (int i = 0; i < g_peer_count; i++) {
        if (g_peers[i].valid &&
            strncmp(g_peers[i].epic.id_string, epic_id, EPIC_ACCOUNT_ID_LENGTH) == 0 &&
            g_peers[i].puid[0] != '\0') {
            return EOS_ProductUserId_FromString(g_peers[i].puid);
        }
    }
    return NULL;
}

// Reverse mapping: ProductUserId string -> EpicAccountId string. The game calls
// EOS_Connect_GetProductUserIdMapping on a session OWNER's puid to rebuild that
// user's Epic identity during a join; without this it can't form the host user
// (UserKeyHandle:-1 / E011).
const char* social_bridge_resolve_epic_by_puid(PlatformState* p, const char* puid) {
    if (!p || !puid || !puid[0]) return NULL;
    // Local user.
    const char* self = local_product_id_string(p);
    if (self && g_auth_state.logged_in &&
        strncmp(puid, self, PRODUCT_USER_ID_LENGTH) == 0) {
        return g_auth_state.account_id.id_string;
    }
    // Discovered peer — its epic+puid pair came in over the LAN beacon.
    refresh_peers(p);
    for (int i = 0; i < g_peer_count; i++) {
        if (g_peers[i].valid && g_peers[i].puid[0] &&
            strncmp(g_peers[i].puid, puid, PRODUCT_USER_ID_LENGTH) == 0 &&
            g_peers[i].epic.id_string[0]) {
            return g_peers[i].epic.id_string;
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

typedef struct {
    char epic_id[EPIC_ACCOUNT_ID_LENGTH + 1];
    uint32_t presence_fp;  // fingerprint of join_info + records
} NotifiedPeer;

static NotifiedPeer g_notified[MAX_DISCOVERED_SESSIONS];
static int g_notified_count = 0;

// FNV-1a over the peer's presence payload — change detection so we can re-fire
// OnPresenceChanged when an EXISTING peer starts/stops hosting (its records or
// join_info change). Games cache presence and only re-query on notification.
static uint32_t presence_fingerprint(const PeerFriend* pf) {
    uint32_t h = 2166136261u;
    const char* ji = pf->join_info;
    while (*ji) { h ^= (uint8_t)*ji++; h *= 16777619u; }
    for (int i = 0; i < pf->record_count; i++) {
        const char* k = pf->records[i].key;
        while (*k) { h ^= (uint8_t)*k++; h *= 16777619u; }
        const char* v = pf->records[i].value;
        while (*v) { h ^= (uint8_t)*v++; h *= 16777619u; }
    }
    return h;
}

static NotifiedPeer* find_notified(const char* epic_id) {
    for (int i = 0; i < g_notified_count; i++) {
        if (strncmp(g_notified[i].epic_id, epic_id, EPIC_ACCOUNT_ID_LENGTH) == 0) return &g_notified[i];
    }
    return NULL;
}

// Broadcast our own user beacon (existence + presence) every ~2s, regardless
// of whether we host a session — peers' join UIs need to see us either way.
static void broadcast_own_beacon(PlatformState* p) {
    if (!p || !p->sessions || !p->sessions->discovery) return;
    if (!g_auth_state.logged_in) return;
    static uint64_t last_beacon_ms = 0;
    uint64_t now = get_time_ms();
    if (now - last_beacon_ms < 2000) return;
    last_beacon_ms = now;

    UserBeacon ub;
    memset(&ub, 0, sizeof(ub));
    strncpy(ub.epic_id, g_auth_state.account_id.id_string, 32);
    const char* puid = local_product_id_string(p);
    if (puid) strncpy(ub.puid, puid, 32);
    strncpy(ub.display_name, g_auth_state.display_name, sizeof(ub.display_name) - 1);
    strncpy(ub.join_info, g_local_presence.join_info, sizeof(ub.join_info) - 1);
    memcpy(ub.records, g_local_presence.records, sizeof(ub.records));
    ub.record_count = g_local_presence.record_count;
    ub.valid = true;
    discovery_broadcast_user(p->sessions->discovery, &ub);
}

void social_bridge_tick(PlatformState* p) {
    if (!p || !p->callbacks) return;
    broadcast_own_beacon(p);
    if (!g_friends_cb && !g_presence_cb) return;  // nobody listening yet
    // Do NOT fire peer friend/presence notifications until our OWN local user is
    // logged in. A peer can be discovered a tick BEFORE the game's Auth login
    // completes (the joiner discovers the host before its own login); firing then
    // sends a friends-update whose LocalUserId is the pre-login placeholder, which
    // the game rejects ("FSocialEOS::HandleFriendsUpdate LocalUserId= not found")
    // AND we mark the peer notified, so the friends-update (is_new only) NEVER
    // re-fires -> the host never registers as an online user -> the EOS join dies
    // with UserKeyHandle:-1. Waiting for login means the peer is still is_new on
    // the next tick and fires once with the real local EpicAccountId.
    if (!g_auth_state.logged_in) return;
    refresh_peers(p);
    for (int i = 0; i < g_peer_count; i++) {
        PeerFriend* pf = &g_peers[i];
        NotifiedPeer* np = find_notified(pf->epic.id_string);
        int is_new = (np == NULL);

        // The game runs FetchFriendInfoAsync EXACTLY ONCE — on the
        // NotFriends->Friends transition — and never retries. (Proven from the
        // joiner log: a later Friends->Friends "presence change" notify is a
        // no-op to the game; it does NOT re-fetch, so growing the friend's
        // presence after the fact never repairs it.) If we add the friend while
        // it's still a bare beacon record — host not hosting yet: no session id,
        // sparse presence — that one-shot fetch runs against empty data AND
        // collides with the game's login-time QueryBlockedAll storm, fails
        // ("Could not FetchFriendInfoAsync - Task Failed"), and the host's user
        // record is never built -> the session join then hits UserKeyHandle:-1
        // / E011 forever. So HOLD the friend's first appearance until the host
        // actually has a joinable session; the single fetch then runs against
        // complete data (session + presence + display name) in a clean
        // post-login window, succeeds, and the host resolves as an online user.
        if (is_new && pf->session_id[0] == '\0') continue;

        uint32_t fp = presence_fingerprint(pf);
        if (!is_new && np->presence_fp == fp) continue;  // nothing changed

        EOS_EpicAccountId local = (EOS_EpicAccountId)&g_auth_state.account_id;
        // Interned handle (see GetFriendAtIndex) so the peer id we notify under is
        // pointer-identical to what UE registers/looks up in its account registry.
        EOS_EpicAccountId peer = EOS_EpicAccountId_FromString(pf->epic.id_string);
        if (!peer) peer = (EOS_EpicAccountId)&pf->epic;
        if (g_friends_cb) {
            EOS_Friends_OnFriendsUpdateInfo info = {0};
            info.ClientData = g_friends_cd;
            info.LocalUserId = local;
            info.TargetUserId = peer;
            info.PreviousStatus = is_new ? EOS_FS_NotFriends : EOS_FS_Friends;
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
        if (is_new) {
            if (g_notified_count < MAX_DISCOVERED_SESSIONS) {
                np = &g_notified[g_notified_count++];
                strncpy(np->epic_id, pf->epic.id_string, EPIC_ACCOUNT_ID_LENGTH);
                np->epic_id[EPIC_ACCOUNT_ID_LENGTH] = '\0';
                np->presence_fp = fp;
            }
            EOS_LOG_INFO("social_bridge: fired friends/presence update for new peer '%s'", pf->display_name);
        } else {
            np->presence_fp = fp;
            EOS_LOG_INFO("social_bridge: fired presence CHANGE for '%s' (records=%d, join_info=%s)",
                         pf->display_name, pf->record_count,
                         pf->join_info[0] ? "set" : "empty");
        }
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
    // Return the INTERNED EpicAccountId handle — the SAME pointer
    // EOS_EpicAccountId_FromString hands out for this id. UE's
    // FOnlineAccountIdRegistryEOS keys accounts on the raw handle POINTER, so the
    // friend UE registers here (QueryFriends -> ResolveEpicIds) must be the same
    // handle any later FromString-based lookup of the host uses; returning a
    // per-peer-struct pointer instead left the host unfindable (UserKeyHandle:-1).
    return EOS_EpicAccountId_FromString(g_peers[Options->Index].epic.id_string);
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
    // Single allocation: Info first (so Info_Release can free the whole block),
    // followed by the records array + stable string storage.
    typedef struct {
        EOS_Presence_Info info;
        EOS_Presence_DataRecord recs[MAX_PRESENCE_RECORDS];
        char keys[MAX_PRESENCE_RECORDS][PRESENCE_KEY_LEN];
        char vals[MAX_PRESENCE_RECORDS][PRESENCE_VALUE_LEN];
    } PresenceInfoBlock;
    PresenceInfoBlock* blk = calloc(1, sizeof(PresenceInfoBlock));
    if (!blk) return EOS_UnexpectedError;
    EOS_Presence_Info* info = &blk->info;
    info->ApiVersion = EOS_PRESENCE_INFO_API_LATEST;
    info->Status = EOS_PS_Online;
    info->UserId = target;
    info->ProductId = "Satisfactory";
    info->ProductVersion = "1.0";
    info->Platform = "EOSLAN";
    info->RichText = "Hosting a game";
    info->ProductName = "Satisfactory";
    info->IntegratedPlatform = "";
    // Relay the host's REAL presence data records (from the LAN announce).
    int n = (pf->record_count > MAX_PRESENCE_RECORDS) ? MAX_PRESENCE_RECORDS : pf->record_count;
    for (int i = 0; i < n; i++) {
        memcpy(blk->keys[i], pf->records[i].key, PRESENCE_KEY_LEN);
        memcpy(blk->vals[i], pf->records[i].value, PRESENCE_VALUE_LEN);
        blk->recs[i].ApiVersion = EOS_PRESENCE_DATARECORD_API_LATEST;
        blk->recs[i].Key = blk->keys[i];
        blk->recs[i].Value = blk->vals[i];
    }
    info->RecordsCount = n;
    info->Records = (n > 0) ? blk->recs : NULL;
    *OutPresence = info;
    EOS_LOG_INFO(">>> EOS_Presence_CopyPresence: Online presence for peer, %d record(s)", n);
    return EOS_Success;
}

EOS_DECLARE_FUNC(void) EOS_Presence_Info_Release(EOS_Presence_Info* PresenceInfo) {
    if (PresenceInfo) free(PresenceInfo);  // Info is first member of the block
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
    // Prefer the host's REAL join-info string (relayed over the LAN announce) —
    // the game defines its own format and parses it back on the joiner. Fall
    // back to the raw session id only if the host never published join info.
    const char* ji = (pf->join_info[0] != '\0') ? pf->join_info : pf->session_id;
    int need = (int)strlen(ji) + 1;
    if (*InOutBufferLength < need) {
        *InOutBufferLength = need;
        return EOS_LimitExceeded;
    }
    strcpy(OutBuffer, ji);
    *InOutBufferLength = need;
    EOS_LOG_INFO(">>> EOS_Presence_GetJoinInfo: '%s'%s", ji,
                 (ji == pf->session_id) ? " (fallback: raw session id)" : "");
    return EOS_Success;
}

// ===================== UserInfo =====================
// UE's friends pipeline queries user info for every friend and BLOCKS the
// friend entry on the completion callback — a stub that never completes left
// the join UI permanently empty. Names come from user beacons for peers and
// from auth state (EOSLAN_USERNAME) for the local user.

// Display name for any known EpicAccountId (local user, peer, or fallback).
const char* social_bridge_display_name_for(EOS_EpicAccountId id) {
    if (id) {
        EOS_EpicAccountIdDetails* d = (EOS_EpicAccountIdDetails*)id;
        if (g_auth_state.logged_in &&
            strncmp(d->id_string, g_auth_state.account_id.id_string, EPIC_ACCOUNT_ID_LENGTH) == 0) {
            return g_auth_state.display_name;
        }
        PeerFriend* pf = find_peer_by_epic(id);
        if (pf && pf->display_name[0] != '\0') {
            return pf->display_name;
        }
    }
    return "LAN_Player";
}

EOS_DECLARE_FUNC(void) EOS_UserInfo_QueryUserInfo(EOS_HUserInfo Handle, const EOS_UserInfo_QueryUserInfoOptions* Options, void* ClientData, const EOS_UserInfo_OnQueryUserInfoCallback CompletionDelegate) {
    PlatformState* p = (PlatformState*)Handle;
    EOS_UserInfo_QueryUserInfoCallbackInfo info = {0};
    info.ResultCode = EOS_Success;
    info.ClientData = ClientData;
    info.LocalUserId = Options ? Options->LocalUserId : NULL;
    info.TargetUserId = Options ? Options->TargetUserId : NULL;
    EOS_LOG_INFO(">>> EOS_UserInfo_QueryUserInfo: completing with Success");
    if (CompletionDelegate && p && p->callbacks) {
        callback_queue_push(p->callbacks, (void*)CompletionDelegate, &info, sizeof(info));
    }
}

EOS_DECLARE_FUNC(void) EOS_UserInfo_QueryUserInfoByDisplayName(EOS_HUserInfo Handle, const EOS_UserInfo_QueryUserInfoByDisplayNameOptions* Options, void* ClientData, const EOS_UserInfo_OnQueryUserInfoByDisplayNameCallback CompletionDelegate) {
    PlatformState* p = (PlatformState*)Handle;
    EOS_UserInfo_QueryUserInfoByDisplayNameCallbackInfo info = {0};
    info.ResultCode = EOS_NotFound;
    info.ClientData = ClientData;
    info.LocalUserId = Options ? Options->LocalUserId : NULL;
    EOS_LOG_INFO(">>> EOS_UserInfo_QueryUserInfoByDisplayName: completing with NotFound");
    if (CompletionDelegate && p && p->callbacks) {
        callback_queue_push(p->callbacks, (void*)CompletionDelegate, &info, sizeof(info));
    }
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_UserInfo_CopyUserInfo(EOS_HUserInfo Handle, const EOS_UserInfo_CopyUserInfoOptions* Options, EOS_UserInfo** OutUserInfo) {
    (void)Handle;
    if (!OutUserInfo) return EOS_InvalidParameters;
    *OutUserInfo = NULL;
    EOS_EpicAccountId target = Options ? Options->TargetUserId : NULL;
    if (!target) return EOS_InvalidParameters;

    // Single allocation: struct + embedded name storage; Release frees it all.
    typedef struct {
        EOS_UserInfo info;
        char name[PEER_DISPLAY_NAME_LEN];
    } UserInfoBlock;
    UserInfoBlock* blk = calloc(1, sizeof(UserInfoBlock));
    if (!blk) return EOS_UnexpectedError;
    strncpy(blk->name, social_bridge_display_name_for(target), sizeof(blk->name) - 1);
    blk->info.ApiVersion = EOS_USERINFO_COPYUSERINFO_API_LATEST;
    blk->info.UserId = target;
    blk->info.Country = "US";
    blk->info.DisplayName = blk->name;
    blk->info.PreferredLanguage = "en";
    blk->info.Nickname = NULL;
    blk->info.DisplayNameSanitized = blk->name;
    *OutUserInfo = &blk->info;
    EOS_LOG_INFO(">>> EOS_UserInfo_CopyUserInfo -> '%s'", blk->name);
    return EOS_Success;
}

EOS_DECLARE_FUNC(void) EOS_UserInfo_Release(EOS_UserInfo* UserInfo) {
    if (UserInfo) free(UserInfo);  // info is first member of the block
}
