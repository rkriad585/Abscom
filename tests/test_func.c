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

static int wrap_calls = 0;
static int inner_calls = 0;

static var inner_square(var x) {
    inner_calls++;
    return v(x->val.i * x->val.i);
}

/* Target-aware wrapper: pre/post logic around call_func(target, args). */
static var timing_wrapper(var target, var args) {
    wrap_calls++;
    var result = call_func(target, args);
    return v(result->val.i + 1);
}

/* Target-aware wrapper that never calls the target (short-circuits). */
static var block_wrapper(var target, var args) {
    (void)target;
    (void)args;
    return v("BLOCKED");
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

    /* def() = make_func plus a display name; func_name() reads it back. */
    var named = def(inner_square, "square");
    CHECK(named->type == ABS_FUNC);
    CHECK(is_none(func_name(f))); /* unnamed make_func() -> None */
    var nm = func_name(named);
    CHECK(nm != NULL && nm->type == ABS_STR && strcmp(nm->val.s, "square") == 0);
    CHECK(call_func(named, v(6))->val.i == 36);
    CHECK(is_err(func_name(v(42))));

    /* decorate_func(): target-aware wrapper gets (target, args) and can run
     * logic around the original before/after calling it back. */
    wrap_calls = 0;
    inner_calls = 0;
    var timed = decorate_func(named, timing_wrapper);
    CHECK(timed->type == ABS_FUNC);
    CHECK(func_meta(timed) == named);
    CHECK(func_name(timed)->type == ABS_STR); /* inherits the target's name */
    CHECK(strcmp(func_name(timed)->val.s, "square") == 0);
    CHECK(call_func(timed, v(5))->val.i == 26); /* 5*5 then +1 */
    CHECK(inner_calls == 1);
    CHECK(wrap_calls == 1);
    CHECK(call_func(timed, v(3))->val.i == 10); /* 3*3 then +1 */
    CHECK(inner_calls == 2);
    /* The original is still callable directly, untouched. */
    CHECK(call_func(named, v(5))->val.i == 25);
    CHECK(inner_calls == 3);
    CHECK(is_err(decorate_func(v(42), timing_wrapper)));

    /* A wrapper may choose not to call the target at all. */
    var blocker = decorate_func(named, block_wrapper);
    var blocked = call_func(blocker, v(9));
    CHECK(blocked != NULL && blocked->type == ABS_STR &&
          strcmp(blocked->val.s, "BLOCKED") == 0);
    CHECK(inner_calls == 3); /* target never ran */

    abs_cleanup();
    return 0;
}
