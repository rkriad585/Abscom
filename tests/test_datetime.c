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

    var now = datetime_now();
    CHECK(now->type == ABS_TIME);

    var d = strftime_val("%Y-%m-%d", now);
    CHECK(d->type == ABS_STR);
    CHECK(strlen(d->val.s) == 10);
    CHECK(d->val.s[4] == '-');
    CHECK(d->val.s[7] == '-');

    var full = strftime_val("%Y-%m-%d %H:%M:%S", now);
    CHECK(full->type == ABS_STR);
    CHECK(strlen(full->val.s) == 19);

    CHECK(is_none(strftime_val("%Y", v(5))));

    var t1 = timedelta(0, 0);
    var t2 = timedelta(1, 0);
    CHECK(t1->type == ABS_TIME && t2->type == ABS_TIME);
    var d1 = strftime_val("%Y-%m-%d", t1);
    var d2 = strftime_val("%Y-%m-%d", t2);
    CHECK(strcmp(d1->val.s, d2->val.s) != 0);

    var t3 = timedelta(0, 86400);
    var d3 = strftime_val("%Y-%m-%d", t3);
    CHECK(strcmp(d1->val.s, d3->val.s) != 0);

    abs_cleanup();
    return 0;
}
