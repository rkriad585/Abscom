#include "abscom/abs.h"

#include <stdio.h>

/* Helper function that throws on invalid input. */
static void risky_operation(int x) {
    if (x < 0) throw("Negative value not allowed!");
    if (x == 0) throw("Division by zero!");
    print(v("Operation success for"), v(x));
}

int main(void) {
    abs_init();

    /* --- 1. Regex --- */
    print(v("--- 1. Regex ---"));
    var email = v("admin@abscom.org");
    if (re_match(v("^.*@.*$"), email)) {
        print(v("Email format valid:"), email);
    } else {
        print(v("Invalid email"));
    }
    print(v("All 'm' matches:"), re_findall(v("m"), email));
    print(v("Squashed:"), re_sub(v("m"), v("M"), email));

    /* --- 2. Exceptions --- */
    print(v("--- 2. Exceptions ---"));
    var e = None;
    try {
        risky_operation(10); /* OK */
        risky_operation(-5); /* throws */
        risky_operation(0);  /* skipped */
    } catch (e) {
        print(v("Caught exception:"), e);
    }
    end_try; /* REQUIRED: resets the try/catch stack index */

    /* --- 3. Date & time --- */
    print(v("--- 3. Date & time ---"));
    var now = datetime_now();
    print(v("Current time:"), strftime_val("%Y-%m-%d %H:%M:%S", now));
    print(v("Tomorrow:"), strftime_val("%Y-%m-%d", timedelta(1, 0)));

    /* --- 4. Generators (O(1) memory over huge ranges) --- */
    print(v("--- 4. Generators ---"));
    var gen = range_gen(0, 1000000, 2);
    print(v("Gen 1:"), next(gen));
    print(v("Gen 2:"), next(gen));
    print(v("Gen 3:"), next(gen));

    /* --- 5. Context manager --- */
    print(v("--- 5. Context manager ---"));
    with(f, abs_new_file(fopen("test_log.txt", "w"))) {
        if (f->val.file_ptr) {
            fprintf(f->val.file_ptr, "Log entry from Abscom 8.0\n");
            print(v("File written safely."));
        } else {
            print(v("Could not open file."));
        }
    }
    /* f is closed here automatically */

    /* --- 6. Encoding & UUID --- */
    print(v("--- 6. Encoding & UUID ---"));
    var token = uuid4();
    print(v("Generated UUID:"), token);
    print(v("Base64 encoded:"), base64_encode(token));

    /* --- 7. Env vars --- */
    print(v("--- 7. Env vars ---"));
    os_setenv("ABSCOM_MODE", "Pro");
    print(v("Env mode:"), os_getenv("ABSCOM_MODE"));

    abs_cleanup();
    return 0;
}
