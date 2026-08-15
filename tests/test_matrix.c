#include "abscom/abs.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static int close_to(double a, double b) {
    return fabs(a - b) < 1e-9;
}

int main(void) {
    abs_init();

    var A = abs_matrix_new(2, 2);
    CHECK(A != NULL && A->type == ABS_MATRIX);
    CHECK(abs_matrix_rows(A) == 2 && abs_matrix_cols(A) == 2);

    abs_matrix_set(A, 0, 0, 1.0);
    abs_matrix_set(A, 0, 1, 2.0);
    abs_matrix_set(A, 1, 0, 3.0);
    abs_matrix_set(A, 1, 1, 4.0);
    CHECK(close_to(abs_matrix_get(A, 0, 0), 1.0));
    CHECK(close_to(abs_matrix_get(A, 1, 1), 4.0));

    abs_matrix_set(A, 5, 5, 99.0);
    CHECK(close_to(abs_matrix_get(A, 5, 5), 0.0));

    var I = abs_matrix_eye(2);
    CHECK(close_to(abs_matrix_get(I, 0, 0), 1.0));
    CHECK(close_to(abs_matrix_get(I, 0, 1), 0.0));
    CHECK(close_to(abs_matrix_get(I, 1, 0), 0.0));
    CHECK(close_to(abs_matrix_get(I, 1, 1), 1.0));

    var P = abs_matrix_mul(A, I);
    CHECK(!is_err(P));
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            CHECK(close_to(abs_matrix_get(P, i, j), abs_matrix_get(A, i, j)));

    var T = abs_matrix_transpose(A);
    CHECK(close_to(abs_matrix_get(T, 0, 1), 3.0));
    CHECK(close_to(abs_matrix_get(T, 1, 0), 2.0));

    var D2 = abs_matrix_det(A);
    CHECK(!is_err(D2) && close_to(D2->val.f, -2.0));

    var B = abs_matrix_new(3, 3);
    double vals[9] = {6, 1, 1, 4, -2, 5, 2, 8, 7};
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            abs_matrix_set(B, i, j, vals[i * 3 + j]);
    var D3 = abs_matrix_det(B);
    CHECK(!is_err(D3) && close_to(D3->val.f, -306.0));

    var one = abs_matrix_new(1, 1);
    abs_matrix_set(one, 0, 0, 7.0);
    var D1 = abs_matrix_det(one);
    CHECK(!is_err(D1) && close_to(D1->val.f, 7.0));

    var bad = abs_matrix_mul(A, abs_matrix_new(3, 3));
    CHECK(is_err(bad));

    CHECK(strcmp(type(A)->val.s, "<class 'matrix'>") == 0);

    abs_cleanup();
    return 0;
}
