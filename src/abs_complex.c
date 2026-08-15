/* Complex numbers: a plain value type (AbsComplex) with basic arithmetic.
 * Unlike the matrix/var object model, complex values are passed by value —
 * they have no heap internals and need no GC tracking. */

#include "abscom/abs.h"

#include <math.h>
#include <stdio.h>

AbsComplex abs_c_add(AbsComplex a, AbsComplex b) {
    AbsComplex r = {a.real + b.real, a.imag + b.imag};
    return r;
}

AbsComplex abs_c_sub(AbsComplex a, AbsComplex b) {
    AbsComplex r = {a.real - b.real, a.imag - b.imag};
    return r;
}

AbsComplex abs_c_mul(AbsComplex a, AbsComplex b) {
    AbsComplex r = {
        a.real * b.real - a.imag * b.imag,
        a.real * b.imag + a.imag * b.real
    };
    return r;
}

double abs_c_mag(AbsComplex a) {
    return sqrt(a.real * a.real + a.imag * a.imag);
}

AbsComplex abs_c_conj(AbsComplex a) {
    AbsComplex r = {a.real, -a.imag};
    return r;
}

void abs_c_print(AbsComplex a) {
    if (a.imag < 0.0)
        printf("(%.2f - %.2fi)", a.real, -a.imag);
    else
        printf("(%.2f + %.2fi)", a.real, a.imag);
}
