#include "abscom/abs.h"

#include <stdio.h>

int main(void) {
    abs_init();

    print(v("=== Abscom AI/ML Layer: One Training Step ===\n"));

    print(v("--- 1. Setup ---"));
    var X = abs_matrix_new(1, 3);
    abs_matrix_set(X, 0, 0, 1.0);
    abs_matrix_set(X, 0, 1, 0.5);
    abs_matrix_set(X, 0, 2, -1.0);

    var Y_target = abs_matrix_new(1, 1);
    abs_matrix_set(Y_target, 0, 0, 1.0);

    var W1 = abs_matrix_random(3, 4);
    var b1 = abs_matrix_new(1, 4);
    var W2 = abs_matrix_random(4, 1);
    var b2 = abs_matrix_new(1, 1);
    print(v("Initialized network (X 1x3, W1 3x4, W2 4x1)."));

    print(v("--- 2. Forward pass (with bias broadcasting) ---"));
    var Z1 = abs_matrix_mul(X, W1);
    abs_matrix_add_row_vector(Z1, b1);
    var A1 = abs_matrix_copy(Z1);
    abs_matrix_apply(A1, abs_act_sigmoid);

    var Z2 = abs_matrix_mul(A1, W2);
    abs_matrix_add_row_vector(Z2, b2);
    var Y_pred = abs_matrix_copy(Z2);
    abs_matrix_apply(Y_pred, abs_act_sigmoid);
    print(v("Prediction:"), Y_pred);

    print(v("--- 3. Loss (MSE) ---"));
    print(v("MSE loss:"), abs_loss_mse(Y_pred, Y_target));

    print(v("--- 4. Backpropagation ---"));
    double learning_rate = 0.1;
    var Error = abs_matrix_sub(Y_pred, Y_target);

    var dZ2_prime = abs_matrix_apply_deriv(Y_pred, abs_diff_sigmoid);
    var Delta_2 = abs_matrix_mul_element(Error, dZ2_prime);
    var A1_T = abs_matrix_transpose(A1);
    var dW2 = abs_matrix_mul(A1_T, Delta_2);

    var W2_T = abs_matrix_transpose(W2);
    var Hidden_Error = abs_matrix_mul(Delta_2, W2_T);
    var dZ1_prime = abs_matrix_apply_deriv(A1, abs_diff_sigmoid);
    var Delta_1 = abs_matrix_mul_element(Hidden_Error, dZ1_prime);
    var X_T = abs_matrix_transpose(X);
    var dW1 = abs_matrix_mul(X_T, Delta_1);

    print(v("Delta_2 (output error x sigmoid')"), Delta_2);

    print(v("--- 5. SGD weight update ---"));
    var W1_new = abs_matrix_sub(W1, abs_matrix_scale(dW1, learning_rate));
    var W2_new = abs_matrix_sub(W2, abs_matrix_scale(dW2, learning_rate));
    print(v("W1 old:"), W1);
    print(v("W1 new:"), W1_new);
    print(v("W2 old:"), W2);
    print(v("W2 new:"), W2_new);

    abs_cleanup();
    return 0;
}
