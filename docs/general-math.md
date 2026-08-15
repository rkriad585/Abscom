# General Mathematics

Scalar math helpers, number theory, geometry, root finding, complex numbers, and raw-array statistics — plain C functions that work on `double`s and `long`s rather than `var` objects.

```c
#include "abscom/abs.h"
```

## Scalar utilities

| Function | Description |
| --- | --- |
| `double abs_sq(double x)` | `x * x`. |
| `double abs_cb(double x)` | `x * x * x`. |
| `double abs_clamp(double x, double min, double max)` | Clamp `x` into `[min, max]`. |
| `double abs_lerp(double a, double b, double t)` | Linear interpolation: `a + t * (b - a)`. |
| `int abs_eq(double a, double b)` | Approximate equality within `ABS_EPSILON`. |

```c
printf("%.2f\n", abs_clamp(10.0, 0.0, 5.0));  /* 5.00 */
printf("%.2f\n", abs_lerp(0.0, 10.0, 0.25));  /* 2.50 */
```

## Number theory and discrete math

| Function | Description |
| --- | --- |
| `long abs_gcd(long a, long b)` | Greatest common divisor (Euclid's algorithm, sign-normalized). |
| `long abs_lcm(long a, long b)` | Least common multiple. |
| `long abs_factorial(int n)` | `n!` (0 for `n < 0`). |
| `int abs_is_prime(long n)` | 1 if `n` is prime, else 0. |
| `long abs_fibonacci(int n)` | The `n`-th Fibonacci number (`fib(0) = 0`, `fib(1) = 1`). |
| `long abs_nPr(int n, int r)` | Permutations (plain C ints; the `var` version is `nPr`). |
| `long abs_nCr(int n, int r)` | Combinations (plain C ints; the `var` version is `nCr`). |

```c
printf("%ld\n", abs_gcd(12, 8));   /* 4    */
printf("%ld\n", abs_lcm(4, 6));    /* 12   */
printf("%ld\n", abs_factorial(5)); /* 120  */
printf("%d\n",  abs_is_prime(17)); /* 1    */
printf("%ld\n", abs_fibonacci(10));/* 55   */
```

## Geometry

| Function | Description |
| --- | --- |
| `double abs_rad2deg(double rad)` | Radians to degrees. |
| `double abs_hypot(double a, double b)` | `sqrt(a*a + b*b)` — Euclidean length. |
| `double abs_dist_euclidean(x1, y1, x2, y2)` | Euclidean distance between two 2D points. |
| `double abs_dist_manhattan(x1, y1, x2, y2)` | Manhattan (L1) distance. |

```c
printf("%.1f\n", abs_rad2deg(ABS_PI));               /* 180.0 */
printf("%.1f\n", abs_hypot(3.0, 4.0));               /* 5.0   */
printf("%.1f\n", abs_dist_euclidean(0, 0, 3, 4));    /* 5.0   */
printf("%.1f\n", abs_dist_manhattan(0, 0, 3, 4));    /* 7.0   */
```

## Root finding

| Function | Description |
| --- | --- |
| `double abs_root_find(double (*f)(double), double (*f_prime)(double), double guess)` | Newton-Raphson root of `f(x) = 0`, up to 100 iterations. Pass the derivative for faster convergence, or `NULL` to use a finite-difference approximation. |

```c
static double quad(double x) { return x * x - 4.0; }

double root = abs_root_find(quad, NULL, 3.0);
printf("%.4f\n", root);   /* 2.0000 */
```

## Complex numbers

A plain value type — no heap internals, so there is nothing to clean up:

```c
typedef struct {
    double real;
    double imag;
} AbsComplex;
```

| Function | Description |
| --- | --- |
| `AbsComplex abs_c_add(AbsComplex a, AbsComplex b)` | Complex sum. |
| `AbsComplex abs_c_sub(AbsComplex a, AbsComplex b)` | Complex difference. |
| `AbsComplex abs_c_mul(AbsComplex a, AbsComplex b)` | Complex product. |
| `double abs_c_mag(AbsComplex a)` | Magnitude `sqrt(re*re + im*im)`. |
| `AbsComplex abs_c_conj(AbsComplex a)` | Complex conjugate. |
| `void abs_c_print(AbsComplex a)` | Print as `(a + bi)` / `(a - bi)` with two-decimal formatting. |

```c
AbsComplex a = {3.0, 4.0};
AbsComplex b = {1.0, -2.0};
abs_c_print(abs_c_add(a, b));   /* (4.00 + 2.00i) */
printf("\n");
abs_c_print(abs_c_mul(a, b));   /* (11.00 - 2.00i) */
printf("\n|a| = %.2f\n", abs_c_mag(a));   /* 5.00 */
```

## Raw-array statistics

These work on plain C `double` arrays; the `var`-list equivalents are the `abs_stats_*` functions.

| Function | Description |
| --- | --- |
| `double abs_stat_mean(double *arr, int size)` | Arithmetic mean. |
| `double abs_stat_median(double *arr, int size)` | Middle value (averages the middle two for even sizes). |
| `double abs_stat_variance(double *arr, int size)` | Population variance (needs `size > 1`). |
| `double abs_stat_stddev(double *arr, int size)` | Population standard deviation. |

```c
double data[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
printf("%.2f\n", abs_stat_mean(data, 5));    /* 3.00 */
printf("%.2f\n", abs_stat_median(data, 5));  /* 3.00 */
printf("%.2f\n", abs_stat_variance(data, 5));/* 2.00 */
```

## Constants

| Macro | Value |
| --- | --- |
| `ABS_PI` | 3.14159265358979323846 |
| `ABS_E` | 2.71828182845904523536 |
| `ABS_SQRT2` | 1.41421356237309504880 |
| `ABS_PHI` | 1.61803398874989484820 |
| `ABS_EPSILON` | `1e-9` (equality and convergence tolerance) |

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).

For `var`-based statistics, combinatorics, and trigonometry, see [Matrices, Statistics, and More](scientific.md). For activations, softmax, loss, and gradients, see [AI/ML Layer](ml.md).
