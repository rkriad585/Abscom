#include "abscom/abs.h"

#include <math.h>

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
    return abs_new_float(abs_num_val(x) * 3.14159265358979323846 / 180.0);
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
