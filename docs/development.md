# Development

This guide covers building, testing, and working on Abscom itself.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Build](#build)
- [Test](#test)
- [Run examples](#run-examples)
- [Adding a feature](#adding-a-feature)
- [Code conventions](#code-conventions)
- [Versioning](#versioning)
- [Release checklist](#release-checklist)

## Prerequisites

- A C11 compiler (GCC, Clang, or LLVM MinGW).
- Meson 1.x and Ninja on your `PATH`.

## Build

```sh
git clone https://github.com/rkriad585/Abscom.git
cd Abscom
./build.sh            # or .\build.ps1 on Windows
```

`build.sh` / `build.ps1` wrap the raw Meson commands (`meson setup build`, `meson compile -C build`, `meson test -C build`) and add `--clean`, `--buildtype <t>`, `--skip-tests`, `--install`, and `--prefix <dir>` options.

The default build compiles both a static and a shared library, the six test executables, and the four example executables.

## Test

```sh
meson test -C build
```

The suite (`tests/test_*.c`) covers:

| Test | Covers |
| --- | --- |
| `test_dynarray` | Dynamic array push/pop/resize/access. |
| `test_string` | String buffer appends, formatting, take. |
| `test_hash` | FNV-1a and djb2 hash functions. |
| `test_hashmap` | Hash map insert/get/remove/foreach and resizing. |
| `test_platform` | Time helpers and file I/O. |
| `test_abs` | The dynamic runtime: literals, containers, JSON, random, functional helpers, sets, OOP, formatting, and error handling. |

Run a single test with `meson test -C build test_abs`.

## Run examples

```sh
./build/examples/demo
./build/examples/py_demo
./build/examples/data_demo
./build/examples/v6_demo
```

`py_demo` writes `demo_output.txt` in the working directory (part of its file-I/O demo).

## Adding a feature

1. Declare the API in the matching header under `include/abscom/`.
2. Implement it in the matching source file under `src/`.
3. Add coverage to the matching `tests/test_*.c` file.
4. If it belongs to the runtime, consider demonstrating it in one of the example programs.
5. Run `meson compile -C build` and `meson test -C build`; both must pass cleanly.

## Code conventions

- C11, no third-party dependencies.
- Core modules return `0` on success and non-zero on failure; the runtime signals errors with `ABS_ERROR` objects.
- The runtime uses `var` (`AbsObj *`), the `v()` literal macro, and pooled allocation.
- Keep the headers self-contained (each includes `abs_common.h` first).
- Follow the existing naming: `abs_<module>_<verb>` for core functions, short Python-like names for the runtime.

## Versioning

- The canonical version lives in `meson.build` (`version: '0.1.0'`).
- The `.version` file at the repository root must stay synchronized with it.
- `installer.sh` and `installer.ps1` each declare a `VERSION` used to construct the prebuilt-asset download URL; keep those in sync too.
- `CHANGELOG.md` documents user-visible changes per release.

## Release checklist

1. Update `meson.build`, `.version`, and the installer `VERSION` strings to the new version.
2. Update `CHANGELOG.md`.
3. Run a clean build and the full test suite (`./build.sh`).
4. Verify the example programs run.
5. Verify an installer install/uninstall cycle (see [installation.md](installation.md)).
6. Regenerate screenshots if the examples changed (`python tools/generate_screenshots.py`).
7. Tag the release (`git tag v0.1.0` and `git push --tags`).

Tagging `v<version>` on `main` triggers the release pipeline (`.github/workflows/release.yml`), which:

- Builds the library on the CI matrix (Linux/macOS/Windows × x86_64/arm64), runs the test suite, and attaches per-platform archives (`abscom-<os>-<arch>.zip` / `.tar.gz`) to a GitHub Release.
- The one-line installers download the matching archive for the host and install it without needing a compiler; if no prebuilt asset matches (or the download fails), they fall back to the source build.

The container workflow (`.github/workflows/container.yml`) publishes `ghcr.io/rkriad585/abscom` with `latest`, branch, semver, and SHA tags on pushes and version tags. The docs workflow (`.github/workflows/docs.yml`) deploys this site to GitHub Pages on every `docs/` or `mkdocs.yml` change.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
