#ifndef EOS_LAN_CALLBACKS_H
#define EOS_LAN_CALLBACKS_H

#include <stddef.h>
#include <stdint.h>

typedef struct CallbackQueue CallbackQueue;

/**
 * Create a new callback queue.
 * Returns NULL on allocation failure.
 */
CallbackQueue* callback_queue_create(void);

/**
 * Destroy a callback queue, freeing all pending callbacks.
 */
void callback_queue_destroy(CallbackQueue* queue);

/**
 * Queue a callback for execution on next process() call.
 *
 * @param queue       The callback queue
 * @param callback_fn Function pointer to call (cast to void*)
 * @param info        Callback info structure to pass to function
 * @param info_size   Size of info structure (will be copied)
 *
 * The info structure is copied, so caller can free/reuse it after this call.
 */
void callback_queue_push(
    CallbackQueue* queue,
    void* callback_fn,
    const void* info,
    size_t info_size
);

/**
 * Process all queued callbacks.
 * Called during EOS_Platform_Tick.
 *
 * @param queue The callback queue
 */
void callback_queue_process(CallbackQueue* queue);

#endif // EOS_LAN_CALLBACKS_H
