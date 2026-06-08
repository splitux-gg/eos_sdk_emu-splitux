/**
 * EOS-LAN Emulator - Integrated Platform options container
 *
 * The Integrated Platform interface lets a game mirror an "integrated" platform
 * (Steam/PSN/etc.) identity into EOS. A game builds an options container, Adds
 * its platform options, hands the container to EOS_Platform_Create, then
 * Releases it.
 *
 * For a clean-room LAN emulator we do NOT mirror any integrated identity — LAN
 * play has no Steam/EOS bridging to do. But the *container lifecycle* still has
 * to behave: the PlayEveryWare EOS plugin (Unity games such as Make Way and
 * V Rising) resolves EOS_IntegratedPlatform_CreateIntegratedPlatformOptionsContainer
 * via GetProcAddress at startup and calls it before EOS_Platform_Create. If we
 * don't export it, GetProcAddress returns NULL, the plugin calls a null pointer
 * and crashes during graphics init — exactly the Make Way crash. So we hand back
 * a real (if inert) container handle and accept Add/Release so the boot path
 * survives. EOS_Platform_Create simply ignores the container handle.
 */

#include "eos/eos_integratedplatform.h"
#include "eos/eos_integratedplatform_types.h"
#include "eos/eos_common.h"
#include "internal/logging.h"
#include <stdlib.h>

#define IPOC_MAGIC 0x49504F43u  /* 'IPOC' */

typedef struct IntegratedPlatformOptionsContainer {
    uint32_t magic;
    int      added;   /* how many platform options the game Added (diagnostic only) */
} IntegratedPlatformOptionsContainer;

EOS_DECLARE_FUNC(EOS_EResult) EOS_IntegratedPlatform_CreateIntegratedPlatformOptionsContainer(
    const EOS_IntegratedPlatform_CreateIntegratedPlatformOptionsContainerOptions* Options,
    EOS_HIntegratedPlatformOptionsContainer* OutIntegratedPlatformOptionsContainerHandle)
{
    (void)Options;  /* only carries ApiVersion; nothing to act on for LAN */
    if (!OutIntegratedPlatformOptionsContainerHandle) {
        EOS_LOG_ERROR("IntegratedPlatform_CreateContainer: OutHandle is NULL");
        return EOS_InvalidParameters;
    }

    IntegratedPlatformOptionsContainer* c = calloc(1, sizeof(*c));
    if (!c) {
        EOS_LOG_ERROR("IntegratedPlatform_CreateContainer: allocation failed");
        return EOS_UnexpectedError;
    }
    c->magic = IPOC_MAGIC;
    *OutIntegratedPlatformOptionsContainerHandle = (EOS_HIntegratedPlatformOptionsContainer)c;
    EOS_LOG_INFO("IntegratedPlatform: created options container (inert; LAN does not mirror integrated identity)");
    return EOS_Success;
}

EOS_DECLARE_FUNC(EOS_EResult) EOS_IntegratedPlatformOptionsContainer_Add(
    EOS_HIntegratedPlatformOptionsContainer Handle,
    const EOS_IntegratedPlatformOptionsContainer_AddOptions* InOptions)
{
    (void)InOptions;  /* accept and ignore the integrated-platform options */
    IntegratedPlatformOptionsContainer* c = (IntegratedPlatformOptionsContainer*)Handle;
    if (!c || c->magic != IPOC_MAGIC) {
        EOS_LOG_ERROR("IntegratedPlatformOptionsContainer_Add: invalid container handle");
        return EOS_InvalidParameters;
    }
    c->added++;
    EOS_LOG_DEBUG("IntegratedPlatformOptionsContainer_Add: accepted option #%d (ignored for LAN)", c->added);
    return EOS_Success;
}

EOS_DECLARE_FUNC(void) EOS_IntegratedPlatformOptionsContainer_Release(
    EOS_HIntegratedPlatformOptionsContainer IntegratedPlatformOptionsContainerHandle)
{
    /* Header contract: safe to call on a NULL handle. */
    IntegratedPlatformOptionsContainer* c =
        (IntegratedPlatformOptionsContainer*)IntegratedPlatformOptionsContainerHandle;
    if (c && c->magic == IPOC_MAGIC) {
        c->magic = 0;
        free(c);
    }
}
