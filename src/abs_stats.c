#include "abscom/abs.h"

#include <math.h>
#include <stdlib.h>

static bool is_num_obj(var o) {
    return o && (o->type == ABS_INT || o->type == ABS_FLOAT);
}

var abs_stats_mean(var list) {
    if (!list || list->type != ABS_LIST || list->val.list.size == 0)
        return abs_new_error("mean expects a non-empty list");
    double sum = 0.0;
    for (size_t i = 0; i < list->val.list.size; i++) {
        if (!is_num_obj(list->val.list.items[i]))
            return abs_new_error("mean expects numeric values");
        sum += abs_num_val(list->val.list.items[i]);
    }
    return abs_new_float(sum / (double)list->val.list.size);
}

var abs_stats_median(var list) {
    if (!list || list->type != ABS_LIST || list->val.list.size == 0)
        return abs_new_error("median expects a non-empty list");
    var s = sorted(list, false);
    if (is_err(s)) return s;
    size_t n = s->val.list.size;
    for (size_t i = 0; i < n; i++) {
        if (!is_num_obj(s->val.list.items[i]))
            return abs_new_error("median expects numeric values");
    }
    if (n % 2 == 1) return s->val.list.items[n / 2];
    double lo = abs_num_val(s->val.list.items[n / 2 - 1]);
    double hi = abs_num_val(s->val.list.items[n / 2]);
    return abs_new_float((lo + hi) / 2.0);
}

var abs_stats_mode(var list) {
    if (!list || list->type != ABS_LIST || list->val.list.size == 0)
        return abs_new_error("mode expects a non-empty list");
    var best = list->val.list.items[0];
    long best_count = 0;
    for (size_t i = 0; i < list->val.list.size; i++) {
        long c = 0;
        for (size_t j = 0; j < list->val.list.size; j++) {
            if (compare_objs(list->val.list.items[i], list->val.list.items[j]) == 0)
                c++;
        }
        if (c > best_count) {
            best_count = c;
            best = list->val.list.items[i];
        }
    }
    return best;
}

var abs_stats_variance(var list) {
    if (!list || list->type != ABS_LIST || list->val.list.size < 2)
        return abs_new_error("variance expects a list of at least 2 values");
    var m = abs_stats_mean(list);
    if (is_err(m)) return m;
    double mean = abs_num_val(m);
    double ss = 0.0;
    for (size_t i = 0; i < list->val.list.size; i++) {
        double d = abs_num_val(list->val.list.items[i]) - mean;
        ss += d * d;
    }
    return abs_new_float(ss / (double)list->val.list.size);
}

var abs_stats_stdev(var list) {
    var v = abs_stats_variance(list);
    if (is_err(v)) return v;
    return abs_new_float(sqrt(abs_num_val(v)));
}

static var num_func(double (*fn)(double), var x, const char *name) {
    if (!is_num_obj(x)) return abs_new_error(name);
    return abs_new_float(fn(abs_num_val(x)));
}

var sin_val(var x)   { return num_func(sin, x, "sin_val expects a number"); }
var cos_val(var x)   { return num_func(cos, x, "cos_val expects a number"); }
var tan_val(var x)   { return num_func(tan, x, "tan_val expects a number"); }
var log_val(var x)   { return num_func(log, x, "log_val expects a number"); }
var log10_val(var x) { return num_func(log10, x, "log10_val expects a number"); }
var sqrt_val(var x)  { return num_func(sqrt, x, "sqrt_val expects a number"); }

var deg2rad(var x) {
    if (!is_num_obj(x)) return abs_new_error("deg2rad expects a number");
    return abs_new_float(abs_num_val(x) * ABS_PI / 180.0);
}

static long long fact_rec(long n) {
    return (n <= 1) ? 1 : n * fact_rec(n - 1);
}

var factorial(var n) {
    if (!is_num_obj(n)) return abs_new_error("factorial expects a number");
    long v = (long)abs_num_val(n);
    if (v < 0) return abs_new_error("factorial expects a non-negative number");
    return abs_new_int((long)fact_rec(v));
}

var nCr(var n, var r) {
    if (!is_num_obj(n) || !is_num_obj(r)) return abs_new_error("nCr expects numbers");
    long N = (long)abs_num_val(n);
    long R = (long)abs_num_val(r);
    if (N < 0 || R < 0 || R > N)
        return abs_new_error("nCr: 0 <= r <= n required");
    long long a = fact_rec(N);
    long long b = fact_rec(R);
    long long c = fact_rec(N - R);
    if (b == 0 || c == 0) return abs_new_int(0);
    return abs_new_int((long)(a / (b * c)));
}

