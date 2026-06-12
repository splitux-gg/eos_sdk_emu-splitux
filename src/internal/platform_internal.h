#ifndef EOS_PLATFORM_INTERNAL_H
#define EOS_PLATFORM_INTERNAL_H

#include <stdbool.h>
#include "eos/eos_sdk.h"
#include "eos/eos_connect_types.h"
#include "eos/eos_sessions_types.h"
#include "eos/eos_p2p_types.h"

// Forward declarations of internal state structures
typedef struct ConnectState ConnectState;
typedef struct SessionsState SessionsState;
typedef struct LobbyState LobbyState;
typedef struct P2PState P2PState;
typedef struct CallbackQueue CallbackQueue;

// Platform state
typedef struct PlatformState {
    bool initialized;
    EOS_Platform_Options options;
    EOS_ENetworkStatus network_status;
    EOS_EApplicationStatus app_status;

    // Subsystem handles
    ConnectState* connect;
    SessionsState* sessions;
    LobbyState* lobby;
    P2PState* p2p;

    // Callback queue
    CallbackQueue* callbacks;

    // LAN networking configuration (from environment variables)
    struct {
        uint16_t discovery_port;
        char broadcast_address[16];
        uint32_t announcement_interval_ms;
        char preferred_local_address[16];
        bool enable_debug_logs;
    } lan_config;
} PlatformState;

// Global state
extern bool g_sdk_initialized;
extern PlatformState* g_platforms[8];

// Implemented in social_bridge.c — fires friends/presence notifications for
// newly LAN-discovered peers. Called once per EOS_Platform_Tick.
void social_bridge_tick(PlatformState* platform);

#endif // EOS_PLATFORM_INTERNAL_H
