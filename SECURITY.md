# Security Policy

## Supported Versions

Abscom is pre-1.0. Security fixes are applied to the latest `main` branch. Once tagged releases exist, this section will list each supported version; until then, `main` is the only supported channel.

| Version | Supported |
| --- | --- |
| `main` | Yes |
| < 0.2.6 (unreleased) | Yes |

## Reporting a Vulnerability

Please do **not** open a public issue for security vulnerabilities. Instead, report them privately to the maintainer at:

- Email: <rkriad585@gmail.com>

Include as much detail as possible:

- Affected version (from `.version`).
- Description of the vulnerability and its impact.
- A minimal reproduction (code or steps).
- Suggested fix, if you have one.

You will receive an acknowledgment within a few days. We will investigate, fix the issue on `main`, and credit reporters (unless anonymity is requested) when a fix is released.

## Security Notes

- The library performs no file or socket I/O automatically; applications opt in via the public API.
- `http_get` performs a blocking HTTP/1.0 GET over an unencrypted connection (port 80). Do not use it for sensitive data.
- Memory is managed via a block pool plus reference-style tracking; misuse (e.g. dereferencing a `var` after `del()`) is the caller's responsibility, as with any C API.
