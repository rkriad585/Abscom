<p align="center">
  <img src="https://raw.githubusercontent.com/rkriad585/Abscom/main/logo/logo.svg" alt="Abscom logo" width="150" height="150">
</p>

<h1 align="center">Abscom</h1>

<p align="center">
  A C11 library of reusable data structures and platform utilities, plus a Python-inspired dynamic runtime (<code>ac_py</code>) for scripting-style C programs.
</p>

## Badges

<p align="center">
  <img src="https://img.shields.io/badge/Language-C11-blue" alt="Language: C11">
  <img src="https://img.shields.io/github/v/release/rkriad585/Abscom" alt="Release">
  <img src="https://img.shields.io/github/actions/workflow/status/rkriad585/Abscom/docs.yml?label=Docs" alt="Docs">
  <img src="https://img.shields.io/github/actions/workflow/status/rkriad585/Abscom/container.yml?label=Container" alt="Container">
  <img src="https://img.shields.io/badge/License-MIT-green" alt="License: MIT">
  <img src="https://img.shields.io/badge/Made%20by-rkriad585-important" alt="Made by rkriad585">
</p>

## Overview

Abscom bundles low-level building blocks — dynamic arrays, growable strings, hash functions, an open-addressing hash map, timing helpers, and simple file I/O — under a single umbrella header. On top of that it ships `ac_py`, a Python-inspired dynamic runtime that brings `var` values, lists, dictionaries, sets, JSON, random utilities, and a light object system to plain C. The library has no dependencies beyond the C standard library (plus Winsock on Windows) and is built and tested with Meson.

## Screenshots

<p align="center">
  <img src="https://raw.githubusercontent.com/rkriad585/Abscom/main/Screenshots/home.png" alt="hello.c — the ac_py quick-start program" width="80%">
</p>

<p align="center">
  <em>Quick start — the <code>ac_py</code> dynamic runtime in a few lines of C.</em>
</p>

<table align="center">
  <tr>
    <td align="center"><img src="https://raw.githubusercontent.com/rkriad585/Abscom/main/Screenshots/ac_py.png" alt="py_demo.c — dictionaries, JSON, sets, foreach, and classes" width="95%"><br><em>ac_py runtime</em></td>
    <td align="center"><img src="https://raw.githubusercontent.com/rkriad585/Abscom/main/Screenshots/core.png" alt="core_demo.c — ac_string, ac_dynarray, ac_hashmap, ac_time, ac_fs" width="95%"><br><em>Core modules</em></td>
    <td align="center"><img src="https://raw.githubusercontent.com/rkriad585/Abscom/main/Screenshots/examples.png" alt="v6_demo.c — foreach, list_comp, and http_get" width="95%"><br><em>Example program</em></td>
  </tr>
</table>

<p align="center">
  <em>More:
    <a href="docs/screenshots.md">View all screenshots</a>
  </em>
</p>

## Table of Contents

