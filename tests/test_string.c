#include "abscom/abs_string.h"

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

int main(void) {
    abs_string_t s;
    CHECK(abs_string_init(&s) == 0);
    CHECK(abs_string_len(&s) == 0);
    CHECK(strcmp(abs_string_c_str(&s), "") == 0);

    CHECK(abs_string_append_cstr(&s, "abc") == 0);
    CHECK(abs_string_append_char(&s, 'd') == 0);
    CHECK(abs_string_append_n(&s, "efgh", 4) == 0);
    CHECK(abs_string_len(&s) == 8);
    CHECK(strcmp(abs_string_c_str(&s), "abcdefgh") == 0);

    CHECK(abs_string_append_fmt(&s, "-%d", 42) == 0);
    CHECK(strcmp(abs_string_c_str(&s), "abcdefgh-42") == 0);

    CHECK(abs_string_set_cstr(&s, "xy") == 0);
    CHECK(abs_string_len(&s) == 2);
    CHECK(strcmp(abs_string_c_str(&s), "xy") == 0);

    char *taken = abs_string_take(&s);
    CHECK(taken != NULL && strcmp(taken, "xy") == 0);
    free(taken);
    CHECK(abs_string_len(&s) == 0);

    abs_string_t s2;
    CHECK(abs_string_init_cstr(&s2, "hello world") == 0);
    CHECK(abs_string_len(&s2) == 11);
    CHECK(abs_string_shrink_to_fit(&s2) == 0);
    abs_string_destroy(&s2);

    abs_string_destroy(&s);
    printf("test_string: OK\n");
    return 0;
}
