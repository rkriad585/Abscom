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

    var cwd = getcwd_val();
    CHECK(cwd->type == ABS_STR && strlen(cwd->val.s) > 0);

    var joined = path_join(cwd, v("probe.txt"));
    CHECK(joined->type == ABS_STR);
    CHECK(strstr(joined->val.s, "probe.txt") != NULL);

    CHECK(path_exists(joined)->val.b == false);

    FILE *f = fopen(joined->val.s, "w");
    CHECK(f != NULL);
    fprintf(f, "x");
    fclose(f);

    CHECK(path_exists(joined)->val.b == true);

    remove(joined->val.s);
    CHECK(path_exists(joined)->val.b == false);

    CHECK(is_err(path_join(v("a"), v(1))));

    abs_cleanup();
    return 0;
}
