# Web Server

A tiny synchronous HTTP/1.1 server built on raw sockets, plus a `server_handle` dispatcher that works without any network at all — so routes are trivially testable.

```c
#include "abscom/abs.h"
```

The server object (`ABS_SERVER`) holds a listening socket and an `ABS_DICT` mapping request paths to function handlers (`ABS_FUNC`).

## API

| Function | Description |
| --- | --- |
| `var Server(int port)` | Create an `ABS_SERVER`. `Server(0)` binds an ephemeral port. |
| `void route(var app, const char *path, AbsFunc handler)` | Register a handler for a path (e.g. `"/"`, `"/about"`). |
| `var server_handle(var app, const char *request_line)` | Dispatch one request line like `"GET /about HTTP/1.1"`; returns the handler's body, or `NULL` for a 404. |
| `void server_run(var app)` | Serve HTTP until killed (blocking loop). |

Handlers use the standard function signature `var (*)(var)`; the request is passed as an argument (usually ignored).

```c
static var api_home(var req) {
    (void)req;
    return v("<h1>Abscom Framework</h1>");
}

var app = Server(0);
route(app, "/", api_home);

/* Dispatch without a socket: handy for tests. */
var body = server_handle(app, "GET / HTTP/1.1");   /* <h1>Abscom Framework</h1> */
if (server_handle(app, "GET /missing HTTP/1.1") == NULL) {
    print(v("404"));
}

/* Blocking HTTP server (Ctrl+C to stop). */
server_run(app);
```

## Notes

- If the socket cannot be created or bound, `Server()` still returns a valid `ABS_SERVER` with `socket_fd == -1`, so `server_handle()` continues to work.
- `server_run` responds `200 OK` when a route matches and `404 Not Found` otherwise, with `Content-Type: text/html; charset=utf-8` and `Connection: close`.
- Windows uses Winsock (linked automatically as `ws2_32`); POSIX uses the standard socket interface. No extra flags are needed on POSIX for the server itself.
- See `examples/framework_demo.c` and `tests/test_server.c` for more.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
