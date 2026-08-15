#include "abscom/abs.h"

#include <stdio.h>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static long next_int(var iter) {
    var item = iter_next(iter);
    if (is_none(item)) return -1;
    return item->val.i;
}

int main(void) {
    abs_init();

    var a = abs_new_list();
    append(a, v(1));
    append(a, v(2));
    var b = abs_new_list();
    append(b, v(10));
    append(b, v(20));

    /* chain(A, B): all of A, then all of B, then None. */
    var c = chain(a, b);
    CHECK(c != NULL && c->type == ABS_ITERATOR);
    CHECK(next_int(c) == 1);
    CHECK(next_int(c) == 2);
    CHECK(next_int(c) == 10);
    CHECK(next_int(c) == 20);
    CHECK(is_none(iter_next(c)));
    CHECK(is_none(iter_next(c))); /* stays exhausted */

    /* chain with a single source still drains it. */
    var only = chain(a, None);
    CHECK(next_int(only) == 1);
    CHECK(next_int(only) == 2);
    CHECK(is_none(iter_next(only)));

    /* cycle(A): repeats forever. */
    var cy = cycle(a);
    CHECK(cy != NULL && cy->type == ABS_ITERATOR);
    CHECK(next_int(cy) == 1);
    CHECK(next_int(cy) == 2);
    CHECK(next_int(cy) == 1);
    CHECK(next_int(cy) == 2);
    CHECK(next_int(cy) == 1);

    /* iter_next on a non-list source is treated as empty, so the chain just
     * yields the second list, and an empty cycle yields None, not a crash. */
    var bad = chain(v(99), b);
    CHECK(next_int(bad) == 10);
    CHECK(next_int(bad) == 20);
    CHECK(is_none(iter_next(bad)));
    var empty_cy = cycle(v(99));
    CHECK(is_none(iter_next(empty_cy)));

    abs_cleanup();
    return 0;
}
