# AI/ML Layer

Activations, losses, and numerical gradients on top of the scientific layer's matrices — everything a small neural network needs for a forward pass.

```c
#include "abscom/abs.h"
```

No new object types are needed: the layer works on the existing `ABS_MATRIX` values from [scientific.md](scientific.md).

## Matrix arithmetic

The scientific layer provides element-wise matrix operations, a random constructor for weight initialization, and broadcasting:

| Function | Description |
| --- | --- |
| `var abs_matrix_random(int rows, int cols)` | Uniformly random entries in `[-1, 1]` (weight init). |
| `var abs_matrix_eye(int n)` | `n x n` identity matrix. |
| `var abs_matrix_copy(var m)` | Deep copy. |
| `var abs_matrix_add(var A, var B)` | Element-wise sum; `ABS_ERROR` on dimension mismatch. |
| `var abs_matrix_sub(var A, var B)` | Element-wise difference; `ABS_ERROR` on dimension mismatch. |
| `var abs_matrix_mul_element(var A, var B)` | Hadamard (element-wise) product; needed for backpropagation. |
| `var abs_matrix_scale(var m, double s)` | Multiply every element by `s` (returns a new matrix). |
| `var abs_matrix_add_scalar(var m, double s)` | Add `s` to every element (returns a new matrix). |
| `void abs_matrix_apply(var m, double (*func)(double))` | Apply `func` to every element in place. |
| `void abs_matrix_add_row_vector(var m, var v)` | Broadcasting: add the `1 x cols` row vector `v` to every row of `m` in place (bias terms). |

```c
var A = abs_matrix_new(2, 2);
abs_matrix_set(A, 0, 0, 1.0); abs_matrix_set(A, 0, 1, 2.0);
abs_matrix_set(A, 1, 0, 3.0); abs_matrix_set(A, 1, 1, 4.0);

print(abs_matrix_add(A, A));          /* [[2.00, 4.00], [6.00, 8.00]] */
print(abs_matrix_mul_element(A, A));  /* [[1.00, 4.00], [9.00, 16.00]] */
print(abs_matrix_scale(A, 0.5));      /* [[0.50, 1.00], [1.50, 2.00]] */

var W = abs_matrix_random(2, 4);      /* weights in [-1, 1] */
```

Matrix multiplication is `abs_matrix_mul(A, B)` and transpose is `abs_matrix_transpose(m)` (both from the scientific layer).

## Reductions and argmax

| Function | Description |
| --- | --- |
| `var abs_matrix_sum(var m)` | Sum of all elements (float). |
| `var abs_matrix_mean(var m)` | Arithmetic mean (float). |
| `var abs_matrix_min(var m)` | Smallest element (float). |
| `var abs_matrix_max(var m)` | Largest element (float). |
| `long abs_matrix_argmax(var m)` | Flat index of the largest element; `-1` for non-matrices. |

```c
print(abs_matrix_sum(Rm));      /* 10.00 */
print(abs_matrix_mean(Rm));     /* 2.50  */
print(abs_matrix_argmax(Rm));   /* 3     */
```

`abs_matrix_argmax` is the building block for classification: pick the predicted class per row, then compare with `abs_accuracy`.

## Activations

The activation functions take and return plain `double`s, so they plug straight into `abs_matrix_apply`:

| Function | Description |
| --- | --- |
| `double abs_act_sigmoid(double x)` | Logistic sigmoid `1 / (1 + e^-x)`. |
| `double abs_act_relu(double x)` | Rectified linear unit `max(0, x)`. |
| `double abs_act_tanh(double x)` | Hyperbolic tangent. |
| `void abs_matrix_softmax(var m)` | Row-wise softmax, applied in place. |

```c
print(v(ABS_PI));                     /* 3.14 */

var Z = abs_matrix_mul(X, W1);
abs_matrix_apply(Z, abs_act_sigmoid); /* hidden layer activation */
```

`abs_matrix_softmax` subtracts each row's max before exponentiating, so it stays numerically stable on large logits. Every row sums to `1.0` afterwards:

```c
var logits = abs_matrix_new(2, 2);
abs_matrix_set(logits, 0, 0, 1.0); abs_matrix_set(logits, 0, 1, 2.0);
abs_matrix_set(logits, 1, 0, 3.0); abs_matrix_set(logits, 1, 1, 4.0);

abs_matrix_softmax(logits);
print(logits);                        /* [[0.27, 0.73], [0.27, 0.73]] */
```

## Derivatives (backpropagation)

Each activation has a derivative that takes the *activated* output `y` (not the pre-activation `x`), which is what backpropagation has on hand:

