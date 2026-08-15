/* AI/ML layer: element-wise activation functions, row-wise softmax,
 * mean-squared-error loss, and a central-difference numerical gradient.
 * Matrices are Abscom's ABS_MATRIX values (see abs_matrix_* in abs_matrix.c). */

#include "abscom/abs.h"

#include <math.h>

double abs_act_sigmoid(double x) {
    return 1.0 / (1.0 + exp(-x));
}

double abs_act_relu(double x) {
    return (x > 0.0) ? x : 0.0;
}

double abs_act_tanh(double x) {
    return tanh(x);
}

double abs_diff_sigmoid(double y) {
    return y * (1.0 - y);
}

double abs_diff_relu(double y) {
    return (y > 0.0) ? 1.0 : 0.0;
}

double abs_diff_tanh(double y) {
    return 1.0 - y * y;
}

var abs_matrix_apply_deriv(var m, double (*func)(double)) {
    if (!m || m->type != ABS_MATRIX) return abs_new_error("Not a matrix");
    var c = abs_matrix_copy(m);
    if (is_err(c)) return c;
    if (func) abs_matrix_apply(c, func);
    return c;
}

void abs_matrix_softmax(var m) {
    if (!m || m->type != ABS_MATRIX) return;
    int rows = m->val.matrix.rows;
    int cols = m->val.matrix.cols;
    for (int i = 0; i < rows; i++) {
        /* Max over the row, for numerical stability. */
        double max_val = m->val.matrix.data[i * cols];
        for (int j = 1; j < cols; j++) {
            double v = m->val.matrix.data[i * cols + j];
            if (v > max_val) max_val = v;
        }
        /* Exponentiate and normalize the row. */
        double sum_exp = 0.0;
        for (int j = 0; j < cols; j++) {
            double e = exp(m->val.matrix.data[i * cols + j] - max_val);
            m->val.matrix.data[i * cols + j] = e;
            sum_exp += e;
        }
        if (sum_exp == 0.0) sum_exp = 1.0;
        for (int j = 0; j < cols; j++)
            m->val.matrix.data[i * cols + j] /= sum_exp;
    }
}

var abs_loss_mse(var y_true, var y_pred) {
    if (!y_true || y_true->type != ABS_MATRIX || !y_pred || y_pred->type != ABS_MATRIX)
        return abs_new_error("Not a matrix");
    if (y_true->val.matrix.rows != y_pred->val.matrix.rows ||
        y_true->val.matrix.cols != y_pred->val.matrix.cols)
        return abs_new_error("Dimension mismatch");
    int n = y_true->val.matrix.rows * y_true->val.matrix.cols;
    double sum_sq = 0.0;
    for (int i = 0; i < n; i++) {
        double d = y_true->val.matrix.data[i] - y_pred->val.matrix.data[i];
        sum_sq += d * d;
    }
    return abs_new_float(sum_sq / (double)n);
}

var abs_accuracy(var y_true, var y_pred) {
    if (!y_true || y_true->type != ABS_MATRIX || !y_pred || y_pred->type != ABS_MATRIX)
        return abs_new_error("Not a matrix");
    if (y_true->val.matrix.rows != y_pred->val.matrix.rows ||
        y_true->val.matrix.cols != y_pred->val.matrix.cols)
        return abs_new_error("Dimension mismatch");
    int rows = y_true->val.matrix.rows;
    int cols = y_true->val.matrix.cols;
    long correct = 0;
    for (int i = 0; i < rows; i++) {
        long best_true = 0;
        long best_pred = 0;
        double v_true = y_true->val.matrix.data[i * cols];
        double v_pred = y_pred->val.matrix.data[i * cols];
        for (int j = 1; j < cols; j++) {
            double a = y_true->val.matrix.data[i * cols + j];
            double b = y_pred->val.matrix.data[i * cols + j];
            if (a > v_true) { v_true = a; best_true = j; }
            if (b > v_pred) { v_pred = b; best_pred = j; }
        }
        if (best_true == best_pred) correct++;
    }
    return abs_new_float((double)correct / (double)rows);
}

var abs_grad(AbsFunc f, var x) {
    if (!f) return abs_new_error("abs_grad requires a function");
    if (!x || (x->type != ABS_INT && x->type != ABS_FLOAT))
        return abs_new_error("abs_grad expects a number");
    double xv = abs_num_val(x);
    double h = 1e-5;
    var fp = f(abs_new_float(xv + h));
    var fm = f(abs_new_float(xv - h));
    if (is_err(fp) || is_err(fm))
        return abs_new_error("abs_grad: function evaluation failed");
    return abs_new_float((abs_num_val(fp) - abs_num_val(fm)) / (2.0 * h));
}

/* One-hot encode a label vector. Labels may be a column (N x 1) or a row
 * (1 x N) matrix of class indices in [0, num_classes). */
var abs_matrix_one_hot_encode(var labels, int num_classes) {
    if (!labels || labels->type != ABS_MATRIX)
        return abs_new_error("Not a matrix");
    int rows = labels->val.matrix.rows;
    int cols = labels->val.matrix.cols;
    if (rows != 1 && cols != 1)
        return abs_new_error("Labels must be a 1D vector");
    if (num_classes <= 0)
        return abs_new_error("num_classes must be positive");
    int n = rows * cols;
    var oh = abs_matrix_new(n, num_classes);
    if (is_err(oh)) return oh;
    for (int i = 0; i < n; i++) {
        int cls = (int)labels->val.matrix.data[i];
        if (cls >= 0 && cls < num_classes)
            oh->val.matrix.data[i * num_classes + cls] = 1.0;
    }
    return oh;
}

/* Return a var list [X_train, X_test, Y_train, Y_test] using sequential
 * slicing; n_test = round(n_samples * test_ratio). */
var abs_matrix_train_test_split(var X, var Y, double test_ratio) {
    if (!X || X->type != ABS_MATRIX || !Y || Y->type != ABS_MATRIX)
        return abs_new_error("Not a matrix");
    if (X->val.matrix.rows != Y->val.matrix.rows)
        return abs_new_error("X and Y must have the same number of rows");
    if (test_ratio <= 0.0 || test_ratio >= 1.0)
        return abs_new_error("test_ratio must be in (0, 1)");
    int n = X->val.matrix.rows;
    if (n < 2)
        return abs_new_error("Need at least two samples to split");
    int n_test = (int)(n * test_ratio + 0.5);
    if (n_test <= 0) n_test = 1;
    if (n_test >= n) n_test = n - 1;
    int n_train = n - n_test;
    var list = List();
    append(list, abs_matrix_slice(X, 0, n_train, 0, X->val.matrix.cols));
    append(list, abs_matrix_slice(X, n_train, n, 0, X->val.matrix.cols));
    append(list, abs_matrix_slice(Y, 0, n_train, 0, Y->val.matrix.cols));
    append(list, abs_matrix_slice(Y, n_train, n, 0, Y->val.matrix.cols));
    return list;
}
