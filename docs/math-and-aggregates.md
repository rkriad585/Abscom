# Aggregates and Math

Helpers that reduce collections and wrap simple arithmetic on `var` values.

```c
#include "abscom/abs.h"
```

## Aggregates

All three take a **list** (not two values).

| Function | Description |
| --- | --- |
| `var min_val(var list)` | Smallest item by numeric/string ordering; `None` if empty. |
| `var max_val(var list)` | Largest item; `None` if empty. |
| `var sum_val(var list)` | Sum of a numeric list (int if all ints, otherwise float). |

```c
var xs = range(0, 5);        /* [0,1,2,3,4] */
print(min_val(xs));          /* 0           */
print(max_val(xs));          /* 4           */
print(sum_val(xs));          /* 10          */
```

## Arithmetic

| Function | Description |
| --- | --- |
| `var add(var a, var b)` | Numeric addition; concatenates two strings, or two lists. |
| `var sub(var a, var b)` | Numeric subtraction (non-numbers return `None`). |
| `var mul(var a, var b)` | Numeric multiplication; also repeats a string when one operand is an int. |
| `var abs_val(var obj)` | Absolute value of a number (non-numbers return 0). |
| `var pow_val(var base, var exp)` | `base` raised to `exp`; always returns a float. |
| `var round_val(var obj, int digits)` | Round a float; `digits == 0` returns an int; non-floats pass through unchanged. |

```c
print(add(v(2), v(3)));          /* 5     */
print(sub(v(10), v(4)));         /* 6     */
print(mul(v(6), v(7)));          /* 42    */
print(abs_val(v(-5)));           /* 5     */
print(pow_val(v(2), v(10)));     /* 1024.0 */
print(round_val(v(3.14159), 2)); /* 3.14   */

print(add(abs_new_str("foo"), abs_new_str("bar")));  /* foobar  */
print(mul(abs_new_str("ab"), v(3)));                 /* ababab  */
```

## Comparison

| Function | Description |
| --- | --- |
| `int compare_objs(var a, var b)` | -1, 0, or 1 using numeric or string ordering. |
| `var eq(var a, var b)` | Equality as `True`/`False` (see [truthiness-and-logic.md](truthiness-and-logic.md)). |

`min_val`/`max_val` order items with `compare_objs`, so they work on numeric or string lists.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
