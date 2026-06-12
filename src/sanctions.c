/**
 * EOS-LAN Emulator - Sanctions interface
 *
 * Minimal real implementation (NOT a generated 0/NULL stub) because UE5's
 * FOnlineSubsystemEOS::Init treats a null sanctions handle as fatal:
 *   "failed to init EOS platform, couldn't get player sanction handle"
 * and aborts the whole EOS platform — falling back to the NULL subsystem.
 * It then calls EOS_Sanctions_QueryActivePlayerSanctions during login and
 * waits for the callback, so that must complete (with zero sanctions) or
 * login stalls. This unblocks every UE5 OnlineSubsystemEOS title.
 *
 * GetSanctionsInterface returns the platform handle itself, so the query
 * functions can reach platform->callbacks (same trick the other interfaces
 * use). No player is ever sanctioned in LAN play.
 */

#include "eos/eos_sanctions.h"
#include "eos/eos_sanctions_types.h"
#include "internal/platform_internal.h"
#include "internal/callbacks.h"
#include "internal/logging.h"

EOS_DECLARE_FUNC(EOS_HSanctions) EOS_Platform_GetSanctionsInterface(EOS_HPlatform Handle) {
    EOS_LOG_API_ENTER();
    PlatformState* platform = (PlatformState*)Handle;
    if (!platform) {
        EOS_LOG_ERROR("Invalid platform handle");
        return NULL;
    }
    // Hand back the platform handle; the query funcs cast it back to reach
    // the callback queue. UE only needs this to be non-null to proceed.
    return (EOS_HSanctions)platform;
}

EOS_DECLARE_FUNC(void) EOS_Sanctions_QueryActivePlayerSanctions(
    EOS_HSanctions Handle,
    const EOS_Sanctions_QueryActivePlayerSanctionsOptions* Options,
    void* ClientData,
    const EOS_Sanctions_OnQueryActivePlayerSanctionsCallback CompletionDelegate)
{
    EOS_LOG_API_ENTER();
    PlatformState* platform = (PlatformState*)Handle;

    EOS_Sanctions_QueryActivePlayerSanctionsCallbackInfo info;
    info.ResultCode = EOS_Success;          // no sanctions service in LAN; treat as clean
    info.ClientData = ClientData;
    info.TargetUserId = Options ? Options->TargetUserId : NULL;
    info.LocalUserId = Options ? Options->LocalUserId : NULL;

    if (CompletionDelegate && platform && platform->callbacks) {
        callback_queue_push(platform->callbacks, (void*)CompletionDelegate, &info, sizeof(info));
        EOS_LOG_INFO("QueryActivePlayerSanctions: returning EOS_Success (0 sanctions)");
    } else {
        EOS_LOG_WARN("QueryActivePlayerSanctions: no callback queue; cannot complete");
    }
}

// After a clean query there are zero sanctions for any user.
EOS_DECLARE_FUNC(uint32_t) EOS_Sanctions_GetPlayerSanctionCount(
    EOS_HSanctions Handle,
    const EOS_Sanctions_GetPlayerSanctionCountOptions* Options)
{
    (void)Handle; (void)Options;
    return 0;
}
