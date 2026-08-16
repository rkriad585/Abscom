/* WebSockets: RFC 6455 handshake and framing over an already-connected socket.
 *
 * The functions operate on an accepted client socket (abs_socket), so they can
 * be layered on top of any listening endpoint:
 *
 *   var app = Server(port);                     abs_server.c, or your own
 *   int client = accept(server_sock, ...);      connection arrives
 *   if (ws_accept(client, request_buf))         101 Switching Protocols
 *       ws_send(client, "hello");
 *   var reply = ws_recv(client);                decoded, unmasked string
 *
 * The handshake needs real SHA-1 (RFC 3174) plus base64, both implemented
 * here in the public-domain style. The frame encode/decode helpers are public
 * so tests and tools can exercise the wire format without a live socket.
 *
 * Server-to-client frames are unmasked (RFC 6455 section 5.3); client-to-server
 * frames are masked and are unmasked on receipt.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#endif

#include "abscom/abs.h"

#define WS_MAGIC "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define WS_MAX_FRAME (1u << 24) /* 16 MiB sanity cap for received frames */

/* --- SHA-1 core (RFC 3174 style) --- */

static uint32_t rol32(uint32_t x, int n) {
    return (x << n) | (x >> (32 - n));
}

static void sha1_transform(uint32_t state[5], const uint8_t block[64]) {
    uint32_t w[80], a, b, c, d, e, f, k, t;
    int i;
    for (i = 0; i < 16; i++) {
        w[i] = ((uint32_t)block[i * 4] << 24) |
               ((uint32_t)block[i * 4 + 1] << 16) |
               ((uint32_t)block[i * 4 + 2] << 8) | block[i * 4 + 3];
    }
    for (i = 16; i < 80; i++)
        w[i] = rol32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);

    a = state[0]; b = state[1]; c = state[2]; d = state[3]; e = state[4];
    for (i = 0; i < 80; i++) {
        if (i < 20) {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
        }
        t = rol32(a, 5) + f + e + k + w[i];
        e = d; d = c; c = rol32(b, 30); b = a; a = t;
    }
    state[0] += a; state[1] += b; state[2] += c; state[3] += d; state[4] += e;
}

static void sha1_bytes(const void *input, size_t len, uint8_t *out) {
    uint32_t state[5] = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476,
                          0xC3D2E1F0 };
    uint8_t buf[64];
    size_t i, rem;
    uint64_t bits = (uint64_t)len * 8;
    const uint8_t *msg = (const uint8_t *)input;

    for (i = 0; len - i >= 64; i += 64) sha1_transform(state, msg + i);
    rem = len - i;
    memcpy(buf, msg + i, rem);
    buf[rem++] = 0x80;
    if (rem > 56) {
        memset(buf + rem, 0, 64 - rem);
        sha1_transform(state, buf);
        rem = 0;
    }
    memset(buf + rem, 0, 56 - rem);
    for (i = 0; i < 8; i++) buf[63 - i] = (uint8_t)((bits >> (i * 8)) & 0xFF);
    sha1_transform(state, buf);
    for (i = 0; i < 5; i++) {
        out[i * 4] = (uint8_t)(state[i] >> 24);
        out[i * 4 + 1] = (uint8_t)(state[i] >> 16);
        out[i * 4 + 2] = (uint8_t)(state[i] >> 8);
        out[i * 4 + 3] = (uint8_t)state[i];
    }
}

/* --- base64 (RFC 4648) --- */

