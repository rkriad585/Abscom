# Installation

Abscom is a C11 library built with [Meson](https://mesonbuild.com/) and [Ninja](https://ninja-build.org/). There are no third-party C dependencies: only the C standard library, plus `ws2_32` on Windows (linked automatically).

## One-line installer

The one-line installers first try to download a prebuilt release asset matching the host platform (produced by the automatic release pipeline) and install it without a compiler. If no matching asset is available they fall back to downloading the source from GitHub, building it (with Meson when available, otherwise a direct `cc`/`gcc` compile), installing to a prefix, and recording an uninstall manifest.

Windows (PowerShell):

```powershell
irm https://raw.githubusercontent.com/rkriad585/Abscom/main/installer.ps1 | iex
```

Linux and macOS:

```sh
curl -fsSL https://raw.githubusercontent.com/rkriad585/Abscom/main/installer.sh | sh
```

Both scripts accept:

| Flag (PowerShell / Unix) | Meaning |
| --- | --- |
| `-Prefix <dir>` / `--prefix <dir>` | Install prefix (defaults: `$HOME\abscom` on Windows; `/usr/local` if writable or running as root, else `$HOME/.local` on Unix) |
| `-BuildType <t>` / `--buildtype <t>` | `debug`, `debugoptimized`, `release`, or `plain` (default `release`) |
| `-SkipTests` / `--skip-tests` | Do not run the test suite |
| `-ForceDirect` / `--force-direct` | Skip Meson; compile the static library directly |
| `-SelfUninstall` / `--selfuninstall` | Remove all installed files and the download/build cache |
| `-CacheDir <dir>` / `--cache-dir <dir>` | Where sources, the build, and the manifest live |

Both scripts also honor the `ABSCOM_ASSET_URL` environment variable to force a specific prebuilt asset URL (e.g. a local `file://` copy during testing).

Uninstall:

```powershell
(Invoke-RestMethod https://raw.githubusercontent.com/rkriad585/Abscom/main/installer.ps1) + " -SelfUninstall" | iex
```

```sh
curl -fsSL https://raw.githubusercontent.com/rkriad585/Abscom/main/installer.sh | sh -s -- --selfuninstall
```

## Prerequisites

- A C11 compiler: GCC, Clang (including LLVM MinGW), or a Meson-compatible toolchain.
- Meson 1.x.
- Ninja.

The one-line installers do not require Meson or Ninja — they fall back to a direct compile with `cc`/`gcc`.

## Building

From the repository root, use the convenience wrapper or Meson directly:

```sh
./build.sh                     # configure, compile, and test
./build.sh --install --prefix "$HOME/.local"
```

On Windows:

```powershell
.\build.ps1
.\build.ps1 -Install -Prefix "C:\Abscom"
```

Or the raw commands:

```sh
meson setup build
meson compile -C build
```

This produces:

- A static library (`libabscom.a`, or `abscom.lib`/`libabscom.a` depending on toolchain).
- A shared library (`libabscom.so` / `libabscom.dll` / `libabscom.dylib`).
- Test executables in `build/tests/`.
- Example executables in `build/examples/` (`demo`, `py_demo`, `data_demo`, `v6_demo`).

## Running the tests

```sh
meson test -C build
```

`build.sh` / `build.ps1` run this automatically (pass `--skip-tests` / `-SkipTests` to skip).

## Installing

```sh
meson install -C build
```

or `./build.sh --install --prefix <dir>` / `.\build.ps1 -Install -Prefix <dir>`.

This installs the headers into `include/abscom/` and the libraries into the platform library directory, plus a pkg-config file (`abscom.pc`) into `lib/pkgconfig/`. The installed headers are: `abs.h`, `abs_common.h`, `abs_dynarray.h`, `abs_string.h`, `abs_hash.h`, `abs_hashmap.h`, `abs_time.h`, and `abs_fs.h`.

## Linking manually

Link your application against the static library and the include directory:

```sh
cc -std=c11 app.c -Iinclude build/libabscom.a -o app
```

On Windows, also link the Winsock library:

```sh
cc -std=c11 app.c -Iinclude build/libabscom.a -lws2_32 -o app.exe
```

To link an installed copy, use pkg-config (set `PKG_CONFIG_PATH` when using a custom prefix):

```sh
PKG_CONFIG_PATH="$HOME/.local/lib/pkgconfig"
cc -std=c11 app.c $(pkg-config --cflags --libs abscom) -o app
```

## Platform notes

- **Windows**: `meson.build` detects the host system and links `ws2_32` automatically; `http_get` requires a working Winsock stack (initialized by `abs_init`).
- **POSIX (Linux/macOS)**: socket and `clock_gettime` interfaces are used; no extra libraries are needed.
- The build defaults to C11 and `warning_level=3`. See [configuration.md](configuration.md) to change these.

## Docker

A `Dockerfile` is included that builds and tests the project on a Debian base image:

```sh
docker build -t abscom .
```

See [deployment.md](deployment.md) for details.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
