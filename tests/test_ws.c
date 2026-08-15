#include "abscom/abs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
            return 1;                                                        \
        }                                                                    \
    } while (0)

/* Round-trip: encode a server frame, then decode it back to the message. */
static int round_trip(const char *msg, size_t msg_len) {
    size_t frame_len = msg_len + (msg_len <= 125 ? 2 : (msg_len <= 65535 ? 4 : 10));
    char *frame = (char *)malloc(frame_len);
    char *out = (char *)malloc(msg_len + 1);
    size_t written;
    long n;
    int ok;
    if (!frame || !out) return 1;
    written = ws_encode_frame(frame, frame_len, msg);
    if (written == 0) { free(frame); free(out); return 1; }
    n = ws_decode_frame(frame, written, out, msg_len + 1);
    ok = (n == (long)msg_len && memcmp(out, msg, msg_len) == 0);
    free(frame);
    free(out);
    return ok ? 0 : 1;
}

int main(void) {
    abs_init();

    /* RFC 6455 section 1.3 worked example. */
    var accept = ws_compute_accept("dGhlIHNhbXBsZSBub25jZQ==");
    CHECK(accept != NULL && accept->type == ABS_STR);
    CHECK(strcmp(accept->val.s, "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=") == 0);

    /* Another key, checked against the independent "websocket-key-check" tools. */
    accept = ws_compute_accept("x3JJHMbDL1EzLkh9GBhXDw==");
    CHECK(accept != NULL && accept->type == ABS_STR);
    CHECK(strcmp(accept->val.s, "HSmrc0sMlYUkAGmm5OPpG2HaGWk=") == 0);

    /* ws_compute_accept(NULL) -> ABS_ERROR. */
    var bad = ws_compute_accept(NULL);
    CHECK(bad != NULL && bad->type == ABS_ERROR);

    /* ws_accept returns false without touching the socket when the request
     * carries no Sec-WebSocket-Key header (or is NULL). */
    CHECK(ws_accept((abs_socket)(-1), "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n") == false);
    CHECK(ws_accept((abs_socket)(-1), NULL) == false);

    /* --- frame encoding: header bytes for each length class --- */
    {
        char frame[16];
        size_t n;

        /* 7-bit length. */
        n = ws_encode_frame(frame, sizeof(frame), "Hello");
        CHECK(n == 7);
        CHECK((unsigned char)frame[0] == 0x81);
        CHECK((unsigned char)frame[1] == 5);
        CHECK(memcmp(frame + 2, "Hello", 5) == 0);

        /* 16-bit extended length. */
        {
            char msg[201];
            char big_frame[4 + 200];
            memset(msg, 'a', 200);
            msg[200] = 0;
            n = ws_encode_frame(big_frame, sizeof(big_frame), msg);
            CHECK(n == 4 + 200);
            CHECK((unsigned char)big_frame[1] == 126);
            CHECK((unsigned char)big_frame[2] == 0x00);
            CHECK((unsigned char)big_frame[3] == 200);
        }

        /* 64-bit extended length needs 10 header bytes. */
        {
            char msg[65537];
            memset(msg, 'b', 65536);
            msg[65536] = 0;
            n = ws_encode_frame(frame, sizeof(frame), msg);
            CHECK(n == 0); /* frame buffer too small for a 65 KiB message */
        }
    }

    /* --- frame decoding: masked client frame (RFC 6455 section 5.7) --- */
    {
        const unsigned char mask[4] = { 0x37, 0xfa, 0x21, 0x3d };
        const char payload[] = "Hello";
        char frame[16];
        char out[16];
        long n;
        size_t i;
        frame[0] = (char)0x81;
        frame[1] = (char)(0x80 | 5); /* FIN + text, masked */
        memcpy(frame + 2, mask, 4);
        for (i = 0; i < 5; i++) frame[6 + i] = (char)(payload[i] ^ mask[i % 4]);
        n = ws_decode_frame(frame, 11, out, sizeof(out));
        CHECK(n == 5);
        CHECK(memcmp(out, "Hello", 5) == 0);
    }

    /* --- frame decoding: error paths --- */
    CHECK(ws_decode_frame(NULL, 10, NULL, 0) == -1);
    CHECK(ws_decode_frame("\x81", 1, NULL, 0) == -1);              /* truncated header */
    CHECK(ws_decode_frame("\x81\x7e", 2, NULL, 0) == -1);          /* missing ext length */
    CHECK(ws_decode_frame("\x81\x7f\x00", 3, NULL, 0) == -1);      /* missing 64-bit len */
    {
        char out[4];
        CHECK(ws_decode_frame("\x81\x05Hello", 7, out, sizeof(out)) == -1); /* small cap */
    }
    {
        char frame[4] = { (char)0x81, 126, 0x00, 0x10 };           /* 16-byte payload */
        CHECK(ws_decode_frame(frame, sizeof(frame), NULL, 0) == -1); /* truncated */
    }
    {
        char masked[4] = { (char)0x81, (char)0x85, 1, 2 };         /* mask declared */
        CHECK(ws_decode_frame(masked, sizeof(masked), NULL, 0) == -1); /* missing mask */
    }

    /* --- round trips --- */
    CHECK(round_trip("Hello", 5) == 0);
    CHECK(round_trip("", 0) == 0);
    CHECK(round_trip("x", 1) == 0);
    {
        char msg[201];
        memset(msg, 'A', 200);
        msg[200] = 0;
        CHECK(round_trip(msg, 200) == 0);
    }
    {
        char msg[70001];
        memset(msg, 'B', 70000);
        msg[70000] = 0;
        CHECK(round_trip(msg, 70000) == 0);
    }

    abs_cleanup();
    return 0;
}
