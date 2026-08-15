#include "abscom/abs.h"

#include <stdio.h>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
            return 1;                                                        \
        }                                                                    \
    } while (0)

int main(void) {
    abs_init();

    var g = range_gen(0, 5, 1);
    CHECK(g->type == ABS_GENERATOR);
    CHECK(next(g)->val.i == 0);
    CHECK(next(g)->val.i == 1);
    CHECK(next(g)->val.i == 2);
    CHECK(next(g)->val.i == 3);
    CHECK(next(g)->val.i == 4);
    CHECK(is_none(next(g)));
    CHECK(is_none(next(g))); /* stays None once exhausted */

    var g2 = range_gen(0, 10, 2);
    CHECK(next(g2)->val.i == 0);
    CHECK(next(g2)->val.i == 2);
    CHECK(next(g2)->val.i == 4);

    var g3 = range_gen(5, 0, -1);
    CHECK(next(g3)->val.i == 5);
    CHECK(next(g3)->val.i == 4);
    CHECK(next(g3)->val.i == 3);
    CHECK(next(g3)->val.i == 2);
    CHECK(next(g3)->val.i == 1);
    CHECK(is_none(next(g3)));

    var empty = range_gen(0, 0, 1);
    CHECK(is_none(next(empty)));

    CHECK(is_err(next(v(1))));
    CHECK(is_err(range_gen(0, 5, 0)));

    abs_cleanup();
    return 0;
}
