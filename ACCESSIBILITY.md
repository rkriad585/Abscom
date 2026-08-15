# Accessibility

Abscom is a C library; it ships no graphical user interface of its own. This page documents the accessibility posture of the project's user-facing surface.

## What this project provides

- **Terminal programs** — the example programs (`demo`, `py_demo`, `data_demo`, `v6_demo`) are plain-terminal applications. They print to standard output as text and read from standard input; there is no mouse-dependent UI.
- **Documentation** — all documentation is plain Markdown, screen-reader and text-browser friendly.
- **Logo and images** — every image in `README.md`, `docs/`, and `logo/` carries alt text where practical.

## Guidelines

- Keep terminal output as plain text so it can be read by any terminal, screen reader, or pager.
- Avoid ASCII-art that relies on fixed-width layout for meaning (it does not reflow well).
- If a screenshot or diagram is added, describe its content in text nearby (see `docs/screenshots.md`).
- Prefer descriptive link text over "click here".

## Report an accessibility issue

If you find an accessibility problem in the documentation, examples, or website, please open an issue on GitHub or email <rkriad585@gmail.com>.

Back to [README](README.md).
