#include "abscom/abs.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __AVX__
#include <immintrin.h>
#endif

/* --- Computational backend selection (SIMD / GPU stub) --- */

static AbsBackend _abs_backend = ABS_CPU;

void abs_set_backend(AbsBackend backend) {
    _abs_backend = backend;
    switch (backend) {
        case ABS_CPU:
            printf("[Backend] Set to CPU (scalar kernel).\n");
            break;
        case ABS_CPU_AVX:
#ifdef __AVX__
            printf("[Backend] Set to CPU + AVX (SIMD kernel).\n");
#else
            printf("[Backend] Set to CPU + AVX, but the library was not built "
                   "with -mavx; using the scalar kernel.\n");
#endif
            break;
        case ABS_GPU_CUDA:
            printf("[Backend] GPU_CUDA selected - CUDA not linked; simulation "
                   "mode (CPU fallback).\n");
            break;
    }
}

AbsBackend abs_get_backend(void) {
    return _abs_backend;
}

const char *abs_backend_name(AbsBackend backend) {
    switch (backend) {
        case ABS_CPU:     return "CPU";
        case ABS_CPU_AVX: return "CPU_AVX";
        case ABS_GPU_CUDA:return "GPU_CUDA";
    }
    return "UNKNOWN";
}

/* Row-major GEMM: C[i][j] = sum_k A[i][k] * B[k][j]. */
static void abs_mm_naive(const double *A, const double *B, double *C,
                         int rows, int cols, int common) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double sum = 0.0;
            for (int k = 0; k < common; k++)
                sum += A[i * common + k] * B[k * cols + j];
            C[i * cols + j] = sum;
        }
    }
}

#ifdef __AVX__
/* Same loop, but the reduction over k is vectorized 4 doubles at a time.
 * B is read column-wise, so the four B values for one j are gathered into a
 * stack buffer before loading into a vector. */
static void abs_mm_avx(const double *A, const double *B, double *C,
                       int rows, int cols, int common) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            __m256d sum = _mm256_setzero_pd();
            int k = 0;
            for (; k + 3 < common; k += 4) {
                __m256d a = _mm256_loadu_pd(&A[i * common + k]);
                double bvals[4] = {
                    B[k * cols + j],
                    B[(k + 1) * cols + j],
                    B[(k + 2) * cols + j],
                    B[(k + 3) * cols + j]
                };
                __m256d b = _mm256_loadu_pd(bvals);
                sum = _mm256_add_pd(sum, _mm256_mul_pd(a, b));
            }
            __m128d lo = _mm256_castpd256_pd128(sum);
            __m128d hi = _mm256_extractf128_pd(sum, 1);
            __m128d h = _mm_add_pd(lo, hi);
            double acc = _mm_cvtsd_f64(_mm_add_pd(h, _mm_permute_pd(h, 1)));
            for (; k < common; k++)
                acc += A[i * common + k] * B[k * cols + j];
            C[i * cols + j] = acc;
        }
    }
}
#endif

static void abs_matrix_mul_kernel(const double *A, const double *B, double *C,
                                  int rows, int cols, int common) {
    if (_abs_backend == ABS_GPU_CUDA) {
        static int warned = 0;
        if (!warned) {
            printf("[GPU] abs_matrix_mul: CUDA backend not linked; using CPU "
                   "fallback.\n");
            warned = 1;
        }
    }
#ifdef __AVX__
    if (_abs_backend == ABS_CPU_AVX)
        abs_mm_avx(A, B, C, rows, cols, common);
    else
        abs_mm_naive(A, B, C, rows, cols, common);
#else
    abs_mm_naive(A, B, C, rows, cols, common);
#endif
}

var abs_matrix_new(int rows, int cols) {
    if (rows <= 0 || cols <= 0) return abs_new_error("Invalid matrix dimensions");
    var m = abs_new_obj(ABS_MATRIX);
    if (!m) return abs_new_error("Out of memory");
    m->val.matrix.rows = rows;
    m->val.matrix.cols = cols;
    m->val.matrix.data = (double *)calloc((size_t)rows * (size_t)cols, sizeof(double));
    if (!m->val.matrix.data) return abs_new_error("Out of memory");
    abs_gc_track(m);
    return m;
}

var abs_matrix_eye(int n) {
    if (n <= 0) return abs_new_error("Invalid matrix dimension");
    var m = abs_matrix_new(n, n);
    if (is_err(m)) return m;
    for (int i = 0; i < n; i++)
        m->val.matrix.data[i * n + i] = 1.0;
    return m;
}

