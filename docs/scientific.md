# Scientific Layer

Matrices, statistics, advanced math, combinatorics, CSV parsing, path handling, and basic multithreading — all built on the `var` object model.

```c
#include "abscom/abs.h"
```

Two new object types are added: `ABS_MATRIX` and `ABS_THREAD`. The object pool allocator is guarded by a lock so `abs_new_*` constructors are safe to call from worker threads.

## Matrices

A `var` matrix stores its values in a single flat `double` array, laid out row-major.

| Function | Description |
| --- | --- |
| `var abs_matrix_new(int rows, int cols)` | Zero-initialized `rows x cols` matrix. |
| `var abs_matrix_eye(int n)` | `n x n` identity matrix. |
| `int abs_matrix_rows(var m)` | Row count (0 for non-matrices). |
| `int abs_matrix_cols(var m)` | Column count (0 for non-matrices). |
| `void abs_matrix_set(var m, int r, int c, double val)` | Set element `(r, c)` (bounds-checked). |
| `double abs_matrix_get(var m, int r, int c)` | Get element `(r, c)` (0.0 on error). |
| `var abs_matrix_mul(var A, var B)` | Matrix product; `ABS_ERROR` on dimension mismatch. |
| `var abs_matrix_transpose(var m)` | Transposed copy. |
| `var abs_matrix_det(var m)` | Determinant of a square matrix (Laplace expansion, any size). |
| `void abs_matrix_print(var m)` | Pretty-print a matrix. |

```c
var A = abs_matrix_new(2, 2);
abs_matrix_set(A, 0, 0, 1.0); abs_matrix_set(A, 0, 1, 2.0);
abs_matrix_set(A, 1, 0, 3.0); abs_matrix_set(A, 1, 1, 4.0);

print(v("A:"), A);                    /* Matrix(2x2): [[1.00, 2.00], [3.00, 4.00]] */
print(v("A * I:"), abs_matrix_mul(A, abs_matrix_eye(2)));
print(v("det(A):"), abs_matrix_det(A));   /* -2.00 */
```

Matrices print with `print(...)` in a compact single-line form and stringify with `to_str(...)`.

## Statistics

Statistics functions take a `var` list of numbers.

| Function | Description |
| --- | --- |
| `var abs_stats_mean(var list)` | Arithmetic mean as a float. |
| `var abs_stats_median(var list)` | Middle value (averages the two middle values for even counts). |
| `var abs_stats_mode(var list)` | Most frequent item. |
| `var abs_stats_variance(var list)` | Population variance (requires at least 2 values). |
| `var abs_stats_stdev(var list)` | Population standard deviation. |

```c
var data = List();
append(data, v(10)); append(data, v(20));
append(data, v(20)); append(data, v(40));

print(abs_stats_mean(data));     /* 22.50  */
print(abs_stats_median(data));   /* 20.00  */
print(abs_stats_mode(data));     /* 20     */
print(abs_stats_variance(data)); /* 118.75 */
print(abs_stats_stdev(data));    /* 10.90  */
```

## Advanced math

| Function | Description |
| --- | --- |
| `var sin_val(var x)` | Sine (radians). |
| `var cos_val(var x)` | Cosine (radians). |
| `var tan_val(var x)` | Tangent (radians). |
| `var log_val(var x)` | Natural logarithm. |
| `var log10_val(var x)` | Base-10 logarithm. |
| `var sqrt_val(var x)` | Square root. |
| `var deg2rad(var x)` | Degrees to radians. |

All accept an `ABS_INT` or `ABS_FLOAT` and return a float; a non-number returns `ABS_ERROR`.

```c
print(sin_val(deg2rad(v(45)))); /* 0.71  */
print(log10_val(v(100)));       /* 2.00  */
print(sqrt_val(v(9)));          /* 3.00  */
```

## Combinatorics

| Function | Description |
| --- | --- |
| `var factorial(var n)` | `n!` for `n >= 0`. |
| `var nCr(var n, var r)` | Combinations (`n` choose `r`). |
| `var nPr(var n, var r)` | Permutations. |

Invalid inputs (`n < 0`, `r > n`) return `ABS_ERROR`.

```c
print(factorial(v(5)));   /* 120 */
print(nCr(v(5), v(2)));   /* 10  */
print(nPr(v(5), v(2)));   /* 20  */
```

## Paths and the working directory

Uses `\` on Windows and `/` elsewhere.

| Function | Description |
| --- | --- |
| `var path_join(var p1, var p2)` | Join two path strings with the platform separator. |
| `var path_exists(var path)` | `True`/`False` for an existing file or directory. |
| `var getcwd_val(void)` | Current working directory as a string. |

```c
var cwd = getcwd_val();
var p = path_join(cwd, v("out.csv"));
print(v("Saving to:"), p);
print(path_exists(p));   /* False (before the file is written) */
```

## CSV parsing

A simple CSV reader/writer. Numeric fields are parsed into ints or floats; everything else stays a string. Quoted fields (commas inside quotes) are not supported.

| Function | Description |
| --- | --- |
| `var csv_read(const char *filename)` | List of lists; `ABS_ERROR` if the file can't be opened. |
| `void csv_write(const char *filename, var list_of_lists)` | Write a list of rows. |

```c
var rows = List();
var row = List();
append(row, v(1)); append(row, v(2.5)); append(row, v("hello"));
append(rows, row);

csv_write("out.csv", rows);
var back = csv_read("out.csv");
print(back);   /* [[1, 2.5, hello]] */
```

## Threading

`thread_start` runs `AbsThreadFunc func(var arg)` on a new OS thread; `thread_join` blocks until it finishes and returns the `var` the function produced.

| Function | Description |
| --- | --- |
| `var thread_start(AbsThreadFunc func, var arg)` | Start a thread; returns an `ABS_THREAD` object. |
| `var thread_join(var thread_obj)` | Wait for the thread; returns its result. |

On POSIX, link `-lpthread` (the build systems do this automatically). On Windows the native `CreateThread`/`WaitForSingleObject` APIs are used — no extra flags.

```c
static var heavy_calculation(var input) {
    return factorial(input);
}

var t = thread_start(heavy_calculation, v(5));
var result = thread_join(t);
print(v("Result:"), result);   /* 120 */
```

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
