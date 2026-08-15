#include "abscom/abs_hash.h"

#include <stdio.h>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
            return 1;                                                        \
        }                                                                    \
    } while (0)

int main(void) {
    CHECK(abs_hash_fnv1a32("hello", 5) == abs_hash_fnv1a32("hello", 5));
    CHECK(abs_hash_fnv1a32("hello", 5) != abs_hash_fnv1a32("hallo", 5));
    CHECK(abs_hash_fnv1a32("hello", 5) == 0x4f9f2cabu);

    CHECK(abs_hash_fnv1a64("hello", 5) == abs_hash_fnv1a64("hello", 5));
    CHECK(abs_hash_fnv1a64("hello", 5) != abs_hash_fnv1a64("hallo", 5));
    CHECK(abs_hash_fnv1a64_str("hello") == abs_hash_fnv1a64("hello", 5));
    CHECK(abs_hash_fnv1a64_str(NULL) == 0);

    CHECK(abs_hash_djb2("hello") == abs_hash_djb2("hello"));
    CHECK(abs_hash_djb2("") == 5381u);

    printf("test_hash: OK\n");
    return 0;
}
