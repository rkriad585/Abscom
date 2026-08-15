# Lifecycle and Memory

The runtime owns all objects in a memory pool. You initialize once, use objects freely, and let teardown reclaim everything.

```c
#include "abscom/abs.h"
```

## `abs_init` and `abs_cleanup`

| Function | Description |
| --- | --- |
| `void abs_init(void)` | Initialize the runtime: reset the memory pool, seed the RNG from the time, and start Winsock on Windows. Call once before any other runtime function. |
| `void abs_cleanup(void)` | Release every tracked object's internals, free the pool, and shut down Winsock. Call at the end of `main`. |

```c
#include "abscom/abs.h"

int main(void) {
    abs_init();

    var xs = range(0, 10);
    foreach (var x, xs) {
        print(x);
    }

    abs_cleanup();
    return 0;
}
```

Every program using the runtime must call `abs_init()` first and `abs_cleanup()` last.

## `del`

| Function | Description |
| --- | --- |
| `void del(var obj)` | Free the object's payload (string buffer, list/dict/set contents, file handle, error message) and mark the object as `ABS_NONE`. |

```c
abs_init();

var s = abs_new_str("temporary");
print(s);            /* temporary */
del(s);
print(s);            /* None (object is now ABS_NONE) */

abs_cleanup();
```

`del` on an already-`None` object (or `NULL`) is a no-op. The object slot itself stays allocated until `abs_cleanup`, but its payload is freed immediately.

## The memory pool

- Objects are arena-allocated in fixed-size blocks; `abs_new_*` hands out `AbsObj` slots from the pool.
- Dynamic payloads (strings, list buffers, dict nodes, file handles) are tracked in a side list and freed by `abs_cleanup`.
- You do **not** need to track or free individual objects — `abs_cleanup` reclaims everything still alive.

## Rules of thumb

1. Always pair `abs_init()` with `abs_cleanup()`.
2. Use `del` for objects you know are finished; skip it for objects that live until teardown.
3. Never call runtime functions before `abs_init()`.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
