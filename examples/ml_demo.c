#include "abscom/abs.h"

#include <stdio.h>

static var square(var x) {
    double v = abs_num_val(x);
    return abs_new_float(v * v);
}

int main(void) {
    abs_init();

    print(v("=== Abscom AI/ML Layer: Neural Network Forward Pass ===\n"));

    print(v("--- 1. Input (3 samples x 2 features) ---"));
    var X = abs_matrix_new(3, 2);
    abs_matrix_set(X, 0, 0, 2.0); abs_matrix_set(X, 0, 1, 9.0);
    abs_matrix_set(X, 1, 0, 1.0); abs_matrix_set(X, 1, 1, 5.0);
    abs_matrix_set(X, 2, 0, 3.0); abs_matrix_set(X, 2, 1, 6.0);
    print(v("X:"), X);

    print(v("--- 2. Random weights (2 -> 4 hidden neurons) ---"));
    var W1 = abs_matrix_random(2, 4);
    print(v("W1:"), W1);
    var W2 = abs_matrix_random(4, 1);

    print(v("--- 3. Forward pass ---"));
    var Z1 = abs_matrix_mul(X, W1);
    abs_matrix_apply(Z1, abs_act_sigmoid);
    print(v("A1 = sigmoid(X . W1):"), Z1);

    var Z2 = abs_matrix_mul(Z1, W2);
    abs_matrix_apply(Z2, abs_act_relu);
    print(v("Y_hat = relu(A1 . W2):"), Z2);

    print(v("--- 4. Mean squared error loss ---"));
    var Y_true = abs_matrix_new(3, 1);
    abs_matrix_set(Y_true, 0, 0, 0.9);
    abs_matrix_set(Y_true, 1, 0, 0.2);
    abs_matrix_set(Y_true, 2, 0, 0.7);
    print(v("Loss:"), abs_loss_mse(Y_true, Z2));

    print(v("--- 5. Numerical gradient ---"));
    print(v("d/dx x^2 at x = 3:"), abs_grad(square, abs_new_float(3.0)));

    abs_cleanup();
    return 0;
}
