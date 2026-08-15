#include "abscom/abs.h"

#include <math.h>
#include <stdio.h>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static int close_to(double a, double b) {
    return fabs(a - b) < 1e-6;
}

static var square(var x) {
    double v = abs_num_val(x);
    return abs_new_float(v * v);
}

int main(void) {
    abs_init();

    /* --- Activations --- */
    CHECK(close_to(abs_act_sigmoid(0.0), 0.5));
    CHECK(close_to(abs_act_sigmoid(10.0), 0.9999546021312976));
    CHECK(close_to(abs_act_relu(-3.0), 0.0));
    CHECK(close_to(abs_act_relu(0.0), 0.0));
    CHECK(close_to(abs_act_relu(2.5), 2.5));
    CHECK(close_to(abs_act_tanh(0.0), 0.0));
    CHECK(close_to(abs_act_tanh(1.0), 0.7615941559557649));

    /* --- Matrix add / sub / scale --- */
    var A = abs_matrix_new(2, 2);
    abs_matrix_set(A, 0, 0, 1.0);
    abs_matrix_set(A, 0, 1, 2.0);
    abs_matrix_set(A, 1, 0, 3.0);
    abs_matrix_set(A, 1, 1, 4.0);

    var B = abs_matrix_new(2, 2);
    abs_matrix_set(B, 0, 0, 5.0);
    abs_matrix_set(B, 0, 1, 6.0);
    abs_matrix_set(B, 1, 0, 7.0);
    abs_matrix_set(B, 1, 1, 8.0);

    var S = abs_matrix_add(A, B);
    CHECK(!is_err(S));
    CHECK(close_to(abs_matrix_get(S, 0, 0), 6.0));
    CHECK(close_to(abs_matrix_get(S, 1, 1), 12.0));

    var D = abs_matrix_sub(B, A);
    CHECK(!is_err(D));
    CHECK(close_to(abs_matrix_get(D, 0, 1), 4.0));
    CHECK(close_to(abs_matrix_get(D, 1, 0), 4.0));

    var Sc = abs_matrix_scale(A, 2.0);
    CHECK(!is_err(Sc));
    CHECK(close_to(abs_matrix_get(Sc, 0, 0), 2.0));
    CHECK(close_to(abs_matrix_get(Sc, 1, 1), 8.0));

    CHECK(is_err(abs_matrix_add(A, abs_matrix_new(3, 3))));
    CHECK(is_err(abs_matrix_sub(A, abs_matrix_new(3, 3))));
    CHECK(is_err(abs_matrix_scale(A, 2.0)) == false);

    /* --- abs_matrix_apply with relu --- */
    var M = abs_matrix_new(1, 3);
    abs_matrix_set(M, 0, 0, -1.0);
    abs_matrix_set(M, 0, 1, 0.0);
    abs_matrix_set(M, 0, 2, 4.0);
    abs_matrix_apply(M, abs_act_relu);
    CHECK(close_to(abs_matrix_get(M, 0, 0), 0.0));
    CHECK(close_to(abs_matrix_get(M, 0, 1), 0.0));
    CHECK(close_to(abs_matrix_get(M, 0, 2), 4.0));

    /* --- Random matrix --- */
    var R = abs_matrix_random(3, 4);
    CHECK(R != NULL && R->type == ABS_MATRIX);
    CHECK(abs_matrix_rows(R) == 3 && abs_matrix_cols(R) == 4);
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++) {
            double v = abs_matrix_get(R, i, j);
            CHECK(v >= -1.0 && v <= 1.0);
        }

    /* --- Softmax --- */
    var SM = abs_matrix_new(2, 2);
    abs_matrix_set(SM, 0, 0, 1.0);
    abs_matrix_set(SM, 0, 1, 2.0);
    abs_matrix_set(SM, 1, 0, 3.0);
    abs_matrix_set(SM, 1, 1, 4.0);
    abs_matrix_softmax(SM);
    double row0 = abs_matrix_get(SM, 0, 0) + abs_matrix_get(SM, 0, 1);
    double row1 = abs_matrix_get(SM, 1, 0) + abs_matrix_get(SM, 1, 1);
    CHECK(close_to(row0, 1.0));
    CHECK(close_to(row1, 1.0));
    CHECK(abs_matrix_get(SM, 0, 0) > 0.0 && abs_matrix_get(SM, 1, 1) > 0.0);

    /* --- MSE loss --- */
    var Y = abs_matrix_new(2, 2);
    var P = abs_matrix_new(2, 2);
    for (int i = 0; i < 4; i++) P->val.matrix.data[i] = 1.0;
    var loss = abs_loss_mse(Y, P);
    CHECK(!is_err(loss) && close_to(loss->val.f, 1.0));
    CHECK(is_err(abs_loss_mse(Y, abs_matrix_new(3, 3))));

    /* --- Numerical gradient --- */
    var g = abs_grad(square, abs_new_float(3.0));
    CHECK(!is_err(g) && close_to(g->val.f, 6.0));
    var g2 = abs_grad(square, abs_new_int(2));
    CHECK(!is_err(g2) && close_to(g2->val.f, 4.0));
    CHECK(is_err(abs_grad(square, abs_new_str("x"))));

    /* --- Hadamard product, scalar add, deep copy --- */
    var H = abs_matrix_mul_element(A, B);
    CHECK(!is_err(H));
    CHECK(close_to(abs_matrix_get(H, 0, 0), 5.0));
    CHECK(close_to(abs_matrix_get(H, 0, 1), 12.0));
    CHECK(close_to(abs_matrix_get(H, 1, 0), 21.0));
    CHECK(close_to(abs_matrix_get(H, 1, 1), 32.0));
    CHECK(is_err(abs_matrix_mul_element(A, abs_matrix_new(3, 3))));

    var AS = abs_matrix_add_scalar(A, 1.0);
    CHECK(!is_err(AS));
    CHECK(close_to(abs_matrix_get(AS, 0, 0), 2.0));
    CHECK(close_to(abs_matrix_get(A, 0, 0), 1.0));

    var Cp = abs_matrix_copy(A);
    CHECK(!is_err(Cp));
    abs_matrix_set(Cp, 0, 0, 99.0);
    CHECK(close_to(abs_matrix_get(A, 0, 0), 1.0));
    CHECK(close_to(abs_matrix_get(Cp, 0, 0), 99.0));

    /* --- Broadcasting: add a row vector to every row --- */
    var V = abs_matrix_new(1, 3);
    abs_matrix_set(V, 0, 0, 10.0);
    abs_matrix_set(V, 0, 1, 20.0);
    abs_matrix_set(V, 0, 2, 30.0);
    var M3 = abs_matrix_new(2, 3);
    abs_matrix_set(M3, 0, 1, 1.0);
    abs_matrix_set(M3, 1, 2, 2.0);
    abs_matrix_add_row_vector(M3, V);
    CHECK(close_to(abs_matrix_get(M3, 0, 0), 10.0));
    CHECK(close_to(abs_matrix_get(M3, 0, 1), 21.0));
    CHECK(close_to(abs_matrix_get(M3, 0, 2), 30.0));
    CHECK(close_to(abs_matrix_get(M3, 1, 0), 10.0));
    CHECK(close_to(abs_matrix_get(M3, 1, 1), 20.0));
    CHECK(close_to(abs_matrix_get(M3, 1, 2), 32.0));
    var BadV = abs_matrix_new(1, 2);
    abs_matrix_add_row_vector(M3, BadV);
    CHECK(close_to(abs_matrix_get(M3, 0, 0), 10.0));

    /* --- Reductions --- */
    var Rm = abs_matrix_new(2, 2);
    abs_matrix_set(Rm, 0, 0, 1.0);
    abs_matrix_set(Rm, 0, 1, 2.0);
    abs_matrix_set(Rm, 1, 0, 3.0);
    abs_matrix_set(Rm, 1, 1, 4.0);
    var Ssum = abs_matrix_sum(Rm);
    CHECK(!is_err(Ssum) && close_to(Ssum->val.f, 10.0));
    var Smean = abs_matrix_mean(Rm);
    CHECK(!is_err(Smean) && close_to(Smean->val.f, 2.5));
    var Smin = abs_matrix_min(Rm);
    CHECK(!is_err(Smin) && close_to(Smin->val.f, 1.0));
    var Smax = abs_matrix_max(Rm);
    CHECK(!is_err(Smax) && close_to(Smax->val.f, 4.0));
    CHECK(abs_matrix_argmax(Rm) == 3);
    CHECK(abs_matrix_argmax(abs_new_str("x")) == -1);
    CHECK(is_err(abs_matrix_sum(abs_new_str("x"))));

    /* --- Activation derivatives (given the activated output y) --- */
    CHECK(close_to(abs_diff_sigmoid(0.5), 0.25));
    CHECK(close_to(abs_diff_relu(3.0), 1.0));
    CHECK(close_to(abs_diff_relu(0.0), 0.0));
    CHECK(close_to(abs_diff_relu(-1.0), 0.0));
    CHECK(close_to(abs_diff_tanh(0.5), 0.75));

    var DV = abs_matrix_new(1, 3);
    abs_matrix_set(DV, 0, 0, -1.0);
    abs_matrix_set(DV, 0, 1, 2.0);
    abs_matrix_set(DV, 0, 2, 3.0);
    var DVr = abs_matrix_apply_deriv(DV, abs_diff_relu);
    CHECK(!is_err(DVr));
    CHECK(close_to(abs_matrix_get(DVr, 0, 0), 0.0));
    CHECK(close_to(abs_matrix_get(DVr, 0, 1), 1.0));
    CHECK(close_to(abs_matrix_get(DVr, 0, 2), 1.0));
    CHECK(close_to(abs_matrix_get(DV, 0, 0), -1.0));

    /* --- Classification accuracy (row argmax match) --- */
    var AT = abs_matrix_new(2, 2);
    abs_matrix_set(AT, 0, 0, 1.0);
    abs_matrix_set(AT, 1, 1, 1.0);
    var AP = abs_matrix_new(2, 2);
    abs_matrix_set(AP, 0, 0, 0.8);
    abs_matrix_set(AP, 0, 1, 0.2);
    abs_matrix_set(AP, 1, 0, 0.3);
    abs_matrix_set(AP, 1, 1, 0.7);
    var acc = abs_accuracy(AT, AP);
    CHECK(!is_err(acc) && close_to(acc->val.f, 1.0));
    abs_matrix_set(AP, 1, 0, 0.9);
    abs_matrix_set(AP, 1, 1, 0.1);
    var acc2 = abs_accuracy(AT, AP);
    CHECK(!is_err(acc2) && close_to(acc2->val.f, 0.5));
    CHECK(is_err(abs_accuracy(AT, abs_matrix_new(3, 3))));

    /* --- Constants --- */
    CHECK(close_to(ABS_PI, 3.14159265358979323846));
    CHECK(close_to(ABS_E, 2.71828182845904523536));

    /* --- One-hot encoding --- */
    var L = abs_matrix_new(4, 1);
    abs_matrix_set(L, 0, 0, 0.0);
    abs_matrix_set(L, 1, 0, 1.0);
    abs_matrix_set(L, 2, 0, 2.0);
    abs_matrix_set(L, 3, 0, 0.0);
    var OH = abs_matrix_one_hot_encode(L, 3);
    CHECK(!is_err(OH) && abs_matrix_rows(OH) == 4 && abs_matrix_cols(OH) == 3);
    CHECK(close_to(abs_matrix_get(OH, 0, 0), 1.0));
    CHECK(close_to(abs_matrix_get(OH, 1, 1), 1.0));
    CHECK(close_to(abs_matrix_get(OH, 2, 2), 1.0));
    CHECK(close_to(abs_matrix_get(OH, 3, 0), 1.0));
    CHECK(close_to(abs_matrix_get(OH, 2, 0), 0.0));
    CHECK(is_err(abs_matrix_one_hot_encode(abs_matrix_new(2, 2), 3)));
    CHECK(is_err(abs_matrix_one_hot_encode(L, 0)));

    /* --- Train/test split --- */
    var X4 = abs_matrix_arange(0.0, 8.0, 1.0);
    var Xm = abs_matrix_reshape(X4, 4, 2);
    var sp = abs_matrix_train_test_split(Xm, L, 0.25);
    CHECK(!is_err(sp) && sp->type == ABS_LIST);
    var xtr = get(sp, 0);
    var xte = get(sp, 1);
    var ytr = get(sp, 2);
    var yte = get(sp, 3);
    CHECK(abs_matrix_rows(xtr) == 3 && abs_matrix_cols(xtr) == 2);
    CHECK(abs_matrix_rows(xte) == 1 && abs_matrix_cols(xte) == 2);
    CHECK(abs_matrix_rows(ytr) == 3 && abs_matrix_cols(ytr) == 1);
    CHECK(abs_matrix_rows(yte) == 1 && abs_matrix_cols(yte) == 1);
    CHECK(close_to(abs_matrix_get(xte, 0, 0), 6.0));
    CHECK(close_to(abs_matrix_get(yte, 0, 0), 0.0));
    CHECK(close_to(abs_matrix_get(xtr, 2, 0), 4.0));
    CHECK(is_err(abs_matrix_train_test_split(Xm, abs_matrix_new(3, 1), 0.5)));
    CHECK(is_err(abs_matrix_train_test_split(Xm, L, 0.0)));
    CHECK(is_err(abs_matrix_train_test_split(Xm, L, 1.0)));
    CHECK(is_err(abs_matrix_train_test_split(abs_matrix_new(1, 1), abs_matrix_new(1, 1), 0.5)));

    abs_cleanup();
    return 0;
}
