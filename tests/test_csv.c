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

    const char *path = "test_csv_tmp.csv";

    var rows = abs_new_list();
    var r1 = abs_new_list();
    append(r1, v(1));
    append(r1, v(2.5));
    append(r1, v("hello"));
    append(rows, r1);
    var r2 = abs_new_list();
    append(r2, v(42));
    append(r2, v("world"));
    append(rows, r2);

    csv_write(path, rows);

    var back = csv_read(path);
    CHECK(!is_err(back));
    CHECK(back->type == ABS_LIST && back->val.list.size == 2);

    var row0 = get(back, 0);
    CHECK(row0->type == ABS_LIST && row0->val.list.size == 3);
    CHECK(get(row0, 0)->type == ABS_INT && get(row0, 0)->val.i == 1);
    CHECK(get(row0, 1)->type == ABS_FLOAT && get(row0, 1)->val.f == 2.5);
    CHECK(get(row0, 2)->type == ABS_STR && strcmp(get(row0, 2)->val.s, "hello") == 0);

    var row1 = get(back, 1);
    CHECK(get(row1, 0)->type == ABS_INT && get(row1, 0)->val.i == 42);
    CHECK(get(row1, 1)->type == ABS_STR && strcmp(get(row1, 1)->val.s, "world") == 0);

    remove(path);

    var missing = csv_read("test_csv_nope.csv");
    CHECK(is_err(missing));

    abs_cleanup();
    return 0;
}
