#include "abscom/abs_dynarray.h"

#include <stdio.h>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
            return 1;                                                        \
        }                                                                    \
    } while (0)

int main(void) {
    abs_dynarray_t arr;
    CHECK(abs_dynarray_init(&arr, sizeof(int)) == 0);
    CHECK(abs_dynarray_len(&arr) == 0);
    CHECK(abs_dynarray_data(&arr) == NULL);

    for (int i = 0; i < 1000; i++) CHECK(abs_dynarray_push(&arr, &i) == 0);
    CHECK(abs_dynarray_len(&arr) == 1000);
    CHECK(abs_dynarray_cap(&arr) >= 1000);

    for (int i = 0; i < 1000; i++) {
        int *v = (int *)abs_dynarray_at(&arr, (size_t)i);
        CHECK(v != NULL && *v == i);
    }
    CHECK(abs_dynarray_at(&arr, 1000) == NULL);
    CHECK(abs_dynarray_at_const(&arr, 500) != NULL);

    abs_dynarray_pop(&arr);
    CHECK(abs_dynarray_len(&arr) == 999);

    CHECK(abs_dynarray_resize(&arr, 2000) == 0);
    CHECK(abs_dynarray_len(&arr) == 2000);
    {
        int *e = (int *)abs_dynarray_at(&arr, 1500);
        CHECK(e != NULL && *e == 0);
    }
    CHECK(abs_dynarray_reserve(&arr, 5000) == 0);
    CHECK(abs_dynarray_cap(&arr) >= 5000);

    abs_dynarray_clear(&arr);
    CHECK(abs_dynarray_len(&arr) == 0);
    abs_dynarray_destroy(&arr);

    printf("test_dynarray: OK\n");
    return 0;
}
