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

    /* --- Shape manipulation --- */
    var arr = abs_matrix_arange(0.0, 12.0, 1.0);
    CHECK(!is_err(arr) && abs_matrix_rows(arr) == 1 && abs_matrix_cols(arr) == 12);
    var resh = abs_matrix_reshape(arr, 4, 3);
    CHECK(!is_err(resh) && abs_matrix_rows(resh) == 4 && abs_matrix_cols(resh) == 3);
    CHECK(close_to(abs_matrix_get(resh, 0, 0), 0.0));
    CHECK(close_to(abs_matrix_get(resh, 3, 2), 11.0));
    CHECK(is_err(abs_matrix_reshape(arr, 3, 5)));

    var flat = abs_matrix_flatten(A);
    CHECK(!is_err(flat) && abs_matrix_rows(flat) == 1 && abs_matrix_cols(flat) == 4);
    CHECK(close_to(abs_matrix_get(flat, 0, 3), 4.0));

    var sl = abs_matrix_slice(resh, 1, 3, 0, 2);
    CHECK(!is_err(sl) && abs_matrix_rows(sl) == 2 && abs_matrix_cols(sl) == 2);
    CHECK(close_to(abs_matrix_get(sl, 0, 0), 3.0));
    CHECK(close_to(abs_matrix_get(sl, 1, 1), 7.0));
    CHECK(is_err(abs_matrix_slice(resh, 0, 5, 0, 1)));

    var S = abs_matrix_new(1, 2);
    abs_matrix_set(S, 0, 0, 5.0);
    abs_matrix_set(S, 0, 1, 6.0);
    var vs = abs_matrix_vstack(A, S);
    CHECK(!is_err(vs) && abs_matrix_rows(vs) == 3 && abs_matrix_cols(vs) == 2);
    CHECK(close_to(abs_matrix_get(vs, 2, 0), 5.0));
    CHECK(close_to(abs_matrix_get(vs, 2, 1), 6.0));
    var hs = abs_matrix_hstack(A, A);
    CHECK(!is_err(hs) && abs_matrix_rows(hs) == 2 && abs_matrix_cols(hs) == 4);
    CHECK(close_to(abs_matrix_get(hs, 0, 3), 2.0));
    CHECK(is_err(abs_matrix_vstack(A, abs_matrix_new(2, 3))));
    CHECK(is_err(abs_matrix_hstack(A, abs_matrix_new(1, 2))));

    /* --- Data generation --- */
    var Ones = abs_matrix_ones(2, 3);
    CHECK(!is_err(Ones) && abs_matrix_rows(Ones) == 2 && abs_matrix_cols(Ones) == 3);
    CHECK(close_to(abs_matrix_get(Ones, 0, 0), 1.0));
    CHECK(close_to(abs_matrix_get(Ones, 1, 2), 1.0));
    var zerosm = abs_matrix_new(2, 2);
    CHECK(close_to(abs_matrix_get(zerosm, 1, 1), 0.0));

    var rg = abs_matrix_arange(0.0, 1.0, 0.2);
    CHECK(!is_err(rg) && abs_matrix_cols(rg) == 5);
    CHECK(close_to(abs_matrix_get(rg, 0, 0), 0.0));
    CHECK(close_to(abs_matrix_get(rg, 0, 4), 0.8));
    CHECK(is_err(abs_matrix_arange(0.0, 1.0, 0.0)));
    CHECK(is_err(abs_matrix_arange(5.0, 0.0, 1.0)));

    var ls = abs_matrix_linspace(0.0, 1.0, 5);
    CHECK(!is_err(ls) && abs_matrix_cols(ls) == 5);
    CHECK(close_to(abs_matrix_get(ls, 0, 0), 0.0));
    CHECK(close_to(abs_matrix_get(ls, 0, 4), 1.0));
    CHECK(is_err(abs_matrix_linspace(0.0, 1.0, 0)));

    /* --- Functional utils --- */
    var mapped = abs_matrix_map(A, abs_act_relu);
    CHECK(!is_err(mapped) && close_to(abs_matrix_get(mapped, 0, 0), 1.0));
    CHECK(close_to(abs_matrix_get(mapped, 1, 1), 4.0));
    var shifted = abs_matrix_arange(-5.0, 5.0, 1.0);
    var filtered = abs_matrix_filter(shifted, NULL);
    CHECK(!is_err(filtered) && abs_matrix_cols(filtered) == 10);
    CHECK(close_to(abs_matrix_get(filtered, 0, 9), 4.0));

    /* --- foreach_mat / print_mat macros --- */
    double acc = 0.0;
    double vv;
    foreach_mat(vv, A) acc += vv;
    CHECK(close_to(acc, 10.0));

    /* --- CSV round-trip --- */
    abs_matrix_write_csv(A, "test_matrix.csv");
    var back = abs_matrix_read_csv("test_matrix.csv");
    CHECK(!is_err(back) && abs_matrix_rows(back) == 2 && abs_matrix_cols(back) == 2);
    CHECK(close_to(abs_matrix_get(back, 0, 1), 2.0));
    CHECK(close_to(abs_matrix_get(back, 1, 0), 3.0));
    remove("test_matrix.csv");
    CHECK(is_err(abs_matrix_read_csv("no_such_file.csv")));

    abs_cleanup();
    return 0;
}
