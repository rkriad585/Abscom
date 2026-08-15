#include "abscom/abs.h"

#include <stdio.h>
#include <stdlib.h>

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
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double sum = 0.0;
            for (int k = 0; k < common; k++)
                sum += A->val.matrix.data[i * common + k] *
                       B->val.matrix.data[k * cols + j];
            C->val.matrix.data[i * cols + j] = sum;
        }
    }
    return C;
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
