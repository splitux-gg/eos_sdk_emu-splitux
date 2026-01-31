#ifndef EOS_LAN_CALLBACKS_H
#define EOS_LAN_CALLBACKS_H

#include <stddef.h>
#include <stdint.h>

/**
 * Opaque callback queue structure.
 * Holds pending callbacks to be processed during EOS_Platform_Tick.
 */
typedef struct CallbackQueue CallbackQueue;

/**
 * Create a new callback queue.
 *
 * The queue uses a ring buffer with a fixed maximum size.
 * Callbacks are stored with their data copied into the queue.
 *
 * @return New callback queue, or NULL on allocation failure.
 */
CallbackQueue* callback_queue_create(void);

/**
 * Destroy a callback queue, freeing all pending callbacks.
 *
 * @param queue The callback queue to destroy (may be NULL)
 */
void callback_queue_destroy(CallbackQueue* queue);

/**
 * Queue a callback for execution on next process() call.
 *
 * The callback will be executed during the next callback_queue_process() call.
 * The info structure is copied into the queue, so the caller can free/reuse
 * it immediately after this call.
 *
 * @param queue       The callback queue
 * @param callback_fn Function pointer to call (cast to void*)
 * @param info        Callback info structure to pass to function
 * @param info_size   Size of info structure in bytes
 *
 * @note If the queue is full or info_size exceeds the maximum, the callback
 *       will be dropped and an error will be logged.
 * @note Callbacks are executed in FIFO order.
 */
void callback_queue_push(
    CallbackQueue* queue,
    void* callback_fn,
    const void* info,
    size_t info_size
);

/**
 * Queue a callback for delayed execution.
 *
 * The callback will be executed during the first process() call after
 * delay_ms milliseconds have elapsed since this call.
 *
 * @param queue       The callback queue
 * @param callback_fn Function pointer to call (cast to void*)
 * @param info        Callback info structure to pass to function
 * @param info_size   Size of info structure in bytes
 * @param delay_ms    Milliseconds to wait before executing
 *
 * @note Delayed callbacks may interleave with non-delayed callbacks
 *       depending on when their delay expires.
 */
void callback_queue_push_delayed(
    CallbackQueue* queue,
    void* callback_fn,
    const void* info,
    size_t info_size,
    uint32_t delay_ms
);

/**
 * Process all pending callbacks whose time has come.
 *
 * This function is called from EOS_Platform_Tick.
 * - Non-delayed callbacks (delay_ms == 0) are executed immediately in FIFO order
 * - Delayed callbacks are executed when their delay has elapsed
 *
 * Callbacks are executed by casting the function pointer to a generic signature
 * that takes a const void* parameter, then calling it with the info buffer.
 *
 * @param queue The callback queue to process
 *
 * @note If a callback queues another callback, the new callback will be
 *       processed in the next tick, not during the current process() call.
 */
void callback_queue_process(CallbackQueue* queue);

/**
 * Get the number of pending callbacks.
 *
 * Useful for debugging and testing.
 *
 * @param queue The callback queue
 * @return Number of callbacks waiting to be processed
 */
int callback_queue_pending_count(CallbackQueue* queue);

/**
 * Clear all pending callbacks without executing them.
 *
 * Used during shutdown to prevent callbacks from firing after
 * the platform has been destroyed.
 *
 * @param queue The callback queue to clear
 */
void callback_queue_clear(CallbackQueue* queue);

#endif // EOS_LAN_CALLBACKS_H
