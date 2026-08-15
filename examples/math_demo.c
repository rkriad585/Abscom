/* General math, number theory, geometry, complex numbers and scalar statistics. */

#include "abscom/abs.h"

#include <stdio.h>

static double quad(double x) { return x * x - 4.0; }

int main(void) {
    abs_init();

    printf("=== General math ===\n");
    printf("sq(5) = %.1f, cb(3) = %.1f\n", abs_sq(5.0), abs_cb(3.0));
    printf("clamp(10, 0, 5) = %.1f\n", abs_clamp(10.0, 0.0, 5.0));
    printf("lerp(0, 10, 0.25) = %.1f\n", abs_lerp(0.0, 10.0, 0.25));
    printf("eq(0.1 + 0.2, 0.3) = %s\n", abs_eq(0.1 + 0.2, 0.3) ? "true" : "false");

    printf("\n=== Number theory ===\n");
    printf("gcd(12, 8) = %ld, lcm(4, 6) = %ld\n", abs_gcd(12, 8), abs_lcm(4, 6));
    printf("factorial(5) = %ld\n", abs_factorial(5));
    printf("is_prime(17) = %d, is_prime(15) = %d\n", abs_is_prime(17), abs_is_prime(15));
    printf("fib(10) = %ld, nPr(5,2) = %ld, nCr(5,2) = %ld\n",
           abs_fibonacci(10), abs_nPr(5, 2), abs_nCr(5, 2));

    printf("\n=== Geometry ===\n");
    printf("rad2deg(PI) = %.1f\n", abs_rad2deg(ABS_PI));
    printf("hypot(3, 4) = %.1f\n", abs_hypot(3.0, 4.0));
    printf("dist_euclidean(0,0 -> 3,4) = %.1f\n", abs_dist_euclidean(0.0, 0.0, 3.0, 4.0));
    printf("dist_manhattan(0,0 -> 3,4) = %.1f\n", abs_dist_manhattan(0.0, 0.0, 3.0, 4.0));

    printf("\n=== Root finding (Newton-Raphson) ===\n");
    printf("sqrt of x^2-4 near 3 = %.4f\n", abs_root_find(quad, NULL, 3.0));

    printf("\n=== Complex numbers ===\n");
    AbsComplex a = {3.0, 4.0};
    AbsComplex b = {1.0, -2.0};
    AbsComplex sum = abs_c_add(a, b);
    AbsComplex prod = abs_c_mul(a, b);
    AbsComplex conj = abs_c_conj(a);
    printf("a = "); abs_c_print(a);
    printf("\nb = "); abs_c_print(b);
    printf("\na + b = "); abs_c_print(sum);
    printf("\na * b = "); abs_c_print(prod);
    printf("\nconj(a) = "); abs_c_print(conj);
    printf("\n|a| = %.2f\n", abs_c_mag(a));

    printf("\n=== Scalar statistics ===\n");
    double data[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    printf("mean = %.2f, median = %.2f, variance = %.2f, stddev = %.2f\n",
           abs_stat_mean(data, 5), abs_stat_median(data, 5),
           abs_stat_variance(data, 5), abs_stat_stddev(data, 5));

    printf("\nConstants: PI = %.10f, E = %.10f, SQRT2 = %.10f, PHI = %.10f\n",
           ABS_PI, ABS_E, ABS_SQRT2, ABS_PHI);

    abs_cleanup();
    return 0;
}
