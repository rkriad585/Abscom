# Ultimate Layer

Computational backends (SIMD/GPU dispatch), scalar autograd, PPM image processing, ASCII and SVG plotting, and a mixed-type DataFrame — the "ultimate" feature set.

```c
#include "abscom/abs.h"
```

Most of the runtime is built on GC-tracked `var` objects, but the ultimate layer's autograd nodes, images, and dataframes are **plain heap objects** (like `AbsComplex`, which is a plain value). They are owned by the caller and released with their `*_free` functions — there is no need to call `abs_init()`/`abs_cleanup()` for them, though the demo does for the matrix parts.

## Computational backends

| Function | Description |
| --- | --- |
| `void abs_set_backend(AbsBackend backend)` | Select `ABS_CPU`, `ABS_CPU_AVX`, or `ABS_GPU_CUDA`. |
| `AbsBackend abs_get_backend(void)` | The currently selected backend. |
| `const char *abs_backend_name(AbsBackend backend)` | Human-readable name (`"CPU"`, `"CPU_AVX"`, `"GPU_CUDA"`). |

`abs_matrix_mul` dispatches on the current backend:

- **`ABS_CPU`** — scalar triple loop.
- **`ABS_CPU_AVX`** — an AVX kernel that vectorizes the reduction over the inner dimension four doubles at a time. It is compiled **only when the library is built with `-mavx`**; otherwise it silently falls back to the scalar kernel. The matrix API and results are identical either way.
- **`ABS_GPU_CUDA`** — a simulation stub: it warns once that CUDA is not linked, then falls back to the CPU kernel.

```c
abs_set_backend(ABS_CPU_AVX);
var C = abs_matrix_mul(A, B);   /* SIMD kernel when built with -mavx */
```

Build with AVX support by adding `-mavx` to the compiler flags (e.g. `make CFLAGS="-O2 -mavx -std=gnu11 -Wall -Wextra -Iinclude"`).

## Scalar autograd

A Micrograd-style computational graph of `double` scalars with reverse-mode differentiation (backpropagation). Nodes are created with `abs_scalar_new`, combined with the arithmetic/activation ops, and gradients are computed with `abs_scalar_backward`.

| Function | Description |
| --- | --- |
| `AbsScalar *abs_scalar_new(double val)` | A leaf holding `val`. |
| `AbsScalar *abs_scalar_add(AbsScalar *a, AbsScalar *b)` | `a + b`. |
| `AbsScalar *abs_scalar_mul(AbsScalar *a, AbsScalar *b)` | `a * b`. |
| `AbsScalar *abs_scalar_relu(AbsScalar *a)` | `max(a, 0)`. |
| `AbsScalar *abs_scalar_sigmoid(AbsScalar *a)` | `1 / (1 + exp(-a))`. |
| `void abs_scalar_backward(AbsScalar *root)` | Reverse pass: fills `grad` on every reachable node, seeding the root at `1.0` and accumulating on top of existing gradients. |
| `void abs_scalar_zero_grad(AbsScalar *root)` | Reset every reachable gradient to zero. |
| `void abs_scalar_free(AbsScalar *root)` | Free the root and its whole subtree (safe on shared subgraphs). |
| `double abs_scalar_val(AbsScalar *v)` / `double abs_scalar_grad(AbsScalar *v)` | Read back a node's value and gradient. |

```c
AbsScalar *a = abs_scalar_new(2.0);
AbsScalar *b = abs_scalar_new(3.0);
AbsScalar *c = abs_scalar_new(-5.0);
AbsScalar *t = abs_scalar_mul(a, b);
AbsScalar *r = abs_scalar_relu(c);
AbsScalar *f = abs_scalar_add(t, r);   /* f = (a * b) + relu(c) = 6 */

abs_scalar_backward(f);
printf("f=%.1f da=%.1f db=%.1f dc=%.1f\n",
       abs_scalar_val(f),
       abs_scalar_grad(a), abs_scalar_grad(b), abs_scalar_grad(c));
/* f=6.0 da=3.0 db=2.0 dc=0.0 */

abs_scalar_free(f);
```

