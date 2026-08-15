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

static int heavy_calls = 0;

static var heavy_calc(var num) {
    heavy_calls++;
    return v(num->val.i * 2);
}

static var greet(var name) {
    (void)name;
    return v("Hello");
}

static var shout_wrapper(var s) {
    (void)s;
    return v("SHOUT");
}

int main(void) {
    abs_init();

    /* Plain wrapped function. */
    var f = make_func(heavy_calc);
    CHECK(f != NULL && f->type == ABS_FUNC);
    CHECK(strcmp(type(f)->val.s, "<class 'function'>") == 0);
    CHECK(call_func(f, v(5))->val.i == 10);
    CHECK(is_err(call_func(v(42), v(1))));

    /* Memoization: the body runs once per distinct argument. */
    heavy_calls = 0;
    var calc = memoize(heavy_calc);
    CHECK(calc->type == ABS_FUNC);
    CHECK(calc->val.func.cache != NULL);
    CHECK(call_memoized(calc, v(100))->val.i == 200);
    CHECK(heavy_calls == 1);
    CHECK(call_memoized(calc, v(100))->val.i == 200);
    CHECK(heavy_calls == 1); /* cache hit: body skipped */
    CHECK(call_memoized(calc, v(7))->val.i == 14);
    CHECK(heavy_calls == 2);

    /* memoize + call_func dispatch through the same cache. */
    CHECK(call_func(calc, v(100))->val.i == 200);
    CHECK(heavy_calls == 2);

    /* Strings also work as cache keys. */
    var greet_m = memoize(greet);
    CHECK(call_memoized(greet_m, v("Bob"))->type == ABS_STR);
    CHECK(call_memoized(greet_m, v("Bob"))->type == ABS_STR);

    /* decorate() swaps the body and keeps the original in metadata. */
    var original = make_func(greet);
    var decorated = decorate(original, shout_wrapper);
    CHECK(decorated->type == ABS_FUNC);
    CHECK(func_meta(decorated) == original);
    CHECK(is_none(func_meta(original))); /* plain funcs have no metadata */
    var out = call_func(decorated, v("x"));
    CHECK(out != NULL && out->type == ABS_STR && strcmp(out->val.s, "SHOUT") == 0);

    abs_cleanup();
    return 0;
}
