# Cryptography

SHA-256 digests and keyed HMAC-SHA-256 MACs, implemented from scratch in plain C (no OpenSSL or third-party code) and returned as lowercase-hex `ABS_STR` values.

```c
#include "abscom/abs.h"
```

## API

| Function | Description |
| --- | --- |
| `var sha256(const char *input)` | The SHA-256 digest of a NUL-terminated string, as a 64-char hex string. |
| `var hmac_sha256(const char *key, const char *msg)` | The HMAC-SHA-256 MAC of `msg` under `key`, as a 64-char hex string. |

```c
var h = sha256("password");
print(h);   /* 5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8 */

var mac = hmac_sha256("secret", "message");
print(mac); /* 64 lowercase hex chars */
```

## Notes

- The SHA-256 core is a compact FIPS 180-4 implementation; HMAC follows RFC 2104. Keys longer than the 64-byte block are pre-hashed, per the RFC.
- Output is always lowercase hex, 64 characters, ready to `strcmp` against or store as a string.
- `sha256(NULL)` or `hmac_sha256(NULL, ...)` return `ABS_ERROR` rather than crashing.
- Correctness is pinned to the FIPS 180-4 and RFC 4231 known-answer vectors in `tests/test_crypto.c` (including the empty string, a multi-block message, and a key larger than one block).
- The same internal hash machinery powers the WebSocket handshake (`ws_compute_accept`) in `abs_ws.c` — see [websockets.md](websockets.md).

See `tests/test_crypto.c` for the full test.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
