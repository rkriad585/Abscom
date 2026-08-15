#include "abscom/abs.h"

#include <stdio.h>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static var compute_factorial(var input) {
    return factorial(input);
}

int main(void) {
    abs_init();

    var t1 = thread_start(compute_factorial, v(5));
    CHECK(t1 != NULL && t1->type == ABS_THREAD);

    var r1 = thread_join(t1);
    CHECK(r1 != NULL && r1->type == ABS_INT && r1->val.i == 120);

    var threads[4];
    long inputs[4] = {3, 4, 5, 6};
    for (int i = 0; i < 4; i++) {
        threads[i] = thread_start(compute_factorial, v(inputs[i]));
        CHECK(threads[i] != NULL && threads[i]->type == ABS_THREAD);
    }
    long expected[4] = {6, 24, 120, 720};
    for (int i = 0; i < 4; i++) {
        var r = thread_join(threads[i]);
        CHECK(r != NULL && r->type == ABS_INT && r->val.i == expected[i]);
    }

    CHECK(is_err(thread_join(v(1))));
    CHECK(is_err(thread_start(NULL, v(1))));

    abs_cleanup();
    return 0;
}