| Function | Description |
| --- | --- |
| `double abs_diff_sigmoid(double y)` | `y * (1 - y)`. |
| `double abs_diff_relu(double y)` | `1` if `y > 0` else `0`. |
| `double abs_diff_tanh(double y)` | `1 - y^2`. |
| `var abs_matrix_apply_deriv(var m, double (*func)(double))` | Copy `m`, then apply `func` to every element of the copy. |

```c
/* delta_output = (y_pred - y_true) * sigmoid'(y_pred) */
var Error = abs_matrix_sub(Y_pred, Y_true);
var delta = abs_matrix_mul_element(Error,
             abs_matrix_apply_deriv(Y_pred, abs_diff_sigmoid));
```

`abs_matrix_apply_deriv` never touches the original matrix, so the activation outputs stay available for the next backward pass.

## Loss

| Function | Description |
| --- | --- |
| `var abs_loss_mse(var y_true, var y_pred)` | Mean squared error; returns a float. `ABS_ERROR` on dimension mismatch. |

```c
var loss = abs_loss_mse(y_true, y_hat);
print(loss);
```

## Evaluation metrics

| Function | Description |
| --- | --- |
| `var abs_accuracy(var y_true, var y_pred)` | Classification accuracy: the fraction of rows where `argmax(y_true)` matches `argmax(y_pred)`; returns a float. |

```c
/* y_true is one-hot; y_pred is a softmax probability matrix */
print(abs_accuracy(y_true, y_pred));   /* 0.50 */
```

## Numerical gradient

| Function | Description |
| --- | --- |
| `var abs_grad(AbsFunc f, var x)` | Central-difference gradient `(f(x+h) - f(x-h)) / 2h` with `h = 1e-5`. |

`f` is any `var (*)(var)` callback (an `AbsFunc`). `x` may be an `ABS_INT` or `ABS_FLOAT`; anything else returns `ABS_ERROR`.

```c
static var square(var x) {
    return abs_new_float(abs_num_val(x) * abs_num_val(x));
}

print(abs_grad(square, abs_new_float(3.0)));   /* 6.00  */
print(abs_grad(square, abs_new_int(2)));       /* 4.00  */
```

## Constants

- `ABS_PI` — `3.14159265358979323846`
- `ABS_E` — `2.71828182845904523536`

## Neural network forward pass

`examples/ml_demo.c` builds a two-layer network: a batch of three samples (`X`, 3×2) is multiplied by random weights `W1` (2×4), passed through `sigmoid`, multiplied by `W2` (4×1), passed through `relu`, and scored with `abs_loss_mse`:

```c
var Z1 = abs_matrix_mul(X, W1);
abs_matrix_apply(Z1, abs_act_sigmoid);

var Z2 = abs_matrix_mul(Z1, W2);
abs_matrix_apply(Z2, abs_act_relu);

print(abs_loss_mse(Y_true, Z2));
```

Run it with:

```sh
./build/examples/ml_demo
```

## One training step (backpropagation)

`examples/ml_train_demo.c` walks through forward pass, loss, backpropagation, and a single SGD weight update — weights, biases with broadcasting, the Hadamard product, and activation derivatives:

```c
var Z1 = abs_matrix_mul(X, W1);
abs_matrix_add_row_vector(Z1, b1);          /* fold in the bias */
var A1 = abs_matrix_copy(Z1);
abs_matrix_apply(A1, abs_act_sigmoid);

var Z2 = abs_matrix_mul(A1, W2);
abs_matrix_add_row_vector(Z2, b2);
var Y_pred = abs_matrix_copy(Z2);
abs_matrix_apply(Y_pred, abs_act_sigmoid);

var Error = abs_matrix_sub(Y_pred, Y_target);
var delta2 = abs_matrix_mul_element(Error,
              abs_matrix_apply_deriv(Y_pred, abs_diff_sigmoid));

var dW2 = abs_matrix_mul(abs_matrix_transpose(A1), delta2);
var W2_new = abs_matrix_sub(W2, abs_matrix_scale(dW2, 0.1));   /* SGD */
```

Run it with:

```sh
./build/examples/ml_train_demo
```

## Preprocessing helpers

For datasets, the [Data Science Layer](data-science.md) provides `abs_matrix_one_hot_encode` (labels to one-hot rows) and `abs_matrix_train_test_split` (returns a var list `[X_train, X_test, Y_train, Y_test]`), plus generators like `abs_matrix_arange`/`abs_matrix_linspace` and Pandas-style `abs_matrix_read_csv`/`abs_matrix_write_csv`.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