Shared subtrees work: a node used by several consumers accumulates gradients from each path (e.g. `f = (a + b) + (a * b)` gives `df/da = 1 + b`).

## Computer vision: PPM images

`AbsImg` owns a `width * height * 3` byte buffer of row-major RGB samples.

| Function | Description |
| --- | --- |
| `AbsImg *abs_img_load_ppm(const char *filename)` | Load a PPM file (`P3` text or `P6` binary, 8- or 16-bit); `NULL` on error. |
| `void abs_img_save_ppm(const AbsImg *img, const char *filename)` | Write as `P3` text. |
| `AbsImg *abs_img_conv2d(const AbsImg *img, int kernel_size, const double *kernel)` | Apply an odd `kernel_size x kernel_size` filter to each channel, zero-padded, output clamped to 0–255. |
| `void abs_img_free(AbsImg *img)` | Free the image. |

```c
AbsImg *img = abs_img_load_ppm("test.ppm");
double box[9];
for (int i = 0; i < 9; i++) box[i] = 1.0 / 9.0;
AbsImg *blurred = abs_img_conv2d(img, 3, box);
abs_img_save_ppm(blurred, "test_blur.ppm");
abs_img_free(blurred);
abs_img_free(img);
```

The kernel is applied independently to the red, green, and blue channels; pixels outside the image are treated as zero (zero padding).

## Plotting

| Function | Description |
| --- | --- |
| `void abs_plot_ascii(const double *y, int n, int height)` | Print a `height`-row ASCII line chart of `y[0..n)`. |
| `void abs_plot_svg(const double *x, const double *y, int n, const char *filename)` | Export an SVG line chart; pass `x == NULL` to use sample indices. |

Both handle flat series and single-point inputs without dividing by zero.

```c
double ys[30];
for (int i = 0; i < 30; i++) ys[i] = sin(i * 0.1);
abs_plot_ascii(ys, 30, 8);                /* prints a terminal chart */
abs_plot_svg(NULL, ys, 30, "plot.svg");   /* write plot.svg */
```

## DataFrame

`AbsDF` holds named columns of `double` or string values over a shared row count.

| Function | Description |
| --- | --- |
| `AbsDF *abs_df_create(int rows)` | An empty frame with `rows` rows. |
| `void abs_df_add_col_double(AbsDF *df, const char *name, const double *values)` | Append a numeric column (copied in). |
| `void abs_df_add_col_string(AbsDF *df, const char *name, const char *const *values)` | Append a string column (each string is copied). |
| `void abs_df_print(const AbsDF *df)` | Print an aligned table. |
| `void abs_df_free(AbsDF *df)` | Free the frame and all column data. |

```c
AbsDF *df = abs_df_create(3);
double ages[3]   = {25.0, 30.0, 22.0};
const char *names[3]  = {"Alice", "Bob", "Carol"};
double scores[3] = {88.5, 91.0, 79.25};

abs_df_add_col_double(df, "Age", ages);
abs_df_add_col_string(df, "Name", names);
abs_df_add_col_double(df, "Score", scores);
abs_df_print(df);
abs_df_free(df);
```

The column structs (`AbsCol`, `AbsColType`, `ABS_COL_DOUBLE`, `ABS_COL_STRING`) are public, so column data can also be read directly (`df->cols[i]->doubles[j]`, `df->cols[i]->strings[j]`).

## Example

See `examples/ultra_demo.c` (`build/examples/ultra_demo`), which switches backends, trains a tiny autograd expression, creates and blurs a PPM image, plots `sin(x)` to the terminal and to SVG, and prints a DataFrame.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).

For the `var`-based matrix operations the backend dispatches over, see [Matrices, Statistics, and More](scientific.md) and [AI/ML Layer](ml.md).

For modern type aliases, 2D/3D/4D vectors, matrices, and quaternions, see [Spatial Math](spatial-math.md).
