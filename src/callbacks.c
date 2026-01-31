// Define POSIX feature test macro before any includes for clock_gettime
#ifndef _WIN32
#define _POSIX_C_SOURCE 199309L
#endif

#include "callbacks.h"
#include "internal/logging.h"
#include <stdlib.h>
#include <string.h>

// Platform-specific timing
#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

/**
 * Maximum number of pending callbacks in the queue.
 * Ring buffer wraps around at this limit.
 */
#define MAX_CALLBACKS 1024

/**
 * Maximum size for callback info structures.
 * Most EOS callbacks are under 1KB, but some can be larger.
 */
#define MAX_CALLBACK_INFO_SIZE 4096

/**
 * Represents a single pending callback in the queue.
 */
typedef struct {
    void* callback_fn;                       // Function pointer to call
    uint8_t info[MAX_CALLBACK_INFO_SIZE];    // Copied callback info structure
    size_t info_size;                        // Actual size of info data
    uint64_t execute_at_ms;                  // 0 = immediate, else timestamp
    int valid;                               // 1 if slot is in use, 0 if processed
} PendingCallback;

/**
 * Callback queue structure.
 * Implements a ring buffer with head/tail pointers.
 */
struct CallbackQueue {
    PendingCallback callbacks[MAX_CALLBACKS]; // Ring buffer of callbacks
    int head;                                 // Index of next callback to process
    int tail;                                 // Index of next free slot
    int count;                                // Number of pending callbacks
};

/**
 * Get current time in milliseconds.
 * Uses platform-specific monotonic clock.
 *
 * @return Current time in milliseconds
 */
static uint64_t get_time_ms(void) {
#ifdef _WIN32
    // Windows: Use QueryPerformanceCounter for high-resolution timing
    static LARGE_INTEGER frequency = {0};
    LARGE_INTEGER counter;

    if (frequency.QuadPart == 0) {
        QueryPerformanceFrequency(&frequency);
    }

    QueryPerformanceCounter(&counter);
    return (uint64_t)((counter.QuadPart * 1000) / frequency.QuadPart);
#else
    // Linux/POSIX: Use clock_gettime with CLOCK_MONOTONIC
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#endif
}

CallbackQueue* callback_queue_create(void) {
    CallbackQueue* queue = (CallbackQueue*)calloc(1, sizeof(CallbackQueue));
    if (!queue) {
        EOS_LOG_ERROR("Failed to allocate callback queue");
        return NULL;
    }

    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;

    EOS_LOG_INFO("Created callback queue");
    return queue;
}

void callback_queue_destroy(CallbackQueue* queue) {
    if (!queue) {
        return;
    }

    int pending = queue->count;
    if (pending > 0) {
        EOS_LOG_WARN("Destroying callback queue with %d pending callbacks", pending);
    }

    free(queue);
    EOS_LOG_DEBUG("Destroyed callback queue");
}

void callback_queue_push(
    CallbackQueue* queue,
    void* callback_fn,
    const void* info,
    size_t info_size
) {
    // Immediate callback (delay = 0)
    callback_queue_push_delayed(queue, callback_fn, info, info_size, 0);
}

void callback_queue_push_delayed(
    CallbackQueue* queue,
    void* callback_fn,
    const void* info,
    size_t info_size,
    uint32_t delay_ms
) {
    if (!queue) {
        EOS_LOG_ERROR("callback_queue_push_delayed: NULL queue");
        return;
    }

    if (!callback_fn) {
        EOS_LOG_ERROR("callback_queue_push_delayed: NULL callback function");
        return;
    }

    // Validate info size
    if (info_size > MAX_CALLBACK_INFO_SIZE) {
        EOS_LOG_ERROR("callback_queue_push_delayed: info_size %zu exceeds maximum %d",
                      info_size, MAX_CALLBACK_INFO_SIZE);
        return;
    }

    // Check if queue is full
    if (queue->count >= MAX_CALLBACKS) {
        EOS_LOG_ERROR("callback_queue_push_delayed: queue full (%d callbacks), dropping callback",
                      MAX_CALLBACKS);
        return;
    }

    // Get the next available slot
    PendingCallback* cb = &queue->callbacks[queue->tail];

    // Fill in callback data
    cb->callback_fn = callback_fn;

    // Copy info structure if provided
    if (info && info_size > 0) {
        memcpy(cb->info, info, info_size);
        cb->info_size = info_size;
    } else {
        cb->info_size = 0;
    }

    // Calculate execution time
    if (delay_ms > 0) {
        cb->execute_at_ms = get_time_ms() + delay_ms;
        EOS_LOG_TRACE("Queued delayed callback (delay=%ums, execute_at=%llu)",
                      delay_ms, (unsigned long long)cb->execute_at_ms);
    } else {
        cb->execute_at_ms = 0; // Immediate execution
        EOS_LOG_TRACE("Queued immediate callback");
    }

    cb->valid = 1;

    // Advance tail pointer (ring buffer)
    queue->tail = (queue->tail + 1) % MAX_CALLBACKS;
    queue->count++;

    EOS_LOG_DEBUG("Callback queued (count=%d)", queue->count);
}

