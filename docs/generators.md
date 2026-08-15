# Generators

A minimal generator facility: `range_gen` produces a lazy sequence and `next` advances it, mirroring Python's `range` iterator.

```c
#include "abscom/abs.h"
```

| Function | Description |
| --- | --- |
| `var range_gen(long start, long stop, long step)` | A generator yielding `start, start+step, ...` while below `stop` (above for negative `step`). |
| `var next(var gen)` | The next value, or `None` once the generator is exhausted. |

`range_gen` returns an `ABS_GENERATOR` object. A `step` of `0` returns an error; negative steps count down. Calling `next` on a non-generator returns an error.

```c
var g = range_gen(0, 5, 2);
var n;
while ((n = next(g)) != NULL && !is_none(n)) {
    print(n);        /* 0  2  4 */
}
```

Generators pair naturally with the `foreach` macro:

```c
var item;
foreach (item, range_gen(10, 0, -3)) {
    print(item);     /* 10  7  4  1 */
}
```

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
