#include "abscom/abs.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static int close_to(double a, double b) {
    return fabs(a - b) < 1e-6;
}

static var nums_list(long *vals, size_t n) {
    var l = abs_new_list();
    for (size_t i = 0; i < n; i++) append(l, abs_new_int(vals[i]));
    return l;
}

int main(void) {
    abs_init();

    long data_vals[] = {10, 20, 20, 40};
    var data = nums_list(data_vals, 4);

    var m = abs_stats_mean(data);
    CHECK(!is_err(m) && close_to(m->val.f, 22.5));

    var med = abs_stats_median(data);
    CHECK(!is_err(med) && close_to(med->val.f, 20.0));

    var md = abs_stats_mode(data);
    CHECK(!is_err(md) && md->val.i == 20);

    var v = abs_stats_variance(data);
    CHECK(!is_err(v) && close_to(v->val.f, 118.75));

    var sd = abs_stats_stdev(data);
    CHECK(!is_err(sd) && close_to(sd->val.f, sqrt(118.75)));

    long odd_vals[] = {3, 1, 2};
    var odd = nums_list(odd_vals, 3);
    var omed = abs_stats_median(odd);
    CHECK(!is_err(omed) && close_to(abs_num_val(omed), 2.0));

    long even_vals[] = {4, 1, 3, 2};
    var even = nums_list(even_vals, 4);
    var emed = abs_stats_median(even);
    CHECK(!is_err(emed) && close_to(abs_num_val(emed), 2.5));

    CHECK(close_to(sin_val(deg2rad(v(90)))->val.f, 1.0));
    CHECK(close_to(cos_val(deg2rad(v(0)))->val.f, 1.0));
    CHECK(close_to(tan_val(deg2rad(v(45)))->val.f, 1.0));
    CHECK(close_to(log_val(v(1))->val.f, 0.0));
    CHECK(close_to(log10_val(v(100))->val.f, 2.0));
    CHECK(close_to(sqrt_val(v(9))->val.f, 3.0));
    CHECK(close_to(deg2rad(v(180))->val.f, 3.14159265358979));

    CHECK(factorial(v(5))->val.i == 120);
    CHECK(factorial(v(0))->val.i == 1);
    CHECK(nCr(v(5), v(2))->val.i == 10);
    CHECK(nPr(v(5), v(2))->val.i == 20);
    CHECK(nCr(v(10), v(3))->val.i == 120);
    CHECK(nPr(v(4), v(4))->val.i == 24);

    CHECK(is_err(abs_stats_mean(abs_new_list())));
    CHECK(is_err(factorial(v(-1))));
    CHECK(is_err(nCr(v(2), v(5))));

    abs_cleanup();
    return 0;
}
