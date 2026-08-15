/* Scalar autograd: a Micrograd-style computational graph of double scalars
 * with reverse-mode differentiation (backpropagation). Nodes are plain heap
 * objects, not GC-tracked var values; freeing the root releases the whole
 * graph. The graph may share subtrees - backward() and free() both tolerate
 * that by traversing with the visited flag before touching any node. */

#include "abscom/abs.h"

#include <math.h>
#include <stdlib.h>

enum {
    ABS_SCALAR_LEAF = 0,
    ABS_SCALAR_ADD = 1,
    ABS_SCALAR_MUL = 2,
    ABS_SCALAR_RELU = 3,
    ABS_SCALAR_SIGMOID = 4
};

struct AbsScalar {
    double val;
    double grad;
    struct AbsScalar *parents[2];
    int op;
    int visited;  /* traversal marker for backward()/zero_grad()/free() */
};

static void abs_scalar_clear_visited(AbsScalar *v) {
    if (!v || !v->visited) return;
    v->visited = 0;
    abs_scalar_clear_visited(v->parents[0]);
    abs_scalar_clear_visited(v->parents[1]);
}

static AbsScalar *abs_scalar_node(double val, int op,
                                  AbsScalar *a, AbsScalar *b) {
    AbsScalar *v = (AbsScalar *)malloc(sizeof(AbsScalar));
    if (!v) return NULL;
    v->val = val;
    v->grad = 0.0;
    v->parents[0] = a;
    v->parents[1] = b;
    v->op = op;
    v->visited = 0;
    return v;
}

AbsScalar *abs_scalar_new(double val) {
    return abs_scalar_node(val, ABS_SCALAR_LEAF, NULL, NULL);
}

AbsScalar *abs_scalar_add(AbsScalar *a, AbsScalar *b) {
    if (!a || !b) return NULL;
    return abs_scalar_node(a->val + b->val, ABS_SCALAR_ADD, a, b);
}

AbsScalar *abs_scalar_mul(AbsScalar *a, AbsScalar *b) {
    if (!a || !b) return NULL;
    return abs_scalar_node(a->val * b->val, ABS_SCALAR_MUL, a, b);
}

AbsScalar *abs_scalar_relu(AbsScalar *a) {
    if (!a) return NULL;
    return abs_scalar_node(a->val > 0.0 ? a->val : 0.0, ABS_SCALAR_RELU, a, NULL);
}

AbsScalar *abs_scalar_sigmoid(AbsScalar *a) {
    if (!a) return NULL;
    double s = 1.0 / (1.0 + exp(-a->val));
    return abs_scalar_node(s, ABS_SCALAR_SIGMOID, a, NULL);
}

double abs_scalar_val(AbsScalar *v) {
    return v ? v->val : 0.0;
}

double abs_scalar_grad(AbsScalar *v) {
    return v ? v->grad : 0.0;
}

void abs_scalar_zero_grad(AbsScalar *root) {
    if (!root || root->visited) return;
    root->visited = 1;
    root->grad = 0.0;
    abs_scalar_zero_grad(root->parents[0]);
    abs_scalar_zero_grad(root->parents[1]);
    abs_scalar_clear_visited(root);
}

static int abs_scalar_count(AbsScalar *v) {
    if (!v || v->visited) return 0;
    v->visited = 1;
    int n = 1;
    n += abs_scalar_count(v->parents[0]);
    n += abs_scalar_count(v->parents[1]);
    return n;
}

static void abs_scalar_build_order(AbsScalar *v, AbsScalar **order,
                                   int *count, int cap) {
    if (!v || v->visited) return;
    v->visited = 1;
    abs_scalar_build_order(v->parents[0], order, count, cap);
    abs_scalar_build_order(v->parents[1], order, count, cap);
    if (*count < cap) order[(*count)++] = v;
}

static void abs_scalar_backward_node(AbsScalar *v) {
    double g = v->grad;
    switch (v->op) {
        case ABS_SCALAR_ADD:
            v->parents[0]->grad += g;
            v->parents[1]->grad += g;
            break;
        case ABS_SCALAR_MUL:
            v->parents[0]->grad += g * v->parents[1]->val;
            v->parents[1]->grad += g * v->parents[0]->val;
            break;
        case ABS_SCALAR_RELU:
            /* ReLU derivative at the *input*: > 0 means the gate is open. */
            v->parents[0]->grad += (v->val > 0.0) ? g : 0.0;
            break;
        case ABS_SCALAR_SIGMOID:
            /* d/dx sigmoid = sigmoid(x) * (1 - sigmoid(x)). */
            v->parents[0]->grad += g * v->val * (1.0 - v->val);
            break;
        default:
            break; /* leaf */
    }
}

void abs_scalar_backward(AbsScalar *root) {
    if (!root) return;
    abs_scalar_zero_grad(root);

    int n = abs_scalar_count(root);
    abs_scalar_clear_visited(root);

    AbsScalar **order = (AbsScalar **)malloc((size_t)n * sizeof(AbsScalar *));
    if (!order) return;
    int count = 0;
    abs_scalar_build_order(root, order, &count, n);
    abs_scalar_clear_visited(root);

    root->grad = 1.0;
    /* Reverse topological order: outputs first, inputs last. */
    for (int i = count - 1; i >= 0; i--)
        abs_scalar_backward_node(order[i]);
    free(order);
}

void abs_scalar_free(AbsScalar *root) {
    if (!root) return;

    /* Freeing the root releases the whole graph. Shared subtrees must be
     * freed exactly once, so first collect every unique reachable node into
     * an array while all of them are still allocated, then free that list.
     * A guard stored inside a node cannot work here: once a node is freed,
     * reading its guard back is itself a use-after-free. */
    int n = abs_scalar_count(root);
    abs_scalar_clear_visited(root);
    AbsScalar **order = (AbsScalar **)malloc((size_t)n * sizeof(AbsScalar *));
    if (!order) return; /* OOM: leak rather than read freed memory */
    int count = 0;
    abs_scalar_build_order(root, order, &count, n);
    for (int i = 0; i < count; i++) free(order[i]);
    free(order);
}
