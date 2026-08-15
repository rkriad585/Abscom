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

static int check_sha(const char *input, const char *expected) {
    var h = sha256(input);
    CHECK(h != NULL && h->type == ABS_STR);
    CHECK(strcmp(h->val.s, expected) == 0);
    return 0;
}

int main(void) {
    abs_init();

    /* FIPS 180-4 known-answer tests. */
    CHECK(check_sha("", "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855") == 0);
    CHECK(check_sha("abc", "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad") == 0);
    CHECK(check_sha("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
                    "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1") == 0);
    CHECK(check_sha("The quick brown fox jumps over the lazy dog",
                    "d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592") == 0);

    /* sha256(NULL) is an ABS_ERROR, not a crash. */
    var bad = sha256(NULL);
    CHECK(bad != NULL && bad->type == ABS_ERROR);

    /* HMAC-SHA-256 known-answer tests (RFC 4231 cases 2 and 6). */
    var h = hmac_sha256("Jefe", "what do ya want for nothing?");
    CHECK(h != NULL && h->type == ABS_STR);
    CHECK(strcmp(h->val.s,
                 "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843") == 0);

    h = hmac_sha256("key", "The quick brown fox jumps over the lazy dog");
    CHECK(h != NULL && h->type == ABS_STR);
    CHECK(strcmp(h->val.s,
                 "f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8") == 0);

    /* A key longer than the 64-byte block is pre-hashed (RFC 2104). */
    {
        char long_key[132];
        memset(long_key, (char)0xaa, 131);
        long_key[131] = 0;
        h = hmac_sha256(long_key, "Test Using Larger Than Block-Size Key - Hash Key First");
        CHECK(h != NULL && h->type == ABS_STR);
        CHECK(strcmp(h->val.s,
                     "60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54") == 0);
    }

    /* hmac_sha256(NULL, ...) is an ABS_ERROR. */
    var badh = hmac_sha256(NULL, "msg");
    CHECK(badh != NULL && badh->type == ABS_ERROR);

    /* The hex output is deterministic and 64 chars long. */
    h = hmac_sha256("secret", "message");
    CHECK(h != NULL && strlen(h->val.s) == 64);
    var again = hmac_sha256("secret", "message");
    CHECK(strcmp(h->val.s, again->val.s) == 0);

    abs_cleanup();
    return 0;
}
