# Getting Started

This guide walks through your first Abscom program. It assumes you have a working build (see [installation.md](installation.md) if you have not built the library yet).

> No build yet? Install in one line on Linux/macOS with `curl -fsSL https://raw.githubusercontent.com/rkriad585/Abscom/main/installer.sh | sh`, or on Windows with `irm https://raw.githubusercontent.com/rkriad585/Abscom/main/installer.ps1 | iex`.

## Hello, Abscom

The easiest way to try Abscom is the Python-inspired runtime. Create a file named `hello.c`:

```c
#include "abscom/abs.h"

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

- `abs_init()` initializes the runtime (memory pool, random seed, and Winsock on Windows). Call it once before using any runtime function.
- `List()` creates an empty dynamic list.
- `append(list, item)` adds an element.
- `v(...)` is a `_Generic` literal macro that wraps a C value into a `var` object (int, float, string, or bool).
- `print(...)` prints each argument separated by spaces, followed by a newline.
- `abs_cleanup()` frees all runtime memory (and shuts down Winsock). Call it before exiting.

## Working with the core library

If you only need data structures and platform helpers, include the umbrella header instead:

```c
#include "abscom/abs.h"

int main(void) {
    abs_string_t s;
    abs_string_init(&s);
    abs_string_append_fmt(&s, "Hello %s!", "world");
    printf("%s\n", abs_string_c_str(&s));
    abs_string_destroy(&s);
    return 0;
}
```

The `abs.h` header includes `abs_dynarray.h`, `abs_string.h`, `abs_hash.h`, `abs_hashmap.h`, `abs_time.h`, `abs_fs.h`, and the dynamic runtime.

## Next steps

- Core library: [common macros](common-macros.md), [dynamic arrays](dynamic-arrays.md), [strings](strings.md), [hashing](hashing.md), [hash maps](hash-maps.md), [time](time.md), [file I/O](file-io.md).
- Runtime: [lifecycle](lifecycle.md), [literals and constructors](literals-and-constructors.md), [lists and ranges](lists-and-ranges.md), [loops](loops.md), [dictionaries](dictionaries.md), [sets](sets.md), [JSON](json.md), and more.
- Run the bundled examples: `demo`, `py_demo`, `data_demo`, and `v6_demo` (see [development.md](development.md)).
- Learn how the modules fit together in [architecture.md](architecture.md).

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
