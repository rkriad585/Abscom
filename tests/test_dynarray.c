#include "abscom/ac_dynarray.h"

#include <stdio.h>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
            return 1;                                                        \
        }                                                                    \
    } while (0)

int main(void) {
    ac_dynarray_t arr;
    CHECK(ac_dynarray_init(&arr, sizeof(int)) == 0);
    CHECK(ac_dynarray_len(&arr) == 0);
    CHECK(ac_dynarray_data(&arr) == NULL);

    for (int i = 0; i < 1000; i++) CHECK(ac_dynarray_push(&arr, &i) == 0);
    CHECK(ac_dynarray_len(&arr) == 1000);
    CHECK(ac_dynarray_cap(&arr) >= 1000);

    for (int i = 0; i < 1000; i++) {
        int *v = (int *)ac_dynarray_at(&arr, (size_t)i);
        CHECK(v != NULL && *v == i);
    }
    CHECK(ac_dynarray_at(&arr, 1000) == NULL);
    CHECK(ac_dynarray_at_const(&arr, 500) != NULL);

    ac_dynarray_pop(&arr);
    CHECK(ac_dynarray_len(&arr) == 999);

    CHECK(ac_dynarray_resize(&arr, 2000) == 0);
    CHECK(ac_dynarray_len(&arr) == 2000);
    {
        int *e = (int *)ac_dynarray_at(&arr, 1500);
        CHECK(e != NULL && *e == 0);
    }
    CHECK(ac_dynarray_reserve(&arr, 5000) == 0);
    CHECK(ac_dynarray_cap(&arr) >= 5000);

    ac_dynarray_clear(&arr);
    CHECK(ac_dynarray_len(&arr) == 0);
    ac_dynarray_destroy(&arr);

    printf("test_dynarray: OK\n");
    return 0;
}
