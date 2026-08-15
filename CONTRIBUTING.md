# Contributing to Abscom

Thanks for taking the time to contribute! This project is small and community-driven, and every improvement helps.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [How to Contribute](#how-to-contribute)
- [Style Guide](#style-guide)
- [Testing](#testing)
- [Reporting Issues](#reporting-issues)

## Code of Conduct

Please review our [Code of Conduct](CODE_OF_CONDUCT.md). By participating, you agree to uphold it.

## Getting Started

1. Fork the repository on GitHub.
2. Clone your fork:

   ```sh
   git clone https://github.com/<your-username>/Abscom.git
   cd Abscom
   ```

3. Build and run the tests to confirm a clean baseline:

   ```sh
   meson setup build
   meson compile -C build
   meson test -C build
   ```

## How to Contribute

- **Bug fixes** — file an issue or open a pull request with a failing test case.
- **Features** — open an issue to discuss the design before implementing. See the "Adding a feature" checklist in [docs/development.md](docs/development.md).
- **Documentation** — improve the README or the pages under `docs/`.

Pull requests should:

- Target the `main` branch.
- Be based on the latest `main`.
- Keep changes focused; avoid unrelated edits.
- Include or update tests for behavior changes.
- Update `CHANGELOG.md` for user-visible changes.

## Style Guide

- C11, no third-party dependencies.
- Core functions: `abs_<module>_<verb>` naming; return `0` on success, non-zero on failure.
- Runtime functions use short Python-like names and signal errors with `ABS_ERROR` objects.
- Keep headers self-contained; each includes `abs_common.h` first.
- Match the existing 4-space indentation used in `src/` and `include/`.

## Testing

Run the whole suite:

```sh
meson test -C build
```

Run one test:

```sh
meson test -C build test_abs
```

Add coverage for new runtime behavior to `tests/test_abs.c`, and for core modules to the matching `tests/test_*.c` file. See [docs/development.md](docs/development.md).

## Reporting Issues

- Use the GitHub issue tracker.
- Include: the version (from `.version`), your OS and toolchain, the exact commands that failed, and expected vs. actual output.
- If it is a bug in the runtime, include a minimal reproduction program.

Thank you for helping make Abscom better!

Back to [README](README.md).