var abs_matrix_random(int rows, int cols) {
    var m = abs_matrix_new(rows, cols);
    if (is_err(m)) return m;
    int n = rows * cols;
    for (int i = 0; i < n; i++)
        m->val.matrix.data[i] = ((double)rand() / (double)RAND_MAX) * 2.0 - 1.0;
    return m;
}

int abs_matrix_rows(var m) {
    return (m && m->type == ABS_MATRIX) ? m->val.matrix.rows : 0;
}

int abs_matrix_cols(var m) {
    return (m && m->type == ABS_MATRIX) ? m->val.matrix.cols : 0;
}

void abs_matrix_set(var m, int r, int c, double val) {
    if (!m || m->type != ABS_MATRIX) return;
    if (r < 0 || r >= m->val.matrix.rows) return;
    if (c < 0 || c >= m->val.matrix.cols) return;
    m->val.matrix.data[r * m->val.matrix.cols + c] = val;
}

double abs_matrix_get(var m, int r, int c) {
    if (!m || m->type != ABS_MATRIX) return 0.0;
    if (r < 0 || r >= m->val.matrix.rows) return 0.0;
    if (c < 0 || c >= m->val.matrix.cols) return 0.0;
    return m->val.matrix.data[r * m->val.matrix.cols + c];
}

var abs_matrix_mul(var A, var B) {
    if (!A || !B || A->type != ABS_MATRIX || B->type != ABS_MATRIX)
        return abs_new_error("Not a matrix");
    if (A->val.matrix.cols != B->val.matrix.rows)
        return abs_new_error("Dimension mismatch");
    int rows = A->val.matrix.rows;
    int cols = B->val.matrix.cols;
    int common = A->val.matrix.cols;
    var C = abs_matrix_new(rows, cols);
    if (is_err(C)) return C;
    abs_matrix_mul_kernel(A->val.matrix.data, B->val.matrix.data,
                          C->val.matrix.data, rows, cols, common);
    return C;
}

var abs_matrix_add(var A, var B) {
    if (!A || !B || A->type != ABS_MATRIX || B->type != ABS_MATRIX)
        return abs_new_error("Not a matrix");
    if (A->val.matrix.rows != B->val.matrix.rows ||
        A->val.matrix.cols != B->val.matrix.cols)
        return abs_new_error("Dimension mismatch");
    var C = abs_matrix_new(A->val.matrix.rows, A->val.matrix.cols);
    if (is_err(C)) return C;
    int n = A->val.matrix.rows * A->val.matrix.cols;
    for (int i = 0; i < n; i++)
        C->val.matrix.data[i] = A->val.matrix.data[i] + B->val.matrix.data[i];
    return C;
}

var abs_matrix_sub(var A, var B) {
    if (!A || !B || A->type != ABS_MATRIX || B->type != ABS_MATRIX)
        return abs_new_error("Not a matrix");
    if (A->val.matrix.rows != B->val.matrix.rows ||
        A->val.matrix.cols != B->val.matrix.cols)
        return abs_new_error("Dimension mismatch");
    var C = abs_matrix_new(A->val.matrix.rows, A->val.matrix.cols);
    if (is_err(C)) return C;
    int n = A->val.matrix.rows * A->val.matrix.cols;
    for (int i = 0; i < n; i++)
        C->val.matrix.data[i] = A->val.matrix.data[i] - B->val.matrix.data[i];
    return C;
}

var abs_matrix_scale(var m, double s) {
    if (!m || m->type != ABS_MATRIX) return abs_new_error("Not a matrix");
    var C = abs_matrix_new(m->val.matrix.rows, m->val.matrix.cols);
    if (is_err(C)) return C;
    int n = m->val.matrix.rows * m->val.matrix.cols;
    for (int i = 0; i < n; i++)
        C->val.matrix.data[i] = m->val.matrix.data[i] * s;
    return C;
}

void abs_matrix_apply(var m, double (*func)(double)) {
    if (!m || m->type != ABS_MATRIX || !func) return;
    int n = m->val.matrix.rows * m->val.matrix.cols;
    for (int i = 0; i < n; i++)
        m->val.matrix.data[i] = func(m->val.matrix.data[i]);
}

var abs_matrix_mul_element(var A, var B) {
    if (!A || !B || A->type != ABS_MATRIX || B->type != ABS_MATRIX)
        return abs_new_error("Not a matrix");
    if (A->val.matrix.rows != B->val.matrix.rows ||
        A->val.matrix.cols != B->val.matrix.cols)
        return abs_new_error("Dimension mismatch");
    var C = abs_matrix_new(A->val.matrix.rows, A->val.matrix.cols);
    if (is_err(C)) return C;
    int n = A->val.matrix.rows * A->val.matrix.cols;
    for (int i = 0; i < n; i++)
        C->val.matrix.data[i] = A->val.matrix.data[i] * B->val.matrix.data[i];
    return C;
}

