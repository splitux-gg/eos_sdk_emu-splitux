# Callback System Implementation Guide

## Scope
You are implementing the **Callback System** - the async operation infrastructure used by all EOS functions.

**Your responsibility:**
- Callback queue data structure
- Queuing callbacks for deferred execution
- Processing callbacks during Platform_Tick
- Memory management for callback info structures
- Optional: delayed callbacks (execute after N milliseconds)

**NOT your responsibility:**
- Calling EOS_Platform_Tick (that's the platform module)
- Specific callback types (each module defines their own)

---

## File Locations

**You will create:**
- `src/callbacks.c` - Implementation
- `src/callbacks.h` - Public interface

---

## Core Concepts

### How EOS Callbacks Work
Most EOS functions are async:
```c
// Game calls this
EOS_Sessions_CreateSessionSearch(handle, &options, &searchHandle);
EOS_SessionSearch_Find(searchHandle, &findOpts, clientData, OnFindComplete);

// Callback fires later during EOS_Platform_Tick
void OnFindComplete(const EOS_SessionSearch_FindCallbackInfo* Data) {
    // Data->ClientData is what was passed above
    // Data->ResultCode indicates success/failure
}
```

The SDK queues the callback and fires it during the next Tick.

---

## API to Implement

### callbacks.h

```c
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
 * Queue a callback for delayed execution.
 *
 * @param delay_ms    Milliseconds to wait before executing
 *
 * Callback will be executed during the first process() call
 * after delay_ms have elapsed.
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
 * Called from EOS_Platform_Tick.
 *
 * Callbacks are executed in FIFO order (for non-delayed).
 * Delayed callbacks execute when their time arrives.
 */
void callback_queue_process(CallbackQueue* queue);

/**
 * Get number of pending callbacks (for debugging).
 */
int callback_queue_pending_count(CallbackQueue* queue);

/**
 * Clear all pending callbacks without executing them.
 * Used during shutdown.
 */
void callback_queue_clear(CallbackQueue* queue);

#endif // EOS_LAN_CALLBACKS_H
```

---

## Data Structures

### callbacks.c internals

```c
#include "callbacks.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_CALLBACKS 1024
#define MAX_CALLBACK_INFO_SIZE 4096

typedef struct {
    void* callback_fn;           // Function pointer
    uint8_t info[MAX_CALLBACK_INFO_SIZE];  // Copied callback info
    size_t info_size;
    uint64_t execute_at_ms;      // 0 = immediate, else timestamp
    int valid;                   // 1 if slot is used
} PendingCallback;

struct CallbackQueue {
    PendingCallback callbacks[MAX_CALLBACKS];
    int head;  // Next slot to read
    int tail;  // Next slot to write
    int count; // Number of pending callbacks
};

// Get current time in milliseconds
static uint64_t get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}
```

---

## Implementation Details

### callback_queue_create
```c
CallbackQueue* callback_queue_create(void) {
    CallbackQueue* q = calloc(1, sizeof(CallbackQueue));
    if (q) {
        q->head = 0;
        q->tail = 0;
        q->count = 0;
    }
    return q;
}
```

### callback_queue_push
```c
void callback_queue_push(
    CallbackQueue* queue,
    void* callback_fn,
    const void* info,
    size_t info_size
) {
    callback_queue_push_delayed(queue, callback_fn, info, info_size, 0);
}

void callback_queue_push_delayed(
    CallbackQueue* queue,
    void* callback_fn,
    const void* info,
    size_t info_size,
    uint32_t delay_ms
) {
    if (!queue || !callback_fn) return;
    if (info_size > MAX_CALLBACK_INFO_SIZE) {
        // Log error - callback info too large
        return;
    }
    if (queue->count >= MAX_CALLBACKS) {
        // Log error - queue full
        return;
    }

    PendingCallback* cb = &queue->callbacks[queue->tail];
    cb->callback_fn = callback_fn;
    if (info && info_size > 0) {
        memcpy(cb->info, info, info_size);
        cb->info_size = info_size;
    } else {
        cb->info_size = 0;
    }
    cb->execute_at_ms = delay_ms > 0 ? get_time_ms() + delay_ms : 0;
    cb->valid = 1;

    queue->tail = (queue->tail + 1) % MAX_CALLBACKS;
    queue->count++;
}
```

### callback_queue_process
```c
void callback_queue_process(CallbackQueue* queue) {
    if (!queue) return;

    uint64_t now = get_time_ms();

    // Process callbacks from head towards tail
    // But only if they're ready (execute_at_ms == 0 or <= now)

    int processed = 0;
    int checked = 0;

    while (checked < queue->count) {
        int idx = (queue->head + checked) % MAX_CALLBACKS;
        PendingCallback* cb = &queue->callbacks[idx];

        if (!cb->valid) {
            checked++;
            continue;
        }

        // Check if ready to execute
        if (cb->execute_at_ms != 0 && cb->execute_at_ms > now) {
            // Not ready yet, skip
            checked++;
            continue;
        }

        // Execute the callback
        // The callback function signature varies, but they all take
        // a pointer to their specific *CallbackInfo structure
        typedef void (*GenericCallback)(const void*);
        GenericCallback fn = (GenericCallback)cb->callback_fn;
        fn(cb->info);

        // Mark as processed
        cb->valid = 0;
        processed++;
        checked++;

        // If this was at head, advance head
        while (queue->count > 0 && !queue->callbacks[queue->head].valid) {
            queue->head = (queue->head + 1) % MAX_CALLBACKS;
            queue->count--;
        }
    }
}
```

---

## Usage Example

How other modules use this:

```c
// In sessions.c
#include "callbacks.h"

void EOS_Sessions_UpdateSession(
    EOS_HSessions Handle,
    const EOS_Sessions_UpdateSessionOptions* Options,
    void* ClientData,
    const EOS_Sessions_OnUpdateSessionCallback CompletionDelegate
) {
    SessionsState* state = (SessionsState*)Handle;

    // Do the actual work...
    EOS_EResult result = do_update_session(state, Options);

    // Build callback info
    EOS_Sessions_UpdateSessionCallbackInfo info = {
        .ResultCode = result,
        .ClientData = ClientData,
        .SessionName = Options->SessionModificationHandle->session_name,
        .SessionId = state->current_session.session_id
    };

    // Queue callback for next tick
    callback_queue_push(
        state->platform->callbacks,
        (void*)CompletionDelegate,
        &info,
        sizeof(info)
    );
}
```

---

## Edge Cases

### Callback During Callback
If a callback queues another callback, it should be processed in the next Tick, not immediately. The current implementation handles this because we only process callbacks that existed when process() was called.

### NULL Callbacks
Some EOS functions allow NULL callbacks. Check before queuing:
```c
if (CompletionDelegate != NULL) {
    callback_queue_push(...);
}
```

### Large Callback Info
Some callbacks have large info structures. MAX_CALLBACK_INFO_SIZE should be at least 4KB. If a specific callback needs more, we need to dynamically allocate.

### Ordering
Non-delayed callbacks should fire in FIFO order. Delayed callbacks fire when their time comes, which may interleave with non-delayed ones.

---

## Testing Criteria

1. **Basic queue/process:**
   ```c
   static int callback_count = 0;
   static void test_callback(const void* info) {
       callback_count++;
   }

   CallbackQueue* q = callback_queue_create();
   int data = 42;
   callback_queue_push(q, test_callback, &data, sizeof(data));
   assert(callback_queue_pending_count(q) == 1);

   callback_queue_process(q);
   assert(callback_count == 1);
   assert(callback_queue_pending_count(q) == 0);
   ```

2. **FIFO ordering:**
   ```c
   static int order[3];
   static int order_idx = 0;
   static void order_callback(const int* val) {
       order[order_idx++] = *val;
   }

   int a=1, b=2, c=3;
   callback_queue_push(q, order_callback, &a, sizeof(a));
   callback_queue_push(q, order_callback, &b, sizeof(b));
   callback_queue_push(q, order_callback, &c, sizeof(c));
   callback_queue_process(q);
   assert(order[0]==1 && order[1]==2 && order[2]==3);
   ```

3. **Delayed callback:**
   ```c
   callback_queue_push_delayed(q, test_callback, &data, sizeof(data), 100);
   callback_queue_process(q);  // Should not fire
   assert(callback_count == 0);

   sleep_ms(150);
   callback_queue_process(q);  // Should fire now
   assert(callback_count == 1);
   ```

4. **Queue full handling:**
   ```c
   for (int i = 0; i < MAX_CALLBACKS + 10; i++) {
       callback_queue_push(q, test_callback, &i, sizeof(i));
   }
   assert(callback_queue_pending_count(q) == MAX_CALLBACKS);
   // Extra pushes should be dropped (logged)
   ```

---

## Build Instructions

```makefile
$(BUILD_DIR)/callbacks.o: src/callbacks.c src/callbacks.h
	$(CC) $(CFLAGS) -c $< -o $@
```

Dependencies:
- Standard C library (stdlib.h, string.h, time.h)
- POSIX time functions (clock_gettime)

For Windows compatibility, replace clock_gettime with QueryPerformanceCounter.

---

## Thread Safety

For v1: NOT thread safe. All callbacks queued and processed from same thread.

Future: Add mutex around queue operations if needed.
