# Macro Utilities

A comprehensive suite of small, dependency-free macros for everyday C work: numeric comparison and clamping, interpolation and shading curves, angle conversion, array and struct introspection, generic swapping, bitwise and power-of-two alignment helpers, and AI activation functions. Every macro wraps its arguments in parentheses to avoid operator-precedence bugs, and the short-name aliases are guard-checked so they never collide with code you already have.

```c
#include "abscom/abs.h"
```

## Arithmetic and comparison

| Macro | Result |
| --- | --- |
| `ABS_MIN(a, b)` / `ABS_MAX(a, b)` | the smaller / larger of `a` and `b` |
| `ABS_MIN3(a, b, c)` / `ABS_MAX3(a, b, c)` | min / max of three arguments |
| `ABS_MIN4(a, b, c, d)` / `ABS_MAX4(a, b, c, d)` | min / max of four arguments |
| `ABS_ABS(x)` | absolute value |
| `ABS_SIGN(x)` | `1` if positive, `-1` if negative, `0` if zero |
| `ABS_CLAMP(x, lo, hi)` | `x` clamped into `[lo, hi]` |
| `ABS_CLAMP01(x)` | `x` clamped into `[0, 1]` |
| `ABS_IN_RANGE(x, min, max)` | non-zero if `min <= x <= max` |

```c
double v = 14.8;
v = ABS_CLAMP(v, 0.0, 12.0);   /* 12.0 */
int flag = ABS_SIGN(-4);       /* -1 */
int hot = ABS_IN_RANGE(37.2, 36.5, 37.5);  /* 1 */
```

## Powers, differences, and approximate equality

| Macro | Result |
| --- | --- |
| `ABS_SQR(x)` | `x * x` |
| `ABS_CUBE(x)` | `x * x * x` |
| `ABS_DIFF(a, b)` | absolute difference `abs(a - b)` |
| `ABS_APPROX_EQ(a, b, eps)` | non-zero when `|a - b| <= eps` |

## Interpolation and shading curves

| Macro | Result |
| --- | --- |
| `ABS_LERP(a, b, t)` | `a + t * (b - a)` — linear interpolation between `a` and `b` |
| `ABS_UNLERP(a, b, val)` | inverse of `LERP`: the `t` that maps `a`→0 and `b`→1 |
| `ABS_REMAP(val, in_a, in_b, out_a, out_b)` | re-scale `val` from one range to another |
| `ABS_STEP(edge, x)` | `0.0` if `x < edge`, otherwise `1.0` |
| `ABS_SMOOTHSTEP(e0, e1, x)` | Hermite-smoothed `3t² - 2t³` step in `[0, 1]` |

`ABS_SMOOTHSTEP` is backed by a portable `static inline` function (evaluating its arguments exactly once), so it works on GCC, Clang, and MSVC alike.

```c
double t = ABS_LERP(0.0, 1.0, 0.5);            /* 0.5 */
double c = ABS_REMAP(2.5, 0.0, 5.0, -40.0, 125.0);   /* 42.5 */
double s = ABS_SMOOTHSTEP(0.0, 1.0, 0.75);     /* 0.84375 */
```

## Angle conversion

| Macro | Result |
| --- | --- |
| `ABS_DEG2RAD_M(deg)` | degrees → radians (`deg * ABS_PI / 180`) |
| `ABS_RAD2DEG_M(rad)` | radians → degrees (`rad * 180 / ABS_PI`) |

## Array, struct, and memory utilities

| Macro | Result |
| --- | --- |
| `ABS_ARRAY_LEN(arr)` | number of elements in a stack array (`sizeof(arr) / sizeof(arr[0])`) |
| `ABS_OFFSETOF(type, member)` | byte offset of `member` within `type` (no `<stddef.h>` dependency) |
| `ABS_CONTAINER_OF(ptr, type, member)` | recover the enclosing struct pointer from a member pointer |

```c
typedef struct { u32 id; f32 health; vec3 position; } Entity;
Entity e = {101, 100.0f, v3(1.0f, 2.0f, 3.0f)};

size_t off = ABS_OFFSETOF(Entity, position);   /* 8 */
Entity *back = ABS_CONTAINER_OF(&e.position, Entity, position);  /* &e */
```

## Generic swap

`ABS_SWAP(a, b)` swaps two values of any (identical) type — integers, floats, structs — via a `__typeof__` temporary on GCC/Clang. On compilers without `__typeof__`, use `ABS_SWAP_T(type, a, b)` which takes the type explicitly.

```c
vec2 p = v2(10.0f, 20.0f), q = v2(99.0f, 88.0f);
ABS_SWAP(p, q);   /* p = (99, 88), q = (10, 20) */
```

## Bitwise and alignment helpers

| Macro | Result |
| --- | --- |
| `ABS_BIT(n)` | `1ULL << n` |
| `ABS_BIT_SET(x, n)` | set bit `n` of `x` (in place) |
| `ABS_BIT_CLEAR(x, n)` | clear bit `n` of `x` (in place) |
| `ABS_BIT_TOGGLE(x, n)` | flip bit `n` of `x` (in place) |
| `ABS_BIT_CHECK(x, n)` | non-zero if bit `n` of `x` is set |
| `ABS_IS_POW2(x)` | non-zero if `x` is a power of two |
| `ABS_ALIGN_UP(x, align)` | round `x` up to a multiple of `align` |
| `ABS_ALIGN_DOWN(x, align)` | round `x` down to a multiple of `align` |

`align` must be a power of two (as is usual for alignment math).

```c
u32 flags = 0;
ABS_BIT_SET(flags, 3);
ABS_BIT_SET(flags, 5);            /* flags == 40 */
ABS_BIT_CHECK(flags, 3);          /* 1 */
ABS_BIT_CLEAR(flags, 3);          /* flags == 32 */

size_t up = ABS_ALIGN_UP(1040, 512);   /* 1536 */
int ok = ABS_IS_POW2(512);             /* 1 */
```

## AI activation helpers

| Macro | Result |
| --- | --- |
| `ABS_RELU_M(x)` | `max(0, x)` |
| `ABS_LEAKY_RELU_M(x, alpha)` | `x` if positive, else `alpha * x` |
| `ABS_HEAVISIDE_M(x)` | `1.0` if `x >= 0`, else `0.0` |

```c
double a = ABS_LEAKY_RELU_M(-4.5, 0.01);   /* -0.045 */
double b = ABS_RELU_M(-4.5);               /* 0.0 */
double c = ABS_HEAVISIDE_M(-4.5);          /* 0.0 */
```

## Short-name aliases

The most common macros also ship as guard-checked unprefixed aliases — each is defined only if a macro of that name does not already exist, so they cannot collide with your own code:

`MIN` · `MAX` · `CLAMP` · `CLAMP01` · `LERP` · `REMAP` · `SIGN` · `SQR` · `ARRAY_LEN` · `SWAP` · `DEG2RAD` · `RAD2DEG` · `BIT` · `IS_POW2`

```c
MIN(12.5, 48.2)          /* 12.5 */
CLAMP(14.8, 0.0, 12.0)   /* 12.0 */
LERP(100, 200, 0.5)      /* 150.0 */
ARRAY_LEN(samples)       /* element count */
DEG2RAD(90.0)            /* PI / 2 */
```

Prefer the `ABS_`-prefixed forms in library code and reserve the short names for application code where brevity matters.

Related: [Common Macros](common-macros.md) for `ABS_API` and header plumbing, [General Mathematics](general-math.md) for the function-based scalar utilities, and [Spatial Math](spatial-math.md) for vectors, matrices, and quaternions.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
