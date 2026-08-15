# Data Science Layer

NumPy-style shape manipulation and data generation, Pandas-style numeric CSV, functional utils, and SciKit-Learn-style preprocessing — all on Abscom's `var` matrix type.

```c
#include "abscom/abs.h"
```

Everything here works on `ABS_MATRIX` values. Shape-changing functions return **new** matrices (the source is never mutated), matching the library's pure-function convention.

## Pythonic macros

| Macro | Description |
| --- | --- |
| `print_mat(m)` | Pretty-print a matrix (`abs_matrix_print`). |
| `foreach_mat(item, m)` | Loop over every element in row-major order, assigning each to `double item`. |

```c
double val;
foreach_mat(val, matrix) printf("%.1f ", val);
```

(`len()` and `foreach()` already exist for the runtime's `var` objects, so the matrix variants are suffixed.)

## Shape manipulation (NumPy style)

| Function | Description |
| --- | --- |
| `var abs_matrix_reshape(var m, int rows, int cols)` | Copy with new dimensions (size must match). |
| `var abs_matrix_flatten(var m)` | Copy as a `1 x N` row vector. |
| `var abs_matrix_slice(var m, int r0, int r1, int c0, int c1)` | Copy of rows `[r0, r1)` and cols `[c0, c1)`. |
| `var abs_matrix_vstack(var A, var B)` | Stack `B` below `A` (columns must match). |
| `var abs_matrix_hstack(var A, var B)` | Stack `B` right of `A` (rows must match). |

```c
var m = abs_matrix_reshape(abs_matrix_arange(0.0, 12.0, 1.0), 4, 3);
var sub = abs_matrix_slice(m, 1, 3, 0, 2);   /* m[1:3, 0:2] */
var stacked = abs_matrix_vstack(m, sub);     /* invalid: cols differ -> ABS_ERROR */
```

## Data generation

`abs_matrix_arange` and `abs_matrix_linspace` return `1 x N` row vectors, mirroring `numpy.arange` / `numpy.linspace`.

| Function | Description |
| --- | --- |
| `var abs_matrix_new(int rows, int cols)` | Zero matrix. |
| `var abs_matrix_ones(int rows, int cols)` | Matrix of ones. |
| `var abs_matrix_eye(int n)` | `n x n` identity. |
| `var abs_matrix_arange(double start, double stop, double step)` | Uniform step sequence; values in `[start, stop)`. |
| `var abs_matrix_linspace(double start, double stop, int steps)` | `steps` evenly spaced values from `start` to `stop` (inclusive). |

```c
var grid = abs_matrix_linspace(0.0, 1.0, 5);   /* [0, 0.25, 0.5, 0.75, 1] */
var rng  = abs_matrix_arange(-5.0, 5.0, 1.0);  /* -5, -4, ..., 4 */
```

## Pandas-style CSV I/O

Numeric-only CSV read/write on matrices. `csv_read`/`csv_write` still handle the mixed-type list-of-lists form.

| Function | Description |
| --- | --- |
| `var abs_matrix_read_csv(const char *filename)` | Read a numeric CSV file into a matrix; `ABS_ERROR` if the file can't be opened or is empty. |
| `void abs_matrix_write_csv(var m, const char *filename)` | Write a matrix as comma-separated values (4-decimal formatting). |

```c
abs_matrix_write_csv(matrix, "out.csv");
var back = abs_matrix_read_csv("out.csv");
```

## Functional utils

| Function | Description |
| --- | --- |
| `var abs_matrix_map(var m, double (*func)(double))` | New matrix with `func` applied to every element. |
| `var abs_matrix_filter(var m, int (*predicate)(double))` | New `1 x N` matrix holding the elements for which `predicate` returns nonzero; `ABS_ERROR` if nothing passes. |

```c
var doubled = abs_matrix_map(m, abs_act_relu);           /* map over elements */
var pos = abs_matrix_filter(abs_matrix_arange(-5, 5, 1), is_positive);
```

## SciKit-Learn-style preprocessing

| Function | Description |
| --- | --- |
| `var abs_matrix_one_hot_encode(var labels, int num_classes)` | Label vector (column or row) to `N x num_classes` one-hot matrix. |
| `var abs_matrix_train_test_split(var X, var Y, double test_ratio)` | Returns a var list `[X_train, X_test, Y_train, Y_test]` by sequential slicing; `n_test = round(n_samples * ratio)`. |

```c
var labels = abs_matrix_new(4, 1);
abs_matrix_set(labels, 0, 0, 0.0);
abs_matrix_set(labels, 1, 0, 1.0);
abs_matrix_set(labels, 2, 0, 2.0);
abs_matrix_set(labels, 3, 0, 0.0);

var y_oh = abs_matrix_one_hot_encode(labels, 3);          /* 4 x 3 */
var splits = abs_matrix_train_test_split(matrix, labels, 0.25);
var x_train = get(splits, 0);                             /* 3 x cols */
var x_test  = get(splits, 1);                             /* 1 x cols */
var y_train = get(splits, 2);
var y_test  = get(splits, 3);
```

## Example

See `examples/ds_demo.c` (`build/examples/ds_demo`), which generates `arange(0, 12)`, reshapes it to `4 x 3`, slices it, round-trips it through CSV, one-hot encodes labels, splits the data, and runs the functional utils.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).

For `var`-based statistics, combinatorics, and trigonometry, see [Matrices, Statistics, and More](scientific.md). For activations, loss, and gradients, see [AI/ML Layer](ml.md). For computational backends, scalar autograd, vision, plotting, and dataframes, see [Ultimate Layer](ultimate.md).
