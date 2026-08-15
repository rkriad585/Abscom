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

static void risky_operation(int x) {
    if (x < 0) throw("Negative value not allowed!");
    if (x == 0) throw("Division by zero!");
}

int main(void) {
    abs_init();

    /* No exception thrown: catch must not run. */
    volatile int caught = 0;
    var e = None;
    try {
        risky_operation(5);
    } catch (e) {
        caught = 1;
    }
    end_try;
    CHECK(caught == 0);

    /* An exception thrown by a nested call is caught and carries the message. */
    caught = 0;
    try {
        risky_operation(-1);
    } catch (e) {
        caught = 1;
        CHECK(is_err(e));
        CHECK(strstr(e->val.error_msg, "Negative") != NULL);
    }
    end_try;
    CHECK(caught == 1);

    /* The remaining statements after a throw are skipped. */
    caught = 0;
    volatile int after_throw = 0;
    try {
        risky_operation(10);
        risky_operation(-5);
        after_throw = 1; /* must never run */
    } catch (e) {
        caught = 1;
    }
    end_try;
    CHECK(caught == 1);
    CHECK(after_throw == 0);

    /* Nested try blocks unwind to the innermost handler. */
    caught = 0;
    try {
        try {
            throw("inner");
        } catch (e) {
            caught = 1;
            CHECK(strstr(e->val.error_msg, "inner") != NULL);
        }
        end_try;
        throw("outer");
    } catch (e) {
        CHECK(strstr(e->val.error_msg, "outer") != NULL);
    }
    end_try;
    CHECK(caught == 1);

    /* close_resource() closes a file and prevents a double close. */
    FILE *raw = fopen("test_except_tmp.txt", "w");
    CHECK(raw != NULL);
    var f = abs_new_file(raw);
    close_resource(f);
    CHECK(f->val.file_ptr == NULL);

    /* with() writes through the file handle and cleans up after the body. */
    with(fp, abs_new_file(fopen("test_except_tmp.txt", "w"))) {
        if (fp->val.file_ptr) {
            fprintf(fp->val.file_ptr, "hello from with");
        } else {
            CHECK(0);
        }
    }
    FILE *verify = fopen("test_except_tmp.txt", "r");
    CHECK(verify != NULL);
    fclose(verify);
    remove("test_except_tmp.txt");

    abs_cleanup();
    return 0;
}