var nPr(var n, var r) {
    if (!is_num_obj(n) || !is_num_obj(r)) return abs_new_error("nPr expects numbers");
    long N = (long)abs_num_val(n);
    long R = (long)abs_num_val(r);
    if (N < 0 || R < 0 || R > N)
        return abs_new_error("nPr: 0 <= r <= n required");
    long long a = fact_rec(N);
    long long d = fact_rec(N - R);
    if (d == 0) return abs_new_int(0);
    return abs_new_int((long)(a / d));
}

/* --- Scalar utilities --- */

double abs_sq(double x) { return x * x; }

double abs_cb(double x) { return x * x * x; }

double abs_clamp(double x, double min, double max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

double abs_lerp(double a, double b, double t) {
    return a + t * (b - a);
}

int abs_eq(double a, double b) {
    return fabs(a - b) < ABS_EPSILON;
}

/* --- Number theory and discrete math --- */

long abs_gcd(long a, long b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b != 0) {
        long t = b;
        b = a % b;
        a = t;
    }
    return a;
}

long abs_lcm(long a, long b) {
    long g = abs_gcd(a, b);
    if (g == 0) return 0;
    long long r = ((long long)(a / g)) * b;
    return (long)(r < 0 ? -r : r);
}

long abs_factorial(int n) {
    if (n < 0) return 0;
    long long res = 1;
    for (int i = 2; i <= n; i++) res *= i;
    return (long)res;
}

int abs_is_prime(long n) {
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;
    for (long i = 5; i * i <= n; i += 6)
        if (n % i == 0 || n % (i + 2) == 0) return 0;
    return 1;
}

long abs_fibonacci(int n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;
    long long a = 0, b = 1;
    for (int i = 2; i <= n; i++) {
        long long c = a + b;
        a = b;
        b = c;
    }
    return (long)b;
}

/* --- Combinatorics (plain C ints) --- */

long abs_nPr(int n, int r) {
    if (r < 0 || r > n) return 0;
    long long res = 1;
    for (int i = 0; i < r; i++) res *= (n - i);
    return (long)res;
}

long abs_nCr(int n, int r) {
    if (r < 0 || r > n) return 0;
    if (r == 0 || r == n) return 1;
    if (r > n / 2) r = n - r;
    long long res = 1;
    for (int i = 0; i < r; i++) {
        res = res * (n - i) / (i + 1);
    }
    return (long)res;
}

/* --- Geometry --- */

double abs_rad2deg(double rad) {
    return rad * (180.0 / ABS_PI);
}

double abs_hypot(double a, double b) {
    return sqrt(a * a + b * b);
}

double abs_dist_euclidean(double x1, double y1, double x2, double y2) {
    double dx = x2 - x1;
    double dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}

double abs_dist_manhattan(double x1, double y1, double x2, double y2) {
    return fabs(x2 - x1) + fabs(y2 - y1);
}

/* --- Numerical analysis --- */

double abs_root_find(double (*f)(double), double (*f_prime)(double),
                     double guess) {
    double x = guess;
    for (int i = 0; i < 100; i++) {
        double y = f(x);
        double dy;
        if (f_prime) {
            dy = f_prime(x);
        } else {
            double h = 1e-6;
            dy = (f(x + h) - f(x - h)) / (2.0 * h);
        }
        if (fabs(dy) < ABS_EPSILON) break;
        double step = y / dy;
        x -= step;
        if (fabs(step) < ABS_EPSILON) return x;
    }
    return x;
}

/* --- Raw double-array statistics --- */

static int cmp_dbl(const void *a, const void *b) {
    double va = *(const double *)a;
    double vb = *(const double *)b;
    if (va < vb) return -1;
    if (va > vb) return 1;
    return 0;
}

double abs_stat_mean(double *arr, int size) {
    if (size <= 0 || !arr) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < size; i++) sum += arr[i];
    return sum / (double)size;
}

double abs_stat_median(double *arr, int size) {
    if (size <= 0 || !arr) return 0.0;
    double *temp = (double *)malloc((size_t)size * sizeof(double));
    if (!temp) return 0.0;
    for (int i = 0; i < size; i++) temp[i] = arr[i];
    qsort(temp, (size_t)size, sizeof(double), cmp_dbl);
    double med;
    if (size % 2 == 0)
        med = (temp[size / 2 - 1] + temp[size / 2]) / 2.0;
    else
        med = temp[size / 2];
    free(temp);
    return med;
}

double abs_stat_variance(double *arr, int size) {
    if (size <= 1 || !arr) return 0.0;
    double m = abs_stat_mean(arr, size);
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        double d = arr[i] - m;
        sum += d * d;
    }
    return sum / (double)size;
}

double abs_stat_stddev(double *arr, int size) {
    return sqrt(abs_stat_variance(arr, size));
}
