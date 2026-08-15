# Frequently Asked Questions

## What is Abscom?

Abscom is a C11 library of reusable data structures (dynamic arrays, strings, hash maps) and platform helpers (time, file I/O, hashing), plus `ac_py`, a Python-inspired dynamic runtime for writing scripting-style C programs. See [getting-started.md](getting-started.md).

## Do I need Python installed?

No. `ac_py` is written in C and only borrows Python's *ergonomics* (`var`, `v(1)`, `List()`, `Dict()`, `print(...)`). It has no runtime dependency on Python.

## What compilers are supported?

Any C11 compiler Meson supports. The code has Windows (`winsock2.h`, `windows.h`) and POSIX (`unistd.h`, `sys/socket.h`) branches. It is developed and tested with Clang (LLVM MinGW) on Windows.

## What dependencies does Abscom have?

None beyond the C standard library. On Windows the build links `ws2_32` (for `http_get`); this is automatic via Meson.

## Why does `ac_py` use a memory pool?

Most `AbsObj` values are allocated from a block pool (`POOL_BLOCK_SIZE` per block) so constructing `var` objects is cheap and there is no per-object `malloc` overhead. Objects with heap internals are tracked separately and freed by `abs_cleanup` / `del`.

## Do I have to call `abs_init` / `abs_cleanup`?

Call `abs_init()` once before using any `ac_py` function — it seeds the RNG and starts Winsock on Windows. Call `abs_cleanup()` before exiting to release pool blocks and internals (and to call `WSACleanup`). The core modules (`ac_dynarray`, `ac_string`, `ac_hashmap`, ...) do not require it.

## Does `del()` free memory?

`del(obj)` frees an object's heap internals (string data, list/dict/set buffers, class name, open files) and marks it `ABS_NONE`. Pool slots themselves are reclaimed when `abs_cleanup` frees the whole pool.

## Why does `sorted` not modify the original list?

`sorted` and `reversed_seq` return new lists by design, mirroring Python. Use `shuffle` for in-place list mutation.

## How are errors reported?

Core modules return non-zero/`NULL`. `ac_py` returns `ABS_ERROR` objects — check them with `is_err(obj)`. For example, `get(list, 100)` and `json_parse("bad")` return errors.

## Why does `http_get` sometimes fail?

`http_get` performs a blocking HTTP/1.0 GET over port 80. It needs network access, a resolvable hostname, and (on Windows) a Winsock stack initialized by `abs_init`. It returns an `ABS_ERROR` object on failure instead of crashing.

## Where are the tests?

Under `tests/`, each file is a standalone C program registered with Meson. Run them all with `meson test -C build`.

## How do I report a bug or request a feature?

Open an issue on the GitHub repository and follow the guidance in [CONTRIBUTING.md](https://github.com/rkriad585/Abscom/blob/main/CONTRIBUTING.md).

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
