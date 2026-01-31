#ifndef EOS_LAN_AUTH_INTERNAL_H
#define EOS_LAN_AUTH_INTERNAL_H

#include "eos/eos_auth.h"
#include <stdbool.h>
#include <stdint.h>

#define EPIC_ACCOUNT_ID_LENGTH 32
#define MAX_AUTH_NOTIFICATIONS 8

// Magic number for EpicAccountId validation
#define EAID_MAGIC 0x45414944  // "EAID"

// Internal representation of an EpicAccountId
typedef struct EOS_EpicAccountIdDetails {
    uint32_t magic;
    char id_string[EPIC_ACCOUNT_ID_LENGTH + 1];  // 32 hex chars + null
} EOS_EpicAccountIdDetails;

// Storage for registered notification callbacks
typedef struct AuthNotificationEntry {
    EOS_NotificationId id;
    void* client_data;
    EOS_Auth_OnLoginStatusChangedCallback callback;
    bool in_use;
} AuthNotificationEntry;

// Auth state - stored globally since Auth interface uses platform handle directly
typedef struct AuthState {
    bool logged_in;
    bool steam_login_completed;  // Set when Connect_Login with Steam credentials succeeds
    EOS_EpicAccountIdDetails account_id;
    char display_name[64];
    AuthNotificationEntry login_status_notifications[MAX_AUTH_NOTIFICATIONS];
} AuthState;

// Global auth state (one per process, similar to how real EOS works)
extern AuthState g_auth_state;

// Initialize auth state
void auth_init(void);

// Auto-login for LAN play
void auth_auto_login(void);

// Fire all stored login status notifications
void auth_fire_login_notifications(EOS_ELoginStatus prev, EOS_ELoginStatus current);

#endif // EOS_LAN_AUTH_INTERNAL_H