void callback_queue_process(CallbackQueue* queue) {
    if (!queue) {
        return;
    }

    if (queue->count == 0) {
        return; // Nothing to process
    }

    uint64_t now = get_time_ms();
    int processed = 0;
    int checked = 0;

    // Process callbacks from head towards tail
    // We only check callbacks that existed when process() was called
    // This prevents infinite loops if callbacks queue new callbacks
    int callbacks_to_check = queue->count;

    while (checked < callbacks_to_check) {
        int idx = (queue->head + checked) % MAX_CALLBACKS;
        PendingCallback* cb = &queue->callbacks[idx];

        // Skip invalid slots (already processed)
        if (!cb->valid) {
            checked++;
            continue;
        }

        // Check if delayed callback is ready to execute
        if (cb->execute_at_ms != 0 && cb->execute_at_ms > now) {
            // Not ready yet, skip for now
            EOS_LOG_TRACE("Delayed callback not ready (execute_at=%llu, now=%llu)",
                          (unsigned long long)cb->execute_at_ms,
                          (unsigned long long)now);
            checked++;
            continue;
        }

        // Execute the callback
        // All EOS callbacks have the signature: void callback(const T* info)
        // We cast to a generic function pointer that takes const void*
        typedef void (*GenericCallback)(const void*);
        GenericCallback fn = (GenericCallback)cb->callback_fn;

        EOS_LOG_TRACE("Executing callback (fn=%p, info_size=%zu)",
                      cb->callback_fn, cb->info_size);

        fn(cb->info);

        // Mark this slot as processed
        cb->valid = 0;
        processed++;
        checked++;

        // Advance head pointer past all invalid (processed) slots
        // This keeps the queue compacted
        while (queue->count > 0 && !queue->callbacks[queue->head].valid) {
            queue->head = (queue->head + 1) % MAX_CALLBACKS;
            queue->count--;
        }
    }

    if (processed > 0) {
        EOS_LOG_INFO("Processed %d callbacks (remaining=%d)", processed, queue->count);
    }
}

int callback_queue_pending_count(CallbackQueue* queue) {
    if (!queue) {
        return 0;
    }

    return queue->count;
}

void callback_queue_clear(CallbackQueue* queue) {
    if (!queue) {
        return;
    }

    int cleared = queue->count;

    // Mark all pending callbacks as invalid
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        queue->callbacks[i].valid = 0;
    }

    // Reset queue state
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;

    if (cleared > 0) {
        EOS_LOG_INFO("Cleared %d pending callbacks", cleared);
    }
}