- [Key Features](#key-features)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Usage Examples](#usage-examples)
- [Documentation](#documentation)
- [Interface](#interface)
- [Architecture](#architecture)
- [Requirements](#requirements)
- [Prerequisites](#prerequisites)
- [Development](#development)
- [Contributing](#contributing)
- [Security](#security)
- [License](#license)

## Key Features

- **Zero-dependency C11 library** — only the standard library, plus `ws2_32` on Windows for `http_get`.
- **Data structures** — dynamic array (`ac_dynarray`), growable string (`ac_string`), and an open-addressing, string-keyed hash map (`ac_hashmap`) with tombstone deletion and automatic resizing.
- **Hashing** — FNV-1a 32/64-bit and djb2 hash functions (`ac_hash`).
- **Platform helpers** — monotonic/wall-clock time (`ac_time`) and file existence/read/write/remove/rename (`ac_fs`).
- **Python-inspired runtime (`ac_py`)** — `var` objects created with the `v()` literal macro, plus `None` / `True` / `False`.
- **Containers** — lists, dictionaries, and deduplicating sets with union/difference/contains operations.
- **Strings** — split/join, strip, case conversion, `startswith`/`endswith`, and `count`.
- **JSON** — `json_parse` (objects, arrays, numbers, booleans, null, nested values) and `json_dump` with full escaping.
- **Functional helpers** — `map_func`, `filter_func`, and `list_comp` (map + filter in one pass).
- **Random utilities** — `randint`, `random_float`, `uniform`, `choice`, `choices`, `sample`, `shuffle`, and seeded/`seed()` sequences.
- **Aggregates & math** — `min_val`, `max_val`, `sum_val`, `abs_val`, `pow_val`, `round_val`.
- **Sequences** — `sorted` (ascending/descending), `reversed_seq`, `zip_lists`, `slice`, and `range`/`range_step`.
- **OOP-lite** — `Class` / `New` / `set_attr` / `get_attr` for lightweight class-and-instance objects.
- **System helpers** — `sleep_sec`, `time_now`, `exec_cmd`, and an HTTP/1.0 `http_get`.
- **One umbrella header** — `abscom/ac.h` includes all core modules; `abscom/ac_py.h` exposes the dynamic runtime.

## Installation

### One-line installer

The quickest way to install Abscom is the one-line installer. It downloads a prebuilt release asset for your platform (from the automatic release pipeline) and installs the headers, libraries, and pkg-config file to a prefix — no compiler needed. When no matching prebuilt asset is available it falls back to downloading the source and building it with Meson (or a direct `cc`/`gcc` compile). The same script uninstalls everything it placed.

Windows (PowerShell):

```powershell
irm https://raw.githubusercontent.com/rkriad585/Abscom/main/installer.ps1 | iex
```

Linux and macOS:

```sh
curl -fsSL https://raw.githubusercontent.com/rkriad585/Abscom/main/installer.sh | sh
```

Uninstall with the same command plus the uninstall flag:

```powershell
(Invoke-RestMethod https://raw.githubusercontent.com/rkriad585/Abscom/main/installer.ps1) + " -SelfUninstall" | iex
```

```sh
curl -fsSL https://raw.githubusercontent.com/rkriad585/Abscom/main/installer.sh | sh -s -- --selfuninstall
```

Pass `-Prefix <dir>` (PowerShell) or `--prefix <dir>` (Unix) to install to another location, and `-ForceDirect` / `--force-direct` to skip Meson. See [docs/installation.md](docs/installation.md) for options.

### Build from source

Requires a C11 compiler, [Meson](https://mesonbuild.com/), and [Ninja](https://ninja-build.org/).

```sh
git clone https://github.com/rkriad585/Abscom.git
cd Abscom
./build.sh                          # configure, build, and run the tests
./build.sh --install --prefix "$HOME/.local"
```

On Windows:

```powershell
git clone https://github.com/rkriad585/Abscom.git
cd Abscom
.\build.ps1
.\build.ps1 -Install -Prefix "$HOME\abscom"
```

The raw Meson commands work too: `meson setup build`, `meson compile -C build`, `meson test -C build`, and `meson install -C build`. On Windows the build automatically links the Winsock library (`ws2_32`) that `http_get` needs; no manual steps are required.

### Linking

Compile against the installed library, or let pkg-config supply the flags:

```sh
export PKG_CONFIG_PATH=<prefix>/lib/pkgconfig
cc -std=c11 hello.c $(pkg-config --cflags --libs abscom) -o hello
```

See [docs/installation.md](docs/installation.md) for platform notes.

## Quick Start

Save the following as `hello.c`:

```c
#include "abscom/ac_py.h"

int main(void) {
    abs_init();

    var nums = List();
    append(nums, v(10));
    append(nums, v(20));
    append(nums, v(30));
    print(v("Sum:"), sum_val(nums));

    var user = Dict();
    dset(user, "name", v("Alice"));
    print(v("Name:"), dget(user, "name"));

    abs_cleanup();
    return 0;
}
```

Build and run it against the static library:

```sh
cc -std=c11 hello.c -Iinclude build/libabscom.a -o hello
./hello
```

On Windows, add `-lws2_32`. If you installed Abscom with the installer instead of building it, use pkg-config (see [Installation](#installation)). Expected output:

```
Sum: 60
Name: Alice
```

See [docs/getting-started.md](docs/getting-started.md) for a longer walkthrough.

## Usage Examples

### Sets, `foreach`, and list comprehension

```c
var s = Set();
set_add(s, v(3));
set_add(s, v(1));
set_add(s, v(3));          /* duplicate, ignored */
print(v("Set:"), s);       /* {3, 1} */

var item;
long total = 0;
foreach (item, range(0, 10)) total += item->val.i;
print(v("foreach sum:"), abs_new_int(total));   /* 45 */

/* square_it and is_odd_b are user callbacks (see examples/v6_demo.c) */
print(v("Squares:"), list_comp(range(0, 10), square_it, is_odd_b));
```

### JSON round-trip

```c
var data = json_parse("{\"id\": 101, \"scores\": [10, 20, 30]}");
print(v("ID:"), dget(data, "id"));

var out = json_dump(data);
print(out);                 /* {"id": 101, "scores": [10, 20, 30]} */
```

### OOP-lite

```c
var Dog = Class("Dog");
var rex = New(Dog);
set_attr(rex, "name", v("Rex"));
print(v("Name:"), get_attr(rex, "name"));   /* Rex */
```

### HTTP/1.0 GET

```c
var html = http_get("http://example.com/");
if (is_err(html)) {
    print(v("HTTP error:"), html);
} else {
    print(v("Body length:"), len(html));
}
```

More examples live in the `examples/` directory (`demo.c`, `py_demo.c`, `data_demo.c`, `v6_demo.c`) and are built as `build/examples/<name>`.

## Documentation

| Document | Description |
| --- | --- |
| [docs/getting-started.md](docs/getting-started.md) | First steps, prerequisites, and a walkthrough. |
| [docs/installation.md](docs/installation.md) | Building, linking, and installing Abscom. |
| [docs/usage.md](docs/usage.md) | Practical API recipes with output. |
| [docs/api.md](docs/api.md) | Full reference for `ac.h` and `ac_py.h`. |
| [docs/architecture.md](docs/architecture.md) | Module layout and how the pieces fit together. |
| [docs/configuration.md](docs/configuration.md) | Build-time options and the (empty) runtime config story. |
| [docs/development.md](docs/development.md) | Building, testing, and releasing. |
| [docs/deployment.md](docs/deployment.md) | Vendoring, installation layout, and distribution. |
| [docs/screenshots.md](docs/screenshots.md) | Screenshot index. |
| [docs/faq.md](docs/faq.md) | Frequently asked questions. |
| [docs/troubleshooting.md](docs/troubleshooting.md) | Common build and runtime problems. |

## Interface

- **Core library** — include `<abscom/ac.h>` to get `ac_dynarray`, `ac_string`, `ac_hash`, `ac_hashmap`, `ac_time`, and `ac_fs` in one header.
- **Dynamic runtime** — include `<abscom/ac_py.h>` for the Python-inspired `var` API (`ac_py`).
- **ABI/export macros** — `AC_API` controls `__declspec(dllexport/dllimport)` on Windows and default visibility on GCC/Clang (`ac_common.h`).
- The library is built as both a static library and a shared library by default (`both_libraries` in Meson).

## Architecture

```mermaid
graph TD
    subgraph core["abscom core (src/, include/abscom/)"]
        common[ac_common.h - API / ABI macros]
        dyn[ac_dynarray - dynamic array]
        str[ac_string - growable string]
        hash[ac_hash - FNV-1a / djb2]
        map[ac_hashmap - open-addressing map]
        time[ac_time - monotonic / wall clock]
        fs[ac_fs - file I/O]
        py[ac_py - Python-like runtime]
    end

    subgraph users[""]
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

See [docs/architecture.md](docs/architecture.md) for the full layout.

## Requirements

- A C11 compiler (GCC, Clang, or MSVC-compatible).
- Meson (tested with 1.x) and Ninja for building from source.
- Windows builds link `ws2_32`; POSIX builds use the standard socket and `clock_gettime` interfaces.
- No third-party C library dependencies.

## Prerequisites

- Linux/macOS: a C toolchain plus Meson and Ninja.
- Windows: a MinGW toolchain (e.g. LLVM MinGW) or MSVC, plus Meson and Ninja; the generated build handles `ws2_32` automatically.

## Development

```sh
git clone https://github.com/rkriad585/Abscom.git
cd Abscom
./build.sh            # or .\build.ps1 on Windows
```

`build.sh` / `build.ps1` wrap `meson setup`, `meson compile`, and `meson test`, and accept `--clean`, `--buildtype`, `--skip-tests`, `--install`, and `--prefix` (`-Clean`, `-BuildType`, `-SkipTests`, `-Install`, `-Prefix` in PowerShell). The equivalent raw commands:

```sh
meson setup build
meson compile -C build
meson test -C build
```

Run the example programs:

```sh
./build/examples/demo
./build/examples/py_demo
./build/examples/data_demo
./build/examples/v6_demo
```

See [docs/development.md](docs/development.md) for details.

## Contributing

Contributions are welcome. Please read [CONTRIBUTING.md](CONTRIBUTING.md) first, and note the [Code of Conduct](CODE_OF_CONDUCT.md).

## Security

Please report security issues responsibly. See [SECURITY.md](SECURITY.md) for the supported versions and reporting policy.

## License

Abscom is released under the [MIT License](LICENSE).

## Acknowledgments

- Built with [Meson](https://mesonbuild.com/) and [Ninja](https://ninja-build.org/).
- `ac_py` is inspired by the ergonomics of the [Python](https://www.python.org/) language.
- Maintained by [rkriad585](https://github.com/rkriad585).