var abs_matrix_add_scalar(var m, double s) {
    if (!m || m->type != ABS_MATRIX) return abs_new_error("Not a matrix");
    var C = abs_matrix_new(m->val.matrix.rows, m->val.matrix.cols);
    if (is_err(C)) return C;
    int n = m->val.matrix.rows * m->val.matrix.cols;
    for (int i = 0; i < n; i++)
        C->val.matrix.data[i] = m->val.matrix.data[i] + s;
    return C;
}

var abs_matrix_copy(var m) {
    if (!m || m->type != ABS_MATRIX) return abs_new_error("Not a matrix");
    var C = abs_matrix_new(m->val.matrix.rows, m->val.matrix.cols);
    if (is_err(C)) return C;
    int n = m->val.matrix.rows * m->val.matrix.cols;
    memcpy(C->val.matrix.data, m->val.matrix.data, (size_t)n * sizeof(double));
    return C;
}

void abs_matrix_add_row_vector(var m, var v) {
    if (!m || m->type != ABS_MATRIX || !v || v->type != ABS_MATRIX) return;
    if (v->val.matrix.rows != 1 || m->val.matrix.cols != v->val.matrix.cols) return;
    int cols = m->val.matrix.cols;
    for (int i = 0; i < m->val.matrix.rows; i++)
        for (int j = 0; j < cols; j++)
            m->val.matrix.data[i * cols + j] += v->val.matrix.data[j];
}

var abs_matrix_sum(var m) {
    if (!m || m->type != ABS_MATRIX) return abs_new_error("Not a matrix");
    double s = 0.0;
    int n = m->val.matrix.rows * m->val.matrix.cols;
    for (int i = 0; i < n; i++) s += m->val.matrix.data[i];
    return abs_new_float(s);
}

var abs_matrix_mean(var m) {
    if (!m || m->type != ABS_MATRIX) return abs_new_error("Not a matrix");
    int n = m->val.matrix.rows * m->val.matrix.cols;
    var s = abs_matrix_sum(m);
    if (is_err(s)) return s;
    return abs_new_float(abs_num_val(s) / (double)n);
}

var abs_matrix_min(var m) {
    if (!m || m->type != ABS_MATRIX) return abs_new_error("Not a matrix");
    int n = m->val.matrix.rows * m->val.matrix.cols;
    double mn = m->val.matrix.data[0];
    for (int i = 1; i < n; i++)
        if (m->val.matrix.data[i] < mn) mn = m->val.matrix.data[i];
    return abs_new_float(mn);
}

var abs_matrix_max(var m) {
    if (!m || m->type != ABS_MATRIX) return abs_new_error("Not a matrix");
    int n = m->val.matrix.rows * m->val.matrix.cols;
    double mx = m->val.matrix.data[0];
    for (int i = 1; i < n; i++)
        if (m->val.matrix.data[i] > mx) mx = m->val.matrix.data[i];
    return abs_new_float(mx);
}

long abs_matrix_argmax(var m) {
    if (!m || m->type != ABS_MATRIX) return -1;
    int n = m->val.matrix.rows * m->val.matrix.cols;
    long best = 0;
    double best_val = m->val.matrix.data[0];
    for (int i = 1; i < n; i++) {
        if (m->val.matrix.data[i] > best_val) {
            best_val = m->val.matrix.data[i];
            best = i;
        }
    }
    return best;
}

var abs_matrix_transpose(var m) {
    if (!m || m->type != ABS_MATRIX) return abs_new_error("Not a matrix");
    int rows = m->val.matrix.rows;
    int cols = m->val.matrix.cols;
    var t = abs_matrix_new(cols, rows);
    if (is_err(t)) return t;
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            t->val.matrix.data[j * rows + i] = m->val.matrix.data[i * cols + j];
    return t;
}

/* Laplace (cofactor) expansion on the raw flat buffer. */
static double det_rec(const double *a, int n) {
    if (n == 1) return a[0];
    if (n == 2) return a[0] * a[3] - a[1] * a[2];
    double det = 0.0;
    double *sub = (double *)malloc(sizeof(double) * (size_t)(n - 1) * (size_t)(n - 1));
    if (!sub) return 0.0;
    for (int col = 0; col < n; col++) {
        int idx = 0;
        for (int i = 1; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (j == col) continue;
                sub[idx++] = a[i * n + j];
            }
        }
        double sign = (col % 2 == 0) ? 1.0 : -1.0;
        det += sign * a[col] * det_rec(sub, n - 1);
    }
    free(sub);
    return det;
}

