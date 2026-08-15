#include "abscom/abs.h"

#include <stdio.h>
#include <string.h>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static var api_home(var req) {
    (void)req;
    return v("<h1>Welcome</h1>");
}

static var api_about(var req) {
    (void)req;
    return v("{\"status\": \"ok\"}");
}

int main(void) {
    abs_init();

    /* A Server(0) picks an ephemeral port; server_handle() needs no socket. */
    var app = Server(0);
    CHECK(app != NULL);
    CHECK(app->type == ABS_SERVER);
    CHECK(strcmp(type(app)->val.s, "<class 'server'>") == 0);

    route(app, "/", api_home);
    route(app, "/api", api_about);
    /* Routing to a non-server object must be a no-op. */
    route(v(42), "/nope", api_home);

    /* Matching routes return the handler's body. */
    var body = server_handle(app, "GET / HTTP/1.1");
    CHECK(body != NULL);
    CHECK(body->type == ABS_STR);
    CHECK(strcmp(body->val.s, "<h1>Welcome</h1>") == 0);

    body = server_handle(app, "GET /api HTTP/1.1");
    CHECK(body != NULL);
    CHECK(strcmp(body->val.s, "{\"status\": \"ok\"}") == 0);

    /* Unknown routes yield NULL so the server can send a 404. */
    CHECK(server_handle(app, "GET /missing HTTP/1.1") == NULL);

    /* Handlers are stored as ABS_FUNC objects in the routes dict. */
    var handler = dget(app->val.server.routes, "/");
    CHECK(handler != NULL);
    CHECK(handler->type == ABS_FUNC);

    abs_cleanup();
    return 0;
}
