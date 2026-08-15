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

int main(void) {
    abs_init();

    CHECK(strcmp(base64_encode(v("hello"))->val.s, "aGVsbG8=") == 0);
    CHECK(strcmp(base64_encode(v(""))->val.s, "") == 0);
    CHECK(strcmp(base64_encode(v("a"))->val.s, "YQ==") == 0);
    CHECK(strcmp(base64_encode(v("ab"))->val.s, "YWI=") == 0);
    CHECK(strcmp(base64_encode(v("abc"))->val.s, "YWJj") == 0);
    CHECK(is_none(base64_encode(v(42))));

    var u = uuid4();
    CHECK(u->type == ABS_STR);
    CHECK(strlen(u->val.s) == 36);
    CHECK(u->val.s[8] == '-' && u->val.s[13] == '-' && u->val.s[18] == '-' &&
          u->val.s[23] == '-');
    CHECK(u->val.s[14] == '4'); /* version nibble */
    CHECK(u->val.s[19] == '8' || u->val.s[19] == '9' ||
          u->val.s[19] == 'a' || u->val.s[19] == 'b');

    var u2 = uuid4();
    CHECK(strcmp(u->val.s, u2->val.s) != 0);

    abs_cleanup();
    return 0;
}
