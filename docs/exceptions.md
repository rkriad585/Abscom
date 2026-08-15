# Exceptions and Context Managers

Abscom adds Python-style `try`/`catch`/`end_try` exception control flow plus a `with` context-manager macro, all on the `var` object model.

```c
#include "abscom/abs.h"
```

## How exceptions work

`throw` records a message and unwinds to the innermost active `try`. The global `abs_last_error` holds the thrown error object (an `ABS_ERROR` value) so the `catch` clause can inspect it. Each `try`/`end_try` pair is balanced: the macro pushes a `jmp_buf` on an internal stack (depth 10) and `end_try` pops it, so nested and sibling `try` blocks coexist. An uncaught `throw` prints the message and calls `exit(1)`.

```c
var divide(long a, long b) {
    if (b == 0) throw("Division by zero!");
    return v(a / b);
}

var result = None;
try {
    result = divide(10, 2);
    print(v("Success:"), result);            /* 5 */
}
catch (result) {
    print(v("Caught:"), result);             /* only on throw */
}
end_try;
```

> The `catch` macro takes the variable name that receives the caught error. Because C++ reserves `try`/`catch`/`throw` as keywords, the exception API is C-only (`#ifndef __cplusplus`).

## `with` — automatic resource cleanup

`with(VAR, INIT)` evaluates the initializer, runs the body exactly once, then calls `close_resource(VAR)`. `close_resource` closes `ABS_FILE` objects and nulls the file pointer so a later `close_file` is a no-op — no double close.

| Macro | Description |
| --- | --- |
| `with(VAR, INIT)` | Run `INIT`, execute the body once, then `close_resource(VAR)`. |
| `close_resource(var obj)` | Free a `var`'s resource (fcloses `ABS_FILE` objects). |

```c
with (f, fopen_safe("hello.txt", "w")) {
    if (is_err(f)) throw("Could not open file!");
    write_file(f, v("Hello with-macro!"));
}
/* f is closed automatically; the block ran exactly once */
```

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
