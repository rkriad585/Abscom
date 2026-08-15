# Formatting and Printing

The runtime prints values with `print` and `print_end`, builds strings with `fmt`, and reads user input with `input`.

```c
#include "abscom/abs.h"
```

## Printing

`print` and `print_end` are variadic: each argument is stringified and joined with a single space.

| Function | Description |
| --- | --- |
| `print(...)` | Print each argument (space-separated) followed by a newline. |
| `print_end(const char *end, ...)` | Print each argument followed by the custom `end` string. |

```c
abs_init();

var name = abs_new_str("Ada");
print(v("Hello,"), name, v("!"));        /* Hello, Ada !   */

print_end("", v("no newline"));          /* no trailing newline */
print_end(", ", v("a"), v("b"));         /* a, b,           */
```

## Formatting

| Function | Description |
| --- | --- |
| `fmt(const char *format, ...)` | Build a string; every `{}` placeholder consumes one `var` argument. |

`fmt` is **not** `printf`-style: placeholders are literal `{}` braces, and arguments are `var` objects stringified with `to_str`:

```c
var out = fmt("Hello, {}! You have {} messages.", name, v(3));
print(out);                 /* Hello, Ada! You have 3 messages. */
```

## Input

| Function | Description |
| --- | --- |
| `var input(const char *prompt)` | Print the prompt, read a line from stdin, return it as a string. |

```c
var answer = input("Your name? ");
print(fmt("Hi {}", answer));
```

## How values stringify

- **Int** — decimal digits (`42`).
- **Float** — two decimals (`3.14`, `2.00`).
- **Str** — the raw text (no quotes).
- **Bool** — `True` / `False`.
- **None** — `None`.
- **List** — `[item, item]` with items stringified recursively.
- **Dict** — `{key: value, ...}`.
- **Set** — `{item, item, ...}`.
- **Error** — `Error: <message>` when printed; `to_str` yields just the message.
- **Class / Instance** — `<class 'name'>` / `<name object>`.
- **File** — `<file>`.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
