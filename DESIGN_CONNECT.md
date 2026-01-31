# Connect Layer Implementation Guide

## Scope
You are implementing the **Connect Layer** - fake authentication for LAN multiplayer.

**Your responsibility:**
- Fake login that generates Product User IDs
- User ID validation and string conversion
- Login status tracking
- Multiple local user support (for split-screen)

**NOT your responsibility:**
- Real authentication (no network calls to Epic)
- Epic Account IDs (EOS_Auth interface, not needed for most games)
- Callback queue (use platform's callback system)

---

## File Locations

**You will create:**
- `src/connect.c` - Implementation
- `src/connect_internal.h` - Internal structures

**You will use:**
- `include/eos/eos_connect.h` - Official EOS headers
- `include/eos/eos_connect_types.h` - Type definitions
- `src/callbacks.h` - For queuing callbacks
- `src/platform_internal.h` - Platform state access

---

## Key Insight

Most games use **EOS_Connect_Login** (Product User ID) rather than **EOS_Auth_Login** (Epic Account ID). The Connect interface is for cross-platform identity that doesn't require an Epic account.

For LAN, we fake this entirely - generate deterministic user IDs that are unique per instance.

---

## API Functions to Implement

### Core Authentication

```c
/**
 * Login a user to get a Product User ID.
 *
 * For LAN emulator:
 * - Ignore the credential type and token
 * - Generate a deterministic ProductUserId based on config
 * - Queue success callback
 *
 * @param Handle Connect interface handle
 * @param Options Login options (we mostly ignore these)
 * @param ClientData Passed to callback
 * @param CompletionDelegate Called when login completes
 */
void EOS_Connect_Login(
    EOS_HConnect Handle,
    const EOS_Connect_LoginOptions* Options,
    void* ClientData,
    const EOS_Connect_OnLoginCallback CompletionDelegate
);

/**
 * Create a new user linked to the provided credentials.
 * For LAN: Same as Login, just creates user ID.
 */
void EOS_Connect_CreateUser(
    EOS_HConnect Handle,
    const EOS_Connect_CreateUserOptions* Options,
    void* ClientData,
    const EOS_Connect_OnCreateUserCallback CompletionDelegate
);

/**
 * Logout a user.
 * For LAN: Mark user as logged out, queue callback.
 */
void EOS_Connect_Logout(
    EOS_HConnect Handle,
    const EOS_Connect_LogoutOptions* Options,
    void* ClientData,
    const EOS_Connect_OnLogoutCallback CompletionDelegate
);
```

### User Query Functions

```c
/**
 * Get count of logged in users.
 */
int32_t EOS_Connect_GetLoggedInUsersCount(EOS_HConnect Handle);

/**
 * Get logged in user by index.
 * Returns NULL if index out of range.
 */
EOS_ProductUserId EOS_Connect_GetLoggedInUserByIndex(EOS_HConnect Handle, int32_t Index);

/**
 * Get login status of a user.
 */
EOS_ELoginStatus EOS_Connect_GetLoginStatus(EOS_HConnect Handle, EOS_ProductUserId LocalUserId);
```

### Product User ID Functions

```c
/**
 * Check if a ProductUserId is valid.
 * Valid means: non-NULL and properly formatted.
 */
EOS_Bool EOS_ProductUserId_IsValid(EOS_ProductUserId AccountId);

/**
 * Convert ProductUserId to string.
 *
 * Format: 32 hex characters (e.g., "0123456789ABCDEF0123456789ABCDEF")
 */
EOS_EResult EOS_ProductUserId_ToString(
    EOS_ProductUserId AccountId,
    char* OutBuffer,
    int32_t* InOutBufferLength
);

/**
 * Create ProductUserId from string.
 */
EOS_ProductUserId EOS_ProductUserId_FromString(const char* ProductUserIdString);
```

### Notification Functions

```c
/**
 * Register for login status change notifications.
 */
EOS_NotificationId EOS_Connect_AddNotifyLoginStatusChanged(
    EOS_HConnect Handle,
    const EOS_Connect_AddNotifyLoginStatusChangedOptions* Options,
    void* ClientData,
    const EOS_Connect_OnLoginStatusChangedCallback Notification
);

void EOS_Connect_RemoveNotifyLoginStatusChanged(
    EOS_HConnect Handle,
    EOS_NotificationId InId
);

/**
 * Register for auth expiration notifications.
 * For LAN: Never fires (auth never expires).
 */
EOS_NotificationId EOS_Connect_AddNotifyAuthExpiration(
    EOS_HConnect Handle,
    const EOS_Connect_AddNotifyAuthExpirationOptions* Options,
    void* ClientData,
    const EOS_Connect_OnAuthExpirationCallback Notification
);

void EOS_Connect_RemoveNotifyAuthExpiration(
    EOS_HConnect Handle,
    EOS_NotificationId InId
);
```

### Other Functions (Stubs)

```c
// These can return appropriate "not supported" or stub values:
void EOS_Connect_LinkAccount(...);           // Queue callback with success
void EOS_Connect_UnlinkAccount(...);         // Queue callback with success
void EOS_Connect_CreateDeviceId(...);        // Queue callback with success
void EOS_Connect_DeleteDeviceId(...);        // Queue callback with success
void EOS_Connect_TransferDeviceIdAccount(...); // Queue callback with success
void EOS_Connect_QueryExternalAccountMappings(...); // Queue callback, empty results
void EOS_Connect_QueryProductUserIdMappings(...);   // Queue callback, empty results
EOS_EResult EOS_Connect_GetExternalAccountMapping(...); // Return EOS_NotFound
EOS_ProductUserId EOS_Connect_GetExternalAccountMappings(...); // Return NULL
EOS_EResult EOS_Connect_GetProductUserIdMapping(...); // Return EOS_NotFound
uint32_t EOS_Connect_GetProductUserExternalAccountCount(...); // Return 0
EOS_EResult EOS_Connect_CopyProductUserExternalAccountByIndex(...); // Return EOS_NotFound
EOS_EResult EOS_Connect_CopyProductUserExternalAccountByAccountType(...); // Return EOS_NotFound
EOS_EResult EOS_Connect_CopyProductUserExternalAccountByAccountId(...); // Return EOS_NotFound
EOS_EResult EOS_Connect_CopyProductUserInfo(...); // Return EOS_NotFound
EOS_EResult EOS_Connect_CopyIdToken(...); // Return EOS_NotFound
void EOS_Connect_VerifyIdToken(...); // Queue callback with success
void EOS_ProductUserIdDetails_Release(...); // No-op
void EOS_Connect_ExternalAccountInfo_Release(...); // No-op
void EOS_Connect_IdToken_Release(...); // No-op
```

---

## Data Structures

### connect_internal.h

```c
#ifndef EOS_LAN_CONNECT_INTERNAL_H
#define EOS_LAN_CONNECT_INTERNAL_H

#include "eos_connect_types.h"
#include "platform_internal.h"
#include <stdbool.h>

#define MAX_LOCAL_USERS 4
#define PRODUCT_USER_ID_LENGTH 32

// Internal representation of a ProductUserId
typedef struct EOS_ProductUserIdDetails {
    uint32_t magic;          // 0x50554944 = "PUID"
    char id_string[PRODUCT_USER_ID_LENGTH + 1];  // 32 hex chars + null
} EOS_ProductUserIdDetails;

typedef struct {
    EOS_ProductUserIdDetails user_id;
    EOS_ELoginStatus status;
    bool in_use;
} LocalUser;

typedef struct {
    // Notification handler
    EOS_Connect_OnLoginStatusChangedCallback callback;
    void* client_data;
    EOS_NotificationId id;
    bool active;
} LoginStatusNotification;

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
    LoginStatusNotification login_notifications[8];
    int login_notification_count;
    EOS_NotificationId next_notification_id;

} ConnectState;

// Creation/destruction (called by platform)
ConnectState* connect_create(PlatformState* platform);
void connect_destroy(ConnectState* state);

// Internal helpers
EOS_ProductUserId connect_generate_user_id(ConnectState* state, int user_index);
bool connect_validate_user_id(EOS_ProductUserId id);

#endif // EOS_LAN_CONNECT_INTERNAL_H
```

---

## User ID Generation

### Format
```
LLLLLLLLLLLLLLLL IIII XXXX
│                │    │
│                │    └── Random component (4 hex = 16 bits)
│                └─────── User index (4 hex = 16 bits, 0000-0003)
└──────────────────────── Instance ID (16 hex = 64 bits)

Total: 24 chars fixed + padding to 32 = LLLLLLLLLLLLLLLL IIII XXXX 0000
```

### Generation Algorithm
```c
EOS_ProductUserId connect_generate_user_id(ConnectState* state, int user_index) {
    EOS_ProductUserIdDetails* id = calloc(1, sizeof(EOS_ProductUserIdDetails));
    if (!id) return NULL;

    id->magic = 0x50554944;

    // Format: {instance_id}{user_index:04x}{random:04x}{padding}
    uint16_t random_part = (uint16_t)(rand() & 0xFFFF);

    snprintf(id->id_string, sizeof(id->id_string),
             "%s%04X%04X00000000",
             state->instance_id,
             (uint16_t)user_index,
             random_part);

    return (EOS_ProductUserId)id;
}
```

### Instance ID from Config
```c
void connect_init_instance_id(ConnectState* state) {
    // Try to read from config file
    const char* config_id = config_get_string("identity", "instance_id", NULL);

    if (config_id && strlen(config_id) == 16) {
        strncpy(state->instance_id, config_id, 16);
        state->instance_id[16] = '\0';
    } else {
        // Generate random instance ID
        for (int i = 0; i < 16; i++) {
            state->instance_id[i] = "0123456789ABCDEF"[rand() % 16];
        }
        state->instance_id[16] = '\0';
    }
}
```

---

## Implementation Details

### EOS_Connect_Login
```c
void EOS_Connect_Login(
    EOS_HConnect Handle,
    const EOS_Connect_LoginOptions* Options,
    void* ClientData,
    const EOS_Connect_OnLoginCallback CompletionDelegate
) {
    ConnectState* state = (ConnectState*)Handle;
    if (!state || state->magic != 0x434F4E4E) {
        // Invalid handle - can't even queue callback
        return;
    }

    // Find free user slot
    int slot = -1;
    for (int i = 0; i < MAX_LOCAL_USERS; i++) {
        if (!state->users[i].in_use) {
            slot = i;
            break;
        }
    }

    EOS_Connect_LoginCallbackInfo info = {0};
    info.ClientData = ClientData;

    if (slot < 0) {
        // No free slots
        info.ResultCode = EOS_LimitExceeded;
        info.LocalUserId = NULL;
    } else {
        // Generate user ID
        EOS_ProductUserId user_id = connect_generate_user_id(state, slot);

        state->users[slot].user_id = *(EOS_ProductUserIdDetails*)user_id;
        state->users[slot].status = EOS_LS_LoggedIn;
        state->users[slot].in_use = true;
        state->user_count++;

        info.ResultCode = EOS_Success;
        info.LocalUserId = (EOS_ProductUserId)&state->users[slot].user_id;

        // Fire login status notifications
        for (int i = 0; i < state->login_notification_count; i++) {
            if (state->login_notifications[i].active) {
                EOS_Connect_LoginStatusChangedCallbackInfo notif = {0};
                notif.ClientData = state->login_notifications[i].client_data;
                notif.LocalUserId = info.LocalUserId;
                notif.CurrentStatus = EOS_LS_LoggedIn;
                notif.PreviousStatus = EOS_LS_NotLoggedIn;

                callback_queue_push(
                    state->platform->callbacks,
                    (void*)state->login_notifications[i].callback,
                    &notif,
                    sizeof(notif)
                );
            }
        }
    }

    // Queue completion callback
    if (CompletionDelegate) {
        callback_queue_push(
            state->platform->callbacks,
            (void*)CompletionDelegate,
            &info,
            sizeof(info)
        );
    }
}
```

### EOS_ProductUserId_IsValid
```c
EOS_Bool EOS_ProductUserId_IsValid(EOS_ProductUserId AccountId) {
    if (AccountId == NULL) return EOS_FALSE;

    EOS_ProductUserIdDetails* id = (EOS_ProductUserIdDetails*)AccountId;
    if (id->magic != 0x50554944) return EOS_FALSE;

    // Check string is valid hex, correct length
    if (strlen(id->id_string) != PRODUCT_USER_ID_LENGTH) return EOS_FALSE;

    for (int i = 0; i < PRODUCT_USER_ID_LENGTH; i++) {
        char c = id->id_string[i];
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
            return EOS_FALSE;
        }
    }

    return EOS_TRUE;
}
```

### EOS_ProductUserId_ToString
```c
EOS_EResult EOS_ProductUserId_ToString(
    EOS_ProductUserId AccountId,
    char* OutBuffer,
    int32_t* InOutBufferLength
) {
    if (!OutBuffer || !InOutBufferLength) {
        return EOS_InvalidParameters;
    }

    if (!EOS_ProductUserId_IsValid(AccountId)) {
        return EOS_InvalidUser;
    }

    EOS_ProductUserIdDetails* id = (EOS_ProductUserIdDetails*)AccountId;
    int required = PRODUCT_USER_ID_LENGTH + 1;  // +1 for null

    if (*InOutBufferLength < required) {
        *InOutBufferLength = required;
        return EOS_LimitExceeded;
    }

    strcpy(OutBuffer, id->id_string);
    *InOutBufferLength = required;
    return EOS_Success;
}
```

### EOS_ProductUserId_FromString
```c
EOS_ProductUserId EOS_ProductUserId_FromString(const char* ProductUserIdString) {
    if (!ProductUserIdString) return NULL;
    if (strlen(ProductUserIdString) != PRODUCT_USER_ID_LENGTH) return NULL;

    // Validate hex string
    for (int i = 0; i < PRODUCT_USER_ID_LENGTH; i++) {
        char c = ProductUserIdString[i];
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
            return NULL;
        }
    }

    EOS_ProductUserIdDetails* id = calloc(1, sizeof(EOS_ProductUserIdDetails));
    if (!id) return NULL;

    id->magic = 0x50554944;
    strncpy(id->id_string, ProductUserIdString, PRODUCT_USER_ID_LENGTH);
    id->id_string[PRODUCT_USER_ID_LENGTH] = '\0';

    // Uppercase for consistency
    for (int i = 0; i < PRODUCT_USER_ID_LENGTH; i++) {
        if (id->id_string[i] >= 'a' && id->id_string[i] <= 'f') {
            id->id_string[i] -= 32;
        }
    }

    return (EOS_ProductUserId)id;
}
```

---

## Integration Points

### What You Provide
```c
// Platform calls these
ConnectState* connect_create(PlatformState* platform);
void connect_destroy(ConnectState* state);

// Sessions/P2P need to get local user ID
EOS_ProductUserId connect_get_local_user(ConnectState* state, int index);
```

### What You Need
```c
// From platform
PlatformState* platform;
platform->callbacks  // For queuing callbacks

// From callbacks module
void callback_queue_push(CallbackQueue* q, void* fn, const void* info, size_t size);
```

---

## Testing Criteria

1. **Login test:**
   ```c
   EOS_Connect_LoginOptions opts = { .ApiVersion = EOS_CONNECT_LOGIN_API_LATEST };
   EOS_ProductUserId logged_in_user = NULL;

   void OnLogin(const EOS_Connect_LoginCallbackInfo* Data) {
       assert(Data->ResultCode == EOS_Success);
       assert(EOS_ProductUserId_IsValid(Data->LocalUserId));
       logged_in_user = Data->LocalUserId;
   }

   EOS_Connect_Login(connect, &opts, NULL, OnLogin);
   EOS_Platform_Tick(platform);  // Process callback
   assert(logged_in_user != NULL);
   ```

2. **User ID string round-trip:**
   ```c
   char buffer[64];
   int32_t len = sizeof(buffer);
   assert(EOS_ProductUserId_ToString(logged_in_user, buffer, &len) == EOS_Success);
   assert(strlen(buffer) == 32);

   EOS_ProductUserId parsed = EOS_ProductUserId_FromString(buffer);
   assert(EOS_ProductUserId_IsValid(parsed));

   char buffer2[64];
   len = sizeof(buffer2);
   EOS_ProductUserId_ToString(parsed, buffer2, &len);
   assert(strcmp(buffer, buffer2) == 0);
   ```

3. **Multiple users (split-screen):**
   ```c
   for (int i = 0; i < 4; i++) {
       EOS_Connect_Login(connect, &opts, NULL, OnLogin);
   }
   // Process all callbacks
   for (int i = 0; i < 4; i++) EOS_Platform_Tick(platform);

   assert(EOS_Connect_GetLoggedInUsersCount(connect) == 4);

   // 5th login should fail
   EOS_Connect_Login(connect, &opts, NULL, OnLoginFail);
   EOS_Platform_Tick(platform);
   // OnLoginFail should get EOS_LimitExceeded
   ```

4. **Login status query:**
   ```c
   assert(EOS_Connect_GetLoginStatus(connect, logged_in_user) == EOS_LS_LoggedIn);

   EOS_ProductUserId fake = EOS_ProductUserId_FromString("00000000000000000000000000000000");
   assert(EOS_Connect_GetLoginStatus(connect, fake) == EOS_LS_NotLoggedIn);
   ```

---

## Build Instructions

```makefile
$(BUILD_DIR)/connect.o: src/connect.c src/connect_internal.h
	$(CC) $(CFLAGS) -I include -c $< -o $@
```

Dependencies:
- Standard C library
- EOS SDK headers
- callbacks.h
- platform_internal.h
