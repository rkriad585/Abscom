# Architecture

Abscom is a C11 library split into a core module set and a Python-like runtime. Everything is compiled into a single library by Meson.

## Repository layout

```
Abscom/
├── include/abscom/       # Public headers
│   ├── ac.h              # Umbrella header (core modules)
│   ├── ac_common.h       # AC_API, extern "C", AC_UNUSED
│   ├── ac_dynarray.h     # Dynamic array
│   ├── ac_string.h       # Growable string
│   ├── ac_hash.h         # Hash functions
│   ├── ac_hashmap.h      # String-keyed hash map
│   ├── ac_time.h         # Time helpers
│   ├── ac_fs.h           # File I/O
│   └── ac_py.h           # Python-like runtime
├── src/                  # Implementations
│   ├── ac_dynarray.c
│   ├── ac_string.c
│   ├── ac_hash.c
│   ├── ac_hashmap.c
│   ├── ac_time.c
│   ├── ac_fs.c
│   └── ac_py.c
├── tests/                # Meson test suite (test_*.c)
├── examples/             # Demo programs (demo, py_demo, data_demo, v6_demo)
├── docs/                 # Documentation
├── tools/                # Dev tooling (generate_screenshots.py)
├── build.sh / build.ps1  # Local build/test/install wrappers
├── installer.sh / installer.ps1  # One-line installers
├── logo/                 # Project logo (logo.svg)
└── Screenshots/          # Screenshots (see docs/screenshots.md)
```

## Module overview

```mermaid
graph TD
    subgraph core["abscom core"]
        common[ac_common.h - API / ABI macros]
        dyn[ac_dynarray - dynamic array]
        str[ac_string - growable string]
        hash[ac_hash - FNV-1a / djb2]
        map[ac_hashmap - open-addressing map]
        time[ac_time - monotonic / wall clock]
        fs[ac_fs - file I/O]
        py[ac_py - Python-like runtime]
    end

    subgraph consumers["consumers"]
        tests[tests/test_* - Meson test suite]
        examples[examples/* - demo programs]
    end

    dyn --> common
    str --> common
    hash --> common
    map --> hash
    time --> common
    fs --> common
    py --> str
    tests --> core
    examples --> core
```

## Core modules

- **`ac_common.h`** — defines `AC_API` (DLL export/import on Windows, default visibility on GCC/Clang), `AC_BEGIN_C_DECLS`/`AC_END_C_DECLS` for C++ interoperability, and `AC_UNUSED`. Every other header includes it.
- **`ac_dynarray`** — a generic (byte-memory) dynamic array. Callers provide `elem_size`; growth doubles capacity (`push` starts at 8).
- **`ac_string`** — a NUL-terminated growable string with `append_*` variants, `append_fmt`, `set_cstr`, and `take` (ownership transfer).
- **`ac_hash`** — FNV-1a 32/64 and djb2. `ac_hashmap` depends on FNV-1a 64 for string keys.
- **`ac_hashmap`** — open-addressing map with linear probing, tombstone deletion, and automatic resizing when load exceeds 70%. `ac_hashmap_free_fn` optionally frees values.
- **`ac_time`** — wraps `QueryPerformanceCounter`/`GetSystemTimeAsFileTime` on Windows and `clock_gettime` on POSIX behind four simple functions.
- **`ac_fs`** — thin wrappers around `fopen`/`remove`/`rename`.

## The `ac_py` runtime

`ac_py` sits on top of `ac_string` and the platform code. Its key design points:

- **Object model** — `AbsObj` is a tagged union (`AbsType` + value union). Pointers are pooled: `ac_py.c` uses a block allocator (`POOL_BLOCK_SIZE` objects per `MemBlock`) for most objects, so allocating is cheap.
- **Memory management** — objects with heap internals (strings, lists, dicts, sets, errors, classes, files) are tracked on a `gc_dynamic_head` list; `abs_cleanup` frees internals and then the pool blocks. `del()` frees an object's internals and marks it `ABS_NONE`.
- **Literal macro** — `v(X)` dispatches on the C type via `_Generic` to the right `abs_new_*` constructor.
- **Containers** — lists and sets share a `{items, size, capacity}` struct; dicts use bucket chains (`DictNode`) with a djb2-style `dict_hash`.
- **Functional layer** — `map_func`/`filter_func` take `AbsFunc` callbacks; `list_comp` takes a `AbsMapFunc` and a bool-returning `AbsFilterFunc`.
- **JSON** — `json_parse` is a small recursive-descent parser over the pooled object model; `json_dump` serializes recursively with string escaping.
- **Networking** — `http_get` performs a blocking HTTP/1.0 GET over a raw socket (Winsock on Windows, BSD sockets on POSIX), reading until the peer closes the connection and returning the response body after `\r\n\r\n`.

## Build system

`meson.build` declares one `project`, builds the sources with `both_libraries` (static + shared), links `ws2_32` on Windows, and installs the core headers under `include/abscom`. `tests/meson.build` registers every `test_*.c` as a Meson test; `examples/meson.build` builds the demo executables.

## Lifecycle

1. `abs_init()` seeds the RNG and, on Windows, calls `WSAStartup`.
2. The program uses `ac_py` (or core modules directly).
3. `abs_cleanup()` frees dynamic internals, all pool blocks, and calls `WSACleanup` on Windows.

See [api.md](api.md) for the full function reference and [getting-started.md](getting-started.md) for a first program.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
