# Sequences and Sorting

Utilities that produce new sequences without mutating their inputs (except `shuffle`, which is covered in [random-utilities.md](random-utilities.md)).

```c
#include "abscom/abs.h"
```

## API

| Function | Description |
| --- | --- |
| `var sorted(var list, bool reverse)` | New list with the items ordered (ascending unless `reverse` is true). |
| `var reversed_seq(var list)` | New list with items in reverse order. |
| `var zip_lists(var list1, var list2)` | New list of `[a_i, b_i]` pairs, stopping at the shorter list. |

`sorted` and `reversed_seq` return fresh lists; the source is left untouched. Non-lists return `None`.

## Example

```c
abs_init();

var xs = List();
append(xs, v(3));
append(xs, v(1));
append(xs, v(2));

var asc = sorted(xs, false);      /* [1, 2, 3] */
var desc = sorted(xs, true);      /* [3, 2, 1] */
var rev = reversed_seq(xs);       /* [2, 1, 3] */

var a = range(0, 3);              /* [0, 1, 2] */
var b = List();
append(b, v("x"));
append(b, v("y"));
append(b, v("z"));
var pairs = zip_lists(a, b);      /* [[0, "x"], [1, "y"], [2, "z"]] */

print(asc);
print(desc);
print(rev);
print(pairs);
```

## Ordering

`sorted` orders with `compare_objs`: numeric lists sort numerically, string lists sort lexicographically. The `reverse` flag is required — pass `True`/`False`.

## Combining with ranges

`range` / `range_step` (see [lists-and-ranges.md](lists-and-ranges.md)) pair naturally with these helpers:

```c
var desc = reversed_seq(range(0, 5));   /* [4, 3, 2, 1, 0] */
```

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
