#include "abscom/abs.h"

#include <math.h>
#include <stdio.h>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static int close_to(double a, double b) {
    return fabs(a - b) < 1e-9;
}

static double quad(double x) { return x * x - 4.0; }   /* roots at +-2 */
static double quad_deriv(double x) { return 2.0 * x; }
static double cubic(double x) { return x * x * x - x; } /* roots at -1, 0, 1 */

int main(void) {
    abs_init();

    /* --- Scalar utilities --- */
    CHECK(close_to(abs_sq(5.0), 25.0));
    CHECK(close_to(abs_sq(-3.0), 9.0));
    CHECK(close_to(abs_cb(3.0), 27.0));
    CHECK(close_to(abs_cb(-2.0), -8.0));
    CHECK(close_to(abs_clamp(10.0, 0.0, 5.0), 5.0));
    CHECK(close_to(abs_clamp(-10.0, 0.0, 5.0), 0.0));
    CHECK(close_to(abs_clamp(2.5, 0.0, 5.0), 2.5));
    CHECK(close_to(abs_lerp(0.0, 10.0, 0.5), 5.0));
    CHECK(close_to(abs_lerp(0.0, 10.0, 0.0), 0.0));
    CHECK(close_to(abs_lerp(0.0, 10.0, 1.0), 10.0));
    CHECK(abs_eq(1.0, 1.0 + 1e-12));
    CHECK(!abs_eq(1.0, 1.0001));

    /* --- Number theory and discrete math --- */
    CHECK(abs_gcd(12, 8) == 4);
    CHECK(abs_gcd(-12, 8) == 4);
    CHECK(abs_gcd(7, 13) == 1);
    CHECK(abs_lcm(4, 6) == 12);
    CHECK(abs_lcm(3, 5) == 15);
    CHECK(abs_factorial(0) == 1);
    CHECK(abs_factorial(5) == 120);
    CHECK(abs_factorial(-3) == 0);
    CHECK(abs_is_prime(2));
    CHECK(abs_is_prime(3));
    CHECK(abs_is_prime(17));
    CHECK(!abs_is_prime(1));
    CHECK(!abs_is_prime(15));
    CHECK(!abs_is_prime(0));
    CHECK(abs_fibonacci(0) == 0);
    CHECK(abs_fibonacci(1) == 1);
    CHECK(abs_fibonacci(2) == 1);
    CHECK(abs_fibonacci(10) == 55);

    /* --- Combinatorics --- */
    CHECK(abs_nPr(5, 2) == 20);
    CHECK(abs_nPr(5, 0) == 1);
    CHECK(abs_nPr(3, 4) == 0);
    CHECK(abs_nCr(5, 2) == 10);
    CHECK(abs_nCr(5, 5) == 1);
    CHECK(abs_nCr(10, 3) == 120);
    CHECK(abs_nCr(4, 0) == 1);

    /* --- Geometry --- */
    CHECK(close_to(abs_rad2deg(ABS_PI), 180.0));
    CHECK(close_to(abs_rad2deg(0.0), 0.0));
    CHECK(close_to(abs_rad2deg(ABS_PI / 2), 90.0));
    CHECK(close_to(abs_hypot(3.0, 4.0), 5.0));
    CHECK(close_to(abs_hypot(0.0, 0.0), 0.0));
    CHECK(close_to(abs_dist_euclidean(0.0, 0.0, 3.0, 4.0), 5.0));
    CHECK(close_to(abs_dist_euclidean(1.0, 1.0, 4.0, 5.0), 5.0));
    CHECK(close_to(abs_dist_manhattan(0.0, 0.0, 3.0, 4.0), 7.0));
    CHECK(close_to(abs_dist_manhattan(1.0, 2.0, 4.0, 6.0), 7.0));

    /* --- Root finding --- */
    CHECK(close_to(fabs(abs_root_find(quad, quad_deriv, 3.0)), 2.0));
    CHECK(close_to(fabs(abs_root_find(quad, NULL, 3.0)), 2.0));
    CHECK(close_to(abs_root_find(cubic, NULL, 0.1), 0.0));

    /* --- Raw double-array statistics --- */
    double odd[5] = {3.0, 1.0, 2.0, 5.0, 4.0};
    double even[4] = {4.0, 1.0, 3.0, 2.0};
    CHECK(close_to(abs_stat_mean(odd, 5), 3.0));
    CHECK(close_to(abs_stat_mean(even, 4), 2.5));
    CHECK(close_to(abs_stat_median(odd, 5), 3.0));
    CHECK(close_to(abs_stat_median(even, 4), 2.5));
    CHECK(close_to(abs_stat_variance(odd, 5), 2.0));
    CHECK(close_to(abs_stat_stddev(odd, 5), sqrt(2.0)));

    /* --- Complex numbers --- */
    AbsComplex a = {3.0, 4.0};
    AbsComplex b = {1.0, -2.0};
    AbsComplex sum = abs_c_add(a, b);
    CHECK(close_to(sum.real, 4.0) && close_to(sum.imag, 2.0));
    AbsComplex diff = abs_c_sub(a, b);
    CHECK(close_to(diff.real, 2.0) && close_to(diff.imag, 6.0));
    AbsComplex prod = abs_c_mul(a, b);
    CHECK(close_to(prod.real, 11.0) && close_to(prod.imag, -2.0));
    CHECK(close_to(abs_c_mag(a), 5.0));
    AbsComplex cj = abs_c_conj(a);
    CHECK(close_to(cj.real, 3.0) && close_to(cj.imag, -4.0));
    AbsComplex z = {0.0, 0.0};
    AbsComplex zc = abs_c_mul(z, z);
    CHECK(close_to(zc.real, 0.0) && close_to(zc.imag, 0.0));

    /* --- Constants --- */
    CHECK(close_to(ABS_SQRT2, 1.41421356237309504880));
    CHECK(close_to(ABS_PHI, 1.61803398874989484820));
    CHECK(ABS_EPSILON == 1e-9);

    abs_cleanup();
    return 0;
}
