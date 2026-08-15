# Getting Started

This guide walks through your first Abscom program. It assumes you have a working build (see [installation.md](installation.md) if you have not built the library yet).

> No build yet? Install in one line on Linux/macOS with `curl -fsSL https://raw.githubusercontent.com/rkriad585/Abscom/main/installer.sh | sh`, or on Windows with `irm https://raw.githubusercontent.com/rkriad585/Abscom/main/installer.ps1 | iex`.

## Hello, Abscom

The easiest way to try Abscom is the Python-inspired runtime (`ac_py`). Create a file named `hello.c`:

```c
#include "abscom/ac_py.h"

int main(void) {
    abs_init();

    var nums = List();
    append(nums, v(10));
    append(nums, v(20));
    append(nums, v(30));

    print(v("Sum:"), sum_val(nums));
    print(v("Sorted:"), sorted(nums, true));

    abs_cleanup();
    return 0;
}
```

Build it against the static library:

```sh
cc -std=c11 hello.c -Iinclude build/libabscom.a -o hello
```

On Windows add `-lws2_32` (required by `http_get`). Run it:

```sh
./hello
```

Expected output:

```
Sum: 60
Sorted: [30, 20, 10]
```

## What just happened?

- `abs_init()` initializes the runtime (memory pool, random seed, and Winsock on Windows). Call it once before using any `ac_py` function.
- `List()` creates an empty dynamic list.
- `append(list, item)` adds an element.
- `v(...)` is a `_Generic` literal macro that wraps a C value into a `var` object (int, float, string, or bool).
- `print(...)` prints each argument separated by spaces, followed by a newline.
- `abs_cleanup()` frees all runtime memory (and shuts down Winsock). Call it before exiting.

## Working with the core library

If you only need data structures and platform helpers, include the umbrella header instead:

```c
#include "abscom/ac.h"

int main(void) {
    ac_string_t s;
    ac_string_init(&s);
    ac_string_append_fmt(&s, "Hello %s!", "world");
    printf("%s\n", ac_string_c_str(&s));
    ac_string_destroy(&s);
    return 0;
}
```

The `ac.h` header includes `ac_dynarray.h`, `ac_string.h`, `ac_hash.h`, `ac_hashmap.h`, `ac_time.h`, and `ac_fs.h`.

## Next steps

- Explore more recipes in [usage.md](usage.md).
- Read the full function reference in [api.md](api.md).
- Run the bundled examples: `demo`, `py_demo`, `data_demo`, and `v6_demo` (see [development.md](development.md)).
- Learn how the modules fit together in [architecture.md](architecture.md).

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