/*
 * =============================================================================
 * UNIT TEST EXAMPLES
 * =============================================================================
 *
 * These tests demonstrate the callback queue functionality.
 * In a real implementation, these would be in a separate test file.
 *
 * Test 1: Basic push and process
 * -----------------------------------------------------------------------------
 *
 * static int callback_count = 0;
 * static void test_callback(const void* info) {
 *     callback_count++;
 * }
 *
 * void test_basic_callback(void) {
 *     CallbackQueue* q = callback_queue_create();
 *     assert(q != NULL);
 *
 *     int data = 42;
 *     callback_queue_push(q, test_callback, &data, sizeof(data));
 *     assert(callback_queue_pending_count(q) == 1);
 *
 *     callback_queue_process(q);
 *     assert(callback_count == 1);
 *     assert(callback_queue_pending_count(q) == 0);
 *
 *     callback_queue_destroy(q);
 * }
 *
 * Test 2: FIFO ordering
 * -----------------------------------------------------------------------------
 *
 * static int order[3];
 * static int order_idx = 0;
 *
 * static void order_callback(const void* info) {
 *     const int* val = (const int*)info;
 *     order[order_idx++] = *val;
 * }
 *
 * void test_fifo_order(void) {
 *     CallbackQueue* q = callback_queue_create();
 *
 *     int a = 1, b = 2, c = 3;
 *     callback_queue_push(q, order_callback, &a, sizeof(a));
 *     callback_queue_push(q, order_callback, &b, sizeof(b));
 *     callback_queue_push(q, order_callback, &c, sizeof(c));
 *
 *     assert(callback_queue_pending_count(q) == 3);
 *
 *     callback_queue_process(q);
 *
 *     assert(order[0] == 1);
 *     assert(order[1] == 2);
 *     assert(order[2] == 3);
 *     assert(callback_queue_pending_count(q) == 0);
 *
 *     callback_queue_destroy(q);
 * }
 *
 * Test 3: Delayed callbacks
 * -----------------------------------------------------------------------------
 *
 * #ifdef _WIN32
 * #include <windows.h>
 * static void sleep_ms(int ms) { Sleep(ms); }
 * #else
 * #include <unistd.h>
 * static void sleep_ms(int ms) { usleep(ms * 1000); }
 * #endif
 *
 * void test_delayed_callback(void) {
 *     CallbackQueue* q = callback_queue_create();
 *     callback_count = 0;
 *
 *     int data = 42;
 *     callback_queue_push_delayed(q, test_callback, &data, sizeof(data), 100);
 *
 *     // Should not fire immediately
 *     callback_queue_process(q);
 *     assert(callback_count == 0);
 *     assert(callback_queue_pending_count(q) == 1);
 *
 *     // Wait for delay to elapse
 *     sleep_ms(150);
 *
 *     // Should fire now
 *     callback_queue_process(q);
 *     assert(callback_count == 1);
 *     assert(callback_queue_pending_count(q) == 0);
 *
 *     callback_queue_destroy(q);
 * }
 *
 * Test 4: Queue full handling
 * -----------------------------------------------------------------------------
 *
 * void test_queue_full(void) {
 *     CallbackQueue* q = callback_queue_create();
 *
 *     int data = 42;
 *
 *     // Fill the queue to maximum
 *     for (int i = 0; i < MAX_CALLBACKS; i++) {
 *         callback_queue_push(q, test_callback, &data, sizeof(data));
 *     }
 *
 *     assert(callback_queue_pending_count(q) == MAX_CALLBACKS);
 *
 *     // Try to add more - should be dropped and logged
 *     for (int i = 0; i < 10; i++) {
 *         callback_queue_push(q, test_callback, &data, sizeof(data));
 *     }
 *
 *     // Still at maximum
 *     assert(callback_queue_pending_count(q) == MAX_CALLBACKS);
 *
 *     callback_queue_destroy(q);
 * }
 *
 * Test 5: Clear pending callbacks
 * -----------------------------------------------------------------------------
 *
 * void test_clear(void) {
 *     CallbackQueue* q = callback_queue_create();
 *     callback_count = 0;
 *
 *     int data = 42;
 *     callback_queue_push(q, test_callback, &data, sizeof(data));
 *     callback_queue_push(q, test_callback, &data, sizeof(data));
 *     callback_queue_push(q, test_callback, &data, sizeof(data));
 *
 *     assert(callback_queue_pending_count(q) == 3);
 *
 *     callback_queue_clear(q);
 *
 *     assert(callback_queue_pending_count(q) == 0);
 *
 *     // Process should do nothing
 *     callback_queue_process(q);
 *     assert(callback_count == 0);
 *
 *     callback_queue_destroy(q);
 * }
 *
 * Test 6: Callback queuing another callback
 * -----------------------------------------------------------------------------
 *
 * static CallbackQueue* g_queue = NULL;
 * static int recursive_count = 0;
 *
 * static void recursive_callback(const void* info) {
 *     const int* depth = (const int*)info;
 *     recursive_count++;
 *
 *     if (*depth < 3) {
 *         int next_depth = *depth + 1;
 *         callback_queue_push(g_queue, recursive_callback, &next_depth, sizeof(next_depth));
 *     }
 * }
 *
 * void test_recursive_queueing(void) {
 *     g_queue = callback_queue_create();
 *     recursive_count = 0;
 *
 *     int depth = 0;
 *     callback_queue_push(g_queue, recursive_callback, &depth, sizeof(depth));
 *
 *     // First process: executes depth=0, queues depth=1
 *     callback_queue_process(g_queue);
 *     assert(recursive_count == 1);
 *
 *     // Second process: executes depth=1, queues depth=2
 *     callback_queue_process(g_queue);
 *     assert(recursive_count == 2);
 *
 *     // Third process: executes depth=2, queues depth=3
 *     callback_queue_process(g_queue);
 *     assert(recursive_count == 3);
 *
 *     // Fourth process: executes depth=3, no more queueing
 *     callback_queue_process(g_queue);
 *     assert(recursive_count == 4);
 *     assert(callback_queue_pending_count(g_queue) == 0);
 *
 *     callback_queue_destroy(g_queue);
 * }
 *
 * =============================================================================
 */
