# Events

A tiny publish/subscribe event emitter: an `EventBus` is a dictionary mapping event names to a list of handler functions.

```c
#include "abscom/abs.h"
```

| Function | Description |
| --- | --- |
| `var EventBus(void)` | Create a bus (an `ABS_DICT`). |
| `void on(var bus, const char *event_name, AbsFunc handler)` | Register a handler for an event. |
| `void emit(var bus, const char *event_name, var data)` | Run every handler for the event, in registration order. |

Handlers receive the emitted payload as their `var` argument and may return a value (ignored by `emit`). Calling `emit` for an event with no handlers is a no-op.

```c
static int logins = 0;

static var on_user_login(var user) {
    logins++;
    print(v("Logged in:"), user);
    return None;
}

var bus = EventBus();
on(bus, "login", on_user_login);
on(bus, "login", on_user_login);

emit(bus, "login", v("Alice"));   /* runs on_user_login twice */
print(v("Logins:"), v(logins));   /* 2 */
```

See `tests/test_events.c` for the full test.
