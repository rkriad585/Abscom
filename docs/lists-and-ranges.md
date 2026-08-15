# Lists and Ranges

Lists are the primary sequence type. They are mutable, grow dynamically, support negative indexing, and have helpers to build ranges and slices.

```c
#include "abscom/abs.h"
```

## List operations

| Function | Description |
| --- | --- |
| `void append(var list, var item)` | Append an item to the end of a list. |
| `var get(var obj, long index)` | Index into a list or set; supports negative indexes. |
| `var len(var obj)` | Length of a string, list, dict, or set. |
| `long get_len_fast(var obj)` | Fast length for lists and sets (0 for everything else). |
| `var range(int start, int stop)` | List of `start..stop-1`. |
| `var range_step(int start, int stop, int step)` | Step-wise range; negative steps allowed. |
| `var slice(var obj, int start, int stop)` | New list of `list[start:stop]`; negative indexes allowed. |

### Building a list

```c
var xs = List();
append(xs, v(10));
append(xs, v(20));
append(xs, v(30));        /* [10, 20, 30] */

print(len(xs));           /* 3 */
```

`append` only accepts lists; calling it on a set does nothing.

### Negative indexes

```c
get(xs, -1);   /* 30 */
get(xs, -2);   /* 20 */
```

Out-of-range indexes produce an `ABS_ERROR` value.

### Ranges

`range` is **two-argument** — there is no single-argument form:

```c
var xs = range(0, 5);              /* [0, 1, 2, 3, 4]     */
var ys = range_step(1, 10, 2);     /* [1, 3, 5, 7, 9]     */
var zs = range_step(10, 0, -2);    /* [10, 8, 6, 4, 2]    */
var n  = range_step(5, 0, 0);      /* [] (step 0 is empty) */
```

### Slices

```c
var xs  = range(0, 6);             /* [0, 1, 2, 3, 4, 5] */
var sub = slice(xs, 1, 4);         /* [1, 2, 3]          */
var rev = slice(xs, -3, 6);        /* [3, 4, 5]          */
```

`slice` returns a fresh list; the source list is unchanged. It works on lists only (sets return an error).

## `get_len_fast` vs `len`

- `len` is safe and type-aware: it returns the length of a string, list, dict, or set (0 for anything else).
- `get_len_fast` skips the type switch — faster, but only meaningful for lists and sets, and returns 0 otherwise. It is what [foreach](loops.md) uses internally.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
