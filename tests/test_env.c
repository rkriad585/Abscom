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

    CHECK(is_none(os_getenv("ABSCOM_VAR_THAT_DOES_NOT_EXIST_XYZ")));

    os_setenv("ABSCOM_TEST_VAR", "hello42");
    var val = os_getenv("ABSCOM_TEST_VAR");
    CHECK(val->type == ABS_STR);
    CHECK(strcmp(val->val.s, "hello42") == 0);

    os_setenv("ABSCOM_TEST_VAR", "updated");
    val = os_getenv("ABSCOM_TEST_VAR");
    CHECK(strcmp(val->val.s, "updated") == 0);

    abs_cleanup();
    return 0;
}
