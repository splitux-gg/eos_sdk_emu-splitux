#ifndef EOS_LAN_CONNECT_INTERNAL_H
#define EOS_LAN_CONNECT_INTERNAL_H

#include "eos/eos_connect.h"
#include "eos/eos_connect_types.h"
#include "platform_internal.h"
#include <stdbool.h>

#define MAX_LOCAL_USERS 4
#define PRODUCT_USER_ID_LENGTH 32
#define MAX_LOGIN_NOTIFICATIONS 8
#define MAX_AUTH_EXPIRATION_NOTIFICATIONS 8

// Internal representation of a ProductUserId
typedef struct EOS_ProductUserIdDetails {
    uint32_t magic;          // 0x50554944 = "PUID"
    char id_string[PRODUCT_USER_ID_LENGTH + 1];  // 32 hex chars + null
} EOS_ProductUserIdDetails;

// Local user state
typedef struct {
    EOS_ProductUserIdDetails user_id;
    EOS_ELoginStatus status;
    bool in_use;
} LocalUser;

// Login status change notification handler
typedef struct {
    EOS_Connect_OnLoginStatusChangedCallback callback;
    void* client_data;
    EOS_NotificationId id;
    bool active;
} LoginStatusNotification;

// Auth expiration notification handler
typedef struct {
    EOS_Connect_OnAuthExpirationCallback callback;
    void* client_data;
    EOS_NotificationId id;
    bool active;
} AuthExpirationNotification;

// Connect state
typedef struct ConnectState {
    uint32_t magic;  // 0x434F4E4E = "CONN"

    // Back-reference to platform
    PlatformState* platform;

    // Instance identifier (for generating unique user IDs)
    char instance_id[17];  // 16 hex chars + null

    // Local logged-in users
    LocalUser users[MAX_LOCAL_USERS];
    int user_count;

    // Notification handlers
    LoginStatusNotification login_notifications[MAX_LOGIN_NOTIFICATIONS];
    int login_notification_count;
    AuthExpirationNotification auth_expiration_notifications[MAX_AUTH_EXPIRATION_NOTIFICATIONS];
    int auth_expiration_notification_count;
    EOS_NotificationId next_notification_id;

} ConnectState;

// Creation/destruction (called by platform)
ConnectState* connect_create(PlatformState* platform);
void connect_destroy(ConnectState* state);

// Internal helpers
EOS_ProductUserId connect_generate_user_id(ConnectState* state, int user_index);
bool connect_validate_user_id(EOS_ProductUserId id);
LocalUser* connect_find_user_by_id(ConnectState* state, EOS_ProductUserId id);
void connect_fire_login_status_notifications(ConnectState* state, EOS_ProductUserId user_id,
                                              EOS_ELoginStatus prev_status, EOS_ELoginStatus curr_status);

#endif // EOS_LAN_CONNECT_INTERNAL_H