static char *b64_encode(const uint8_t *src, size_t len) {
    static const char t[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t olen = 4 * ((len + 2) / 3);
    char *out = (char *)malloc(olen + 1);
    char *p;
    size_t i;
    if (!out) return NULL;
    p = out;
    for (i = 0; i < len; i += 3) {
        uint32_t v = (uint32_t)src[i] << 16;
        if (i + 1 < len) v |= (uint32_t)src[i + 1] << 8;
        if (i + 2 < len) v |= src[i + 2];
        *p++ = t[(v >> 18) & 0x3F];
        *p++ = t[(v >> 12) & 0x3F];
        *p++ = (i + 1 < len) ? t[(v >> 6) & 0x3F] : '=';
        *p++ = (i + 2 < len) ? t[v & 0x3F] : '=';
    }
    *p = 0;
    return out;
}

/* --- handshake --- */

var ws_compute_accept(const char *key) {
    char input[256];
    uint8_t digest[20];
    char *b64;
    var out;
    if (!key) return abs_new_error("ws_compute_accept requires a key");
    snprintf(input, sizeof(input), "%s%s", key, WS_MAGIC);
    sha1_bytes(input, strlen(input), digest);
    b64 = b64_encode(digest, 20);
    if (!b64) return abs_new_error("Out of memory");
    out = abs_new_str(b64);
    free(b64);
    return out;
}

bool ws_accept(abs_socket client_fd, const char *request_str) {
    const char *key_start;
    char key[128];
    size_t i;
    var accept;
    char resp[512];
    int sent;
    if (!request_str) return false;
    key_start = strstr(request_str, "Sec-WebSocket-Key: ");
    if (!key_start) return false;
    key_start += 19;
    i = 0;
    while (key_start[i] && key_start[i] != '\r' && key_start[i] != '\n' &&
           i < sizeof(key) - 1) {
        key[i] = key_start[i];
        i++;
    }
    key[i] = 0;
    accept = ws_compute_accept(key);
    if (!accept || accept->type != ABS_STR) return false;

    snprintf(resp, sizeof(resp),
             "HTTP/1.1 101 Switching Protocols\r\n"
             "Upgrade: websocket\r\n"
             "Connection: Upgrade\r\n"
             "Sec-WebSocket-Accept: %s\r\n\r\n",
             accept->val.s);
#ifdef _WIN32
    sent = (int)send((SOCKET)client_fd, resp, (int)strlen(resp), 0);
#else
    sent = (int)send((int)client_fd, resp, strlen(resp), 0);
#endif
    return sent > 0;
}

/* --- framing --- */

size_t ws_encode_frame(char *out, size_t cap, const char *msg) {
    size_t len, hdr;
    if (!out || !msg) return 0;
    len = strlen(msg);
    hdr = len <= 125 ? 2 : (len <= 65535 ? 4 : 10);
    if (cap < hdr + len) return 0;
    out[0] = (char)0x81; /* FIN + text opcode */
    if (len <= 125) {
        out[1] = (char)len;
    } else if (len <= 65535) {
        out[1] = 126;
        out[2] = (char)((len >> 8) & 0xFF);
        out[3] = (char)(len & 0xFF);
    } else {
        uint64_t L = (uint64_t)len;
        int i;
        out[1] = 127;
        for (i = 0; i < 8; i++) out[2 + i] = (char)((L >> (8 * (7 - i))) & 0xFF);
    }
    memcpy(out + hdr, msg, len);
    return hdr + len;
}

long ws_decode_frame(const char *buf, size_t buf_len, char *out,
                     size_t out_cap) {
    const uint8_t *p;
    const uint8_t *mask;
    uint64_t len;
    size_t off;
    int masked;
    size_t i;
    if (!buf || buf_len < 2 || !out) return -1;
    p = (const uint8_t *)buf;
    masked = (p[1] & 0x80) != 0;

    if ((p[1] & 0x7F) == 126) {
        if (buf_len < 4) return -1;
        len = ((uint64_t)p[2] << 8) | p[3];
        off = 4;
    } else if ((p[1] & 0x7F) == 127) {
        if (buf_len < 10) return -1;
        len = 0;
        for (i = 0; i < 8; i++) len = (len << 8) | p[2 + i];
        off = 10;
    } else {
        len = p[1] & 0x7F;
        off = 2;
    }

    mask = NULL;
    if (masked) {
        if (buf_len < off + 4) return -1;
        mask = p + off;
        off += 4;
    }
    if (len > out_cap || buf_len < off + (size_t)len) return -1;

    if (masked) {
        for (i = 0; i < len; i++) out[i] = (char)(p[off + i] ^ mask[i % 4]);
    } else {
        memcpy(out, p + off, (size_t)len);
    }
    return (long)len;
}

void ws_send(abs_socket fd, const char *msg) {
    size_t len, frame_len, written;
    char *frame;
    if (!msg) return;
    len = strlen(msg);
    frame_len = len + (len <= 125 ? 2 : (len <= 65535 ? 4 : 10));
    frame = (char *)malloc(frame_len);
    if (!frame) return;
    written = ws_encode_frame(frame, frame_len, msg);
    if (written) {
#ifdef _WIN32
        send((SOCKET)fd, frame, (int)written, 0);
#else
        send((int)fd, frame, written, 0);
#endif
    }
    free(frame);
}

static int ws_read_full(abs_socket fd, char *buf, size_t len) {
    size_t got = 0;
    while (got < len) {
#ifdef _WIN32
        int n = (int)recv((SOCKET)fd, buf + got, (int)(len - got), 0);
#else
        int n = (int)recv((int)fd, buf + got, len - got, 0);
#endif
        if (n <= 0) return -1;
        got += (size_t)n;
    }
    return (int)got;
}

var ws_recv(abs_socket fd) {
    uint8_t hdr[2];
    uint8_t ext[8];
    uint8_t mask[4] = { 0, 0, 0, 0 };
    uint64_t len;
    int masked;
    size_t extra;
    char *data;
    var s;
    uint64_t i;

    if (ws_read_full(fd, (char *)hdr, 2) < 0) return None;

    if ((hdr[1] & 0x7F) == 126) {
        extra = 2;
    } else if ((hdr[1] & 0x7F) == 127) {
        extra = 8;
    } else {
        extra = 0;
    }
    if (extra) {
        if (ws_read_full(fd, (char *)ext, extra) < 0) return None;
        if (extra == 2) {
            len = ((uint64_t)ext[0] << 8) | ext[1];
        } else {
            len = 0;
            for (i = 0; i < 8; i++) len = (len << 8) | ext[i];
        }
    } else {
        len = hdr[1] & 0x7F;
    }

    masked = (hdr[1] & 0x80) != 0;
    if (masked) {
        if (ws_read_full(fd, (char *)mask, 4) < 0) return None;
    }

    if (len > WS_MAX_FRAME) return abs_new_error("ws_recv frame too large");
    data = (char *)malloc((size_t)len + 1);
    if (!data) return abs_new_error("Out of memory");
    if (len && ws_read_full(fd, data, (size_t)len) < 0) {
        free(data);
        return None;
    }
    if (masked) {
        for (i = 0; i < len; i++) data[i] ^= (char)mask[i % 4];
    }
    data[len] = 0;
    s = abs_new_str(data);
    free(data);
    return s;
}
