# Troubleshooting

## Table of Contents

- [Meson or Ninja not found](#meson-or-ninja-not-found)
- [Compile errors about `ws2_32` or sockets](#compile-errors-about-ws2_32-or-sockets)
- [My program crashes with a null pointer or access violation](#my-program-crashes-with-a-null-pointer-or-access-violation)
- [`http_get` returns an error](#http_get-returns-an-error)
- [`print` prints nothing](#print-prints-nothing)
- [Tests fail on the very first run](#tests-fail-on-the-very-first-run)
- [DLL import/export errors on Windows](#dll-importexport-errors-on-windows)

## Meson or Ninja not found

```
meson: command not found
```

Ensure Meson and Ninja are installed and on your `PATH`. On Windows with a Python-based Meson install, the executables live in the Python `Scripts` directory (e.g. `...\Python312\Scripts`). Prepend it to `PATH`:

```powershell
$env:PATH = "C:\Users\<you>\AppData\Local\Programs\Python\Python312\Scripts;" + $env:PATH
```

## Compile errors about `ws2_32` or sockets

The build links `ws2_32` automatically when Meson detects Windows. If you are compiling manually, add `-lws2_32`:

```sh
cc -std=c11 app.c -Iinclude build/libabscom.a -lws2_32 -o app.exe
```

If you see conflicting Windows headers, make sure only one toolchain's headers are active (a mix of MSVC and MinGW include paths causes this).

## My program crashes with a null pointer or access violation

- Did you call `abs_init()` before using the runtime? The memory pool must be initialized.
- Are you calling `http_get` without Winsock initialized? That path is set up by `abs_init()`.
- Check `is_err()` on functions that can fail (`get`, `json_parse`, `fopen_safe`, `http_get`, ...) before dereferencing.
- `v()` takes a value; passing a raw `bool` where `print` expects `var` arguments is a type error — wrap booleans with `v(...)`.

## `http_get` returns an error

`http_get` is a blocking HTTP/1.0 GET on port 80. Check:

- The URL is `http://` (HTTPS is not supported).
- The hostname resolves and port 80 is reachable (firewalls/proxies can block it).
- `abs_init()` was called first on Windows.

Inspect the error object's message: `print(v("error:"), err)` prints `Error: <message>`.

## `print` prints nothing

Output is buffered by default in some terminals. For a demo you can disable buffering at the start of `main`:

```c
setvbuf(stdout, NULL, _IONBF, 0);
```

(the `examples/v6_demo.c` program does exactly this).

## Tests fail on the very first run

The test suite writes temporary files (`test_abs_tmp.txt`, `abs_fs_test.txt`, ...) and `py_demo` writes `demo_output.txt`. Make sure the working directory is writable. Rebuild from a clean state if an interrupted run left stale files:

```sh
rm -rf build
meson setup build
meson compile -C build
meson test -C build
```

## DLL import/export errors on Windows

`ABS_API` handles this automatically: it becomes `__declspec(dllexport)` when `ABS_BUILDING_LIBRARY` is defined (set by `meson.build`) and `__declspec(dllimport)` when `ABS_USE_LIBRARY` is defined by a consumer. If you see unresolved symbols against the shared library, define `ABS_USE_LIBRARY` when compiling your application, or link the static library instead.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