var abs_matrix_det(var m) {
    if (!m || m->type != ABS_MATRIX) return abs_new_error("Not a matrix");
    if (m->val.matrix.rows != m->val.matrix.cols)
        return abs_new_error("Matrix must be square");
    return abs_new_float(det_rec(m->val.matrix.data, m->val.matrix.rows));
}

void abs_matrix_print(var m) {
    if (!m || m->type != ABS_MATRIX) {
        printf("<not a matrix>");
        return;
    }
    int rows = m->val.matrix.rows;
    int cols = m->val.matrix.cols;
    printf("Matrix(%dx%d): [", rows, cols);
    for (int i = 0; i < rows; i++) {
        if (i > 0) printf(", ");
        printf("[");
        for (int j = 0; j < cols; j++) {
            if (j > 0) printf(", ");
            printf("%.2f", m->val.matrix.data[i * cols + j]);
        }
        printf("]");
    }
    printf("]");
}

/* --- Shape manipulation (NumPy style) --- */

var abs_matrix_reshape(var m, int rows, int cols) {
    if (!m || m->type != ABS_MATRIX) return abs_new_error("Not a matrix");
    if (rows <= 0 || cols <= 0)
        return abs_new_error("Invalid reshape dimensions");
    if (rows * cols != m->val.matrix.rows * m->val.matrix.cols)
        return abs_new_error("Cannot reshape array of given size");
    var r = abs_matrix_copy(m);
    if (is_err(r)) return r;
    r->val.matrix.rows = rows;
    r->val.matrix.cols = cols;
    return r;
}

var abs_matrix_flatten(var m) {
    if (!m || m->type != ABS_MATRIX) return abs_new_error("Not a matrix");
    return abs_matrix_reshape(m, 1, m->val.matrix.rows * m->val.matrix.cols);
}

var abs_matrix_slice(var m, int r0, int r1, int c0, int c1) {
    if (!m || m->type != ABS_MATRIX) return abs_new_error("Not a matrix");
    int rows = m->val.matrix.rows;
    int cols = m->val.matrix.cols;
    if (r0 < 0 || c0 < 0 || r1 > rows || c1 > cols || r0 >= r1 || c0 >= c1)
        return abs_new_error("Slice out of bounds");
    var s = abs_matrix_new(r1 - r0, c1 - c0);
    if (is_err(s)) return s;
    for (int i = r0; i < r1; i++)
        for (int j = c0; j < c1; j++)
            s->val.matrix.data[(i - r0) * (c1 - c0) + (j - c0)] =
                m->val.matrix.data[i * cols + j];
    return s;
}

var abs_matrix_vstack(var A, var B) {
    if (!A || !B || A->type != ABS_MATRIX || B->type != ABS_MATRIX)
        return abs_new_error("Not a matrix");
    if (A->val.matrix.cols != B->val.matrix.cols)
        return abs_new_error("Column mismatch");
    int cols = A->val.matrix.cols;
    var R = abs_matrix_new(A->val.matrix.rows + B->val.matrix.rows, cols);
    if (is_err(R)) return R;
    size_t na = (size_t)A->val.matrix.rows * (size_t)cols;
    size_t nb = (size_t)B->val.matrix.rows * (size_t)cols;
    memcpy(R->val.matrix.data, A->val.matrix.data, na * sizeof(double));
    memcpy(R->val.matrix.data + na, B->val.matrix.data, nb * sizeof(double));
    return R;
}

var abs_matrix_hstack(var A, var B) {
    if (!A || !B || A->type != ABS_MATRIX || B->type != ABS_MATRIX)
        return abs_new_error("Not a matrix");
    if (A->val.matrix.rows != B->val.matrix.rows)
        return abs_new_error("Row mismatch");
    int rows = A->val.matrix.rows;
    int acols = A->val.matrix.cols;
    int bcols = B->val.matrix.cols;
    var R = abs_matrix_new(rows, acols + bcols);
    if (is_err(R)) return R;
    for (int i = 0; i < rows; i++) {
        memcpy(R->val.matrix.data + (size_t)i * (acols + bcols),
               A->val.matrix.data + (size_t)i * acols,
               (size_t)acols * sizeof(double));
        memcpy(R->val.matrix.data + (size_t)i * (acols + bcols) + acols,
               B->val.matrix.data + (size_t)i * bcols,
               (size_t)bcols * sizeof(double));
    }
    return R;
}

