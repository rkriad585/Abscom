#include "abscom/abs.h"

#include <stdio.h>

static var square_slow(var x) {
    printf(" [computing %ld^2] ", x->val.i);
    return v(x->val.i * x->val.i);
}

int main(void) {
    abs_init();

    /* 1. CRYPTO */
    print(v("--- 1. SHA-256 & HMAC ---"));
    var h = sha256("password");
    print(v("SHA256(\"password\"):"), h);
    var hm = hmac_sha256("secret", "message");
    print(v("HMAC(secret, message):"), hm);

    /* 2. WEBSOCKET HANDSHAKE (RFC 6455 worked example) */
    print(v("\n--- 2. WebSocket handshake ---"));
    var accept = ws_compute_accept("dGhlIHNhbXBsZSBub25jZQ==");
    print(v("Sec-WebSocket-Accept:"), accept);

    /* 3. FRAME ENCODE / DECODE (wire-format simulation, no live socket) */
    print(v("\n--- 3. Frame encode/decode ---"));
    char frame[32];
    char out[32];
    size_t n = ws_encode_frame(frame, sizeof(frame), "Hello");
    long len = ws_decode_frame(frame, n, out, sizeof(out));
    printf("  Frame: %02x %02x ... -> payload (%ld): %.*s\n",
           (unsigned char)frame[0], (unsigned char)frame[1], len, (int)len, out);
    printf("  (ws_accept / ws_send / ws_recv do the same on a connected socket)\n");

    /* 4. SHUFFLING */
    print(v("\n--- 4. Shuffling ---"));
    var deck = List();
    for (int i = 1; i <= 8; i++) append(deck, v(i));
    print(v("Original:"), deck);
    riffle_shuffle(deck);
    print(v("Riffle:  "), deck);
    fisher_yates(deck);
    print(v("FY:      "), deck);

    /* 5. REPEAT ITERATOR */
    print(v("\n--- 5. repeat() iterator ---"));
    var rep = repeat(v("beep"), 3);
    print(v("Repeat:"), iter_next(rep), iter_next(rep), iter_next(rep),
          iter_next(rep));

    /* 6. MEMOIZATION (metaprogramming) */
    print(v("\n--- 6. Memoization ---"));
    var cached_sq = memoize(square_slow);
    print(v("Call 1:"), call_memoized(cached_sq, v(10)));
    print(v("Call 2:"), call_memoized(cached_sq, v(10)));

    abs_cleanup();
    return 0;
}
