/* Micro web server: synchronous HTTP/1.1 over raw sockets.
 *
 * The server object (ABS_SERVER) holds a listening socket plus an ABS_DICT
 * that maps request paths to ABS_FUNC handlers. server_run() loops forever;
 * server_handle() dispatches a single request line and is used by the tests.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "abscom/abs.h"

#define SERVER_BACKLOG 5

var Server(int port) {
    var app = abs_new_obj(ABS_SERVER);
    if (!app) return abs_new_error("Out of memory");
    app->val.server.port = port;
    app->val.server.socket_fd = -1;
    app->val.server.routes = abs_new_dict();

#ifdef _WIN32
    int fd = (int)socket(AF_INET, SOCK_STREAM, 0);
#else
    int fd = socket(AF_INET, SOCK_STREAM, 0);
#endif
    if (fd < 0) {
        abs_gc_track(app);
        return app; /* socket unavailable; server_handle() still works */
    }
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((unsigned short)port);
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Server: bind failed");
#ifdef _WIN32
        closesocket((SOCKET)fd);
#else
        close(fd);
#endif
        abs_gc_track(app);
        return app;
    }
    if (listen(fd, SERVER_BACKLOG) < 0) {
        perror("Server: listen failed");
#ifdef _WIN32
        closesocket((SOCKET)fd);
#else
        close(fd);
#endif
        abs_gc_track(app);
        return app;
    }
    app->val.server.socket_fd = (intptr_t)fd;
    abs_gc_track(app);
    return app;
}

void route(var app, const char *path, AbsFunc handler) {
    if (!app || app->type != ABS_SERVER || !path || !handler) return;
    dset(app->val.server.routes, path, make_func(handler));
}

var server_handle(var app, const char *request_line) {
    if (!app || app->type != ABS_SERVER) return abs_new_error("Not a server");
    char method[16] = "";
    char path[512] = "";
    if (request_line) {
        sscanf(request_line, "%15s %511s", method, path);
    }
    var handler = dget(app->val.server.routes, path);
    if (handler->type == ABS_FUNC) return call_func(handler, None);
    return NULL; /* 404 */
}

void server_run(var app) {
    if (!app || app->type != ABS_SERVER) return;
    printf(" * Serving Abscom app on port %d...\n", app->val.server.port);
    fflush(stdout);

    for (;;) {
#ifdef _WIN32
        intptr_t client_fd = (intptr_t)accept((SOCKET)app->val.server.socket_fd, NULL, NULL);
#else
        intptr_t client_fd = (intptr_t)accept((int)app->val.server.socket_fd, NULL, NULL);
#endif
        if (client_fd < 0) continue;

        char buffer[2048] = { 0 };
#ifdef _WIN32
        recv((SOCKET)client_fd, buffer, (int)sizeof(buffer) - 1, 0);
#else
        recv((int)client_fd, buffer, sizeof(buffer) - 1, 0);
#endif

        var body = server_handle(app, buffer);
        const char *status;
        const char *content;
        char not_found[] = "404 Not Found";
        if (body) {
            var text = to_str(body);
            content = text && text->val.s ? text->val.s : "";
            status = "200 OK";
        } else {
            content = not_found;
            status = "404 Not Found";
        }

        char header[512];
        snprintf(header, sizeof(header),
                 "HTTP/1.1 %s\r\nContent-Type: text/html; charset=utf-8\r\n"
                 "Content-Length: %zu\r\nConnection: close\r\n\r\n",
                 status, strlen(content));
#ifdef _WIN32
        send((SOCKET)client_fd, header, (int)strlen(header), 0);
        send((SOCKET)client_fd, content, (int)strlen(content), 0);
        closesocket((SOCKET)client_fd);
#else
        send((int)client_fd, header, strlen(header), 0);
        send((int)client_fd, content, strlen(content), 0);
        close((int)client_fd);
#endif
    }
}
