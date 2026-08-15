# Configuration

Abscom has **no runtime configuration file** — it reads no environment variables and no config files at run time. All configuration happens at build time through Meson options and compile-time macros.

## Meson options

The project is configured in `meson.build`:

```meson
project('abscom', 'c',
  version: '0.2.6',
  license: 'MIT',
  default_options: [
    'c_std=c11',
    'warning_level=3',
  ])
```

| Option | Default | Effect |
| --- | --- | --- |
| `c_std` | `c11` | C standard used for all sources. |
| `warning_level` | `3` | Compiler warning level (`-Wall -Wextra` and stricter on GCC/Clang). |

Override them at setup time, for example:

```sh
meson setup build -Dc_std=c11 -Dwarning_level=2
```

## Compile-time macros

| Macro | Where defined | Effect |
| --- | --- | --- |
| `ABS_BUILDING_LIBRARY` | `meson.build` (`c_args`) | Marks the library build so `ABS_API` expands to `__declspec(dllexport)` on Windows. |
| `ABS_USE_LIBRARY` | consumer builds (optional) | Marks a consumer so `ABS_API` expands to `__declspec(dllimport)` on Windows. |
| `_WIN32` | compiler | Enables the Windows code paths (Winsock, `Sleep`, QPC/FILETIME time). |
| `POOL_BLOCK_SIZE` | `src/abs.c` (`#define POOL_BLOCK_SIZE 1024`) | Number of `AbsObj` slots per memory-pool block. |

These are internal to the implementation; application code only needs to know about `ABS_API`'s behavior when building/consuming the shared library on Windows.

## Sockets

On Windows, `meson.build` detects the host system and adds `ws2_32` as a dependency automatically (`ws_deps`). POSIX builds use the standard socket headers. There is nothing to configure.

## Author tooling note

The author's publishing workflow uses a configuration directory at `~/.config/neostore/<project>/config.toml` (where `<project>` is `abscom`). This is a tool convention, not a feature of the library itself — Abscom does not read or ship such a file. A default config may be added there by the author's tooling in the future.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
