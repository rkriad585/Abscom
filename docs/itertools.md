# Itertools

Lazy iterators over lists, following the spirit of Python's `itertools`. Both live on the `ABS_ITERATOR` type.

```c
#include "abscom/abs.h"
```

| Function | Description |
| --- | --- |
| `var chain(var list_a, var list_b)` | Yield every element of `a`, then every element of `b`. |
| `var cycle(var list)` | Yield a list's elements forever, wrapping at the end. |
| `var iter_next(var iter)` | The next element, or `None` once the iterator is exhausted. |

```c
var a = List();
append(a, v(1)); append(a, v(2));
var b = List();
append(b, v(10)); append(b, v(20));

var c = chain(a, b);
print(iter_next(c));   /* 1  */
print(iter_next(c));   /* 2  */
print(iter_next(c));   /* 10 */
print(iter_next(c));   /* 20 */
print(iter_next(c));   /* None (stays None afterwards) */

var cy = cycle(a);
print(iter_next(cy));   /* 1 */
print(iter_next(cy));   /* 2 */
print(iter_next(cy));   /* 1 */
print(iter_next(cy));   /* 2 */
```

Notes:

- `chain` treats a non-list source as empty, so `chain(a, None)` just drains `a`. `cycle` on an empty or non-list source yields `None` forever.
- `iter_next` on a non-iterator returns `ABS_ERROR`.

See `tests/test_itertools.c` for the full test.