/* --- Data generation --- */

var abs_matrix_ones(int rows, int cols) {
    var m = abs_matrix_new(rows, cols);
    if (is_err(m)) return m;
    int n = rows * cols;
    for (int i = 0; i < n; i++)
        m->val.matrix.data[i] = 1.0;
    return m;
}

var abs_matrix_arange(double start, double stop, double step) {
    if (step == 0.0) return abs_new_error("step cannot be zero");
    double span = stop - start;
    if (span * step <= 0.0) return abs_new_error("Empty range");
    int count = (int)ceil(span / step - 1e-9);
    if (count <= 0) return abs_new_error("Empty range");
    var m = abs_matrix_new(1, count);
    if (is_err(m)) return m;
    for (int i = 0; i < count; i++)
        m->val.matrix.data[i] = start + (double)i * step;
    return m;
}

var abs_matrix_linspace(double start, double stop, int steps) {
    if (steps <= 0) return abs_new_error("steps must be positive");
    if (steps == 1) {
        var one = abs_matrix_new(1, 1);
        if (is_err(one)) return one;
        one->val.matrix.data[0] = start;
        return one;
    }
    var m = abs_matrix_new(1, steps);
    if (is_err(m)) return m;
    double step = (stop - start) / (double)(steps - 1);
    for (int i = 0; i < steps; i++)
        m->val.matrix.data[i] = start + (double)i * step;
    return m;
}

/* --- Pandas-style numeric CSV I/O --- */

var abs_matrix_read_csv(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return abs_new_error("Cannot open file");
    char buf[8192];
    int rows = 0, cols = 0;
    while (fgets(buf, sizeof buf, fp)) {
        size_t n = strlen(buf);
        while (n > 0 && (buf[n - 1] == '\n' || buf[n - 1] == '\r')) buf[--n] = '\0';
        if (n == 0) continue;
        if (rows == 0) {
            for (const char *p = buf; *p; p++)
                if (*p == ',') cols++;
            cols++;
        }
        rows++;
    }
    if (rows == 0 || cols == 0) {
        fclose(fp);
        return abs_new_error("Empty CSV");
    }
    var m = abs_matrix_new(rows, cols);
    if (is_err(m)) {
        fclose(fp);
        return m;
    }
    rewind(fp);
    int r = 0;
    while (fgets(buf, sizeof buf, fp)) {
        size_t n = strlen(buf);
        while (n > 0 && (buf[n - 1] == '\n' || buf[n - 1] == '\r')) buf[--n] = '\0';
        if (n == 0) continue;
        const char *p = buf;
        for (int c = 0; c < cols; c++) {
            char *end;
            double v = strtod(p, &end);
            if (end == p) break;
            m->val.matrix.data[r * cols + c] = v;
            p = end;
            if (*p == ',') p++;
        }
        r++;
    }
    fclose(fp);
    return m;
}

void abs_matrix_write_csv(var m, const char *filename) {
    if (!m || m->type != ABS_MATRIX) return;
    FILE *fp = fopen(filename, "w");
    if (!fp) return;
    int rows = m->val.matrix.rows;
    int cols = m->val.matrix.cols;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fprintf(fp, "%.4f", m->val.matrix.data[i * cols + j]);
            if (j < cols - 1) fprintf(fp, ",");
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

/* --- Functional utils --- */

var abs_matrix_map(var m, double (*func)(double)) {
    if (!m || m->type != ABS_MATRIX) return abs_new_error("Not a matrix");
    var r = abs_matrix_copy(m);
    if (is_err(r)) return r;
    if (func) {
        int n = r->val.matrix.rows * r->val.matrix.cols;
        for (int i = 0; i < n; i++)
            r->val.matrix.data[i] = func(r->val.matrix.data[i]);
    }
    return r;
}

var abs_matrix_filter(var m, int (*predicate)(double)) {
    if (!m || m->type != ABS_MATRIX) return abs_new_error("Not a matrix");
    int total = m->val.matrix.rows * m->val.matrix.cols;
    int count = 0;
    for (int i = 0; i < total; i++)
        if (!predicate || predicate(m->val.matrix.data[i])) count++;
    if (count == 0) return abs_new_error("Filter selected no elements");
    var r = abs_matrix_new(1, count);
    if (is_err(r)) return r;
    int idx = 0;
    for (int i = 0; i < total; i++)
        if (!predicate || predicate(m->val.matrix.data[i]))
            r->val.matrix.data[idx++] = m->val.matrix.data[i];
    return r;
}
