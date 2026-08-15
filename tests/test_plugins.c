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

static var plugin_double(var x) {
    if (x && x->type == ABS_INT) return v(x->val.i * 2);
    return None;
}

int main(void) {
    abs_init();

    /* A missing library reports an error. */
    var bad = load_library("no_such_library_abscom_9");
    CHECK(bad != NULL);
    CHECK(is_err(bad));
    CHECK(strstr(bad->val.error_msg, "Could not load library") != NULL);

    /* Calling functions on a non-library object returns None. */
    var not_lib = v(42);
    CHECK(is_none(call_lib_func(not_lib, "anything", None)));

    /* Type introspection for the ABS_LIB type. */
    var failed_again = load_library("also_missing");
    CHECK(is_err(failed_again));
    CHECK(strcmp(type(failed_again)->val.s, "<class 'error'>") == 0);

    /* plugin_double works as a plain wrapped function. */
    var wrapped = make_func(plugin_double);
    var out = call_func(wrapped, v(21));
    CHECK(out != NULL && out->type == ABS_INT && out->val.i == 42);

    abs_cleanup();
    return 0;
}
