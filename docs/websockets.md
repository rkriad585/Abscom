# WebSockets

Real-time messaging over the [RFC 6455](https://www.rfc-editor.org/rfc/rfc6455) WebSocket protocol: the server-side opening handshake (with a real SHA-1, not a stub) and text-frame encode/decode, all on top of an already-connected socket.

```c
#include "abscom/abs.h"
```

The functions take an `abs_socket` — the same handle `Server()` uses (a Winsock `SOCKET` on Windows, an `int` on POSIX). They are standalone, so they layer onto any listening endpoint you have already `accept`ed.

## API

| Function | Description |
| --- | --- |
| `bool ws_accept(abs_socket fd, const char *request_str)` | Performs the server-side handshake: extracts the `Sec-WebSocket-Key`, sends the `101 Switching Protocols` response, returns whether it succeeded. |
| `var ws_compute_accept(const char *key)` | The `Sec-WebSocket-Accept` value for a client key: `base64(SHA1(key + magic GUID))`. |
| `void ws_send(abs_socket fd, const char *msg)` | Sends `msg` as a single unmasked text frame. |
| `var ws_recv(abs_socket fd)` | Receives one frame, unmasking it if the client masked it (RFC 6455 requires clients to mask), and returns the payload as an `ABS_STR`. `None` when the connection closes. |
| `size_t ws_encode_frame(char *out, size_t cap, const char *msg)` | Encodes `msg` into a server frame at `out`; returns the frame length (0 if `cap` is too small). Exposed so the wire format can be exercised without a socket. |
| `long ws_decode_frame(const char *buf, size_t buf_len, char *out, size_t out_cap)` | Decodes one frame (masked or unmasked) into `out`; returns the payload length, or `-1` for a malformed/truncated frame. |

## Handshake

```c
var app = Server(0);                       /* listen (abs_server.c) */
intptr_t client_fd = accept(app->val.server.socket_fd, NULL, NULL);

char request[2048] = { 0 };
recv(client_fd, request, sizeof(request) - 1, 0);

if (ws_accept(client_fd, request)) {
    ws_send(client_fd, "hello browser");
    var reply = ws_recv(client_fd);
    print(reply);
}
```

The RFC 6455 worked example checks out:

```c
print(ws_compute_accept("dGhlIHNhbXBsZSBub25jZQ=="));
/* s3pPLMBiTxaQ9kYGzzhZRbK+xOo= */
```

## Framing helpers

`ws_encode_frame` / `ws_decode_frame` round-trip without any network:

```c
char frame[32];
char out[32];
size_t n = ws_encode_frame(frame, sizeof(frame), "Hello");   /* 81 05 48 65 6c 6c 6f */
long len = ws_decode_frame(frame, n, out, sizeof(out));      /* 5, out == "Hello"  */
```

`ws_decode_frame` also handles masked client frames: it reads the 4-byte mask and XORs it off.

## Notes

- Text frames only (opcode `0x1`), unmasked server-to-client per RFC 6455 §5.3. Fragmentation, ping/pong, close frames, and extensions are out of scope — this is a minimal real-time channel, not a full browser-grade stack.
- Payload lengths use the 7-bit / 16-bit / 64-bit encodings; received frames are capped at 16 MiB.
- `ws_recv` loops to read partial `recv()` results and returns `ABS_ERROR` for oversized frames, `None` on disconnect.
- Windows links `ws2_32` automatically; POSIX needs no extra flags.
- See `tests/test_ws.c` for the handshake and framing vectors, and `examples/crypto_demo.c` for a wire-format simulation.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
