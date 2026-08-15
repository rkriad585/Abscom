# Sets

Sets hold unique `var` values. Adding an element that is already present is a no-op, which makes sets handy for de-duplication and membership tests.

```c
#include "abscom/abs.h"
```

## Operations

| Function | Description |
| --- | --- |
| `var Set()` | An empty set. |
| `void set_add(var set_obj, var item)` | Add an item (no-op if an equal item is already present). |
| `bool set_contains(var set_obj, var item)` | 1 if the set contains an equal item, 0 otherwise. |
| `var set_union(var a, var b)` | New set with items from both. |
| `var set_diff(var a, var b)` | New set with items in `a` but not in `b`. |

## Example

```c
abs_init();

var s = Set();
set_add(s, v(1));
set_add(s, v(2));
set_add(s, v(2));              /* no-op */

print(set_contains(s, v(1)));  /* True  */
print(set_contains(s, v(99))); /* False */
print(len(s));                 /* 2     */

var t = Set();
set_add(t, v(2));
set_add(t, v(3));

var u = set_union(s, t);       /* {1, 2, 3} */
var d = set_diff(s, t);        /* {1}       */
```

## Equality semantics

`set_contains` follows the same rules as [eq](truthiness-and-logic.md): numbers compare across int/float, so `v(1)` and `v(1.0)` match each other; strings compare by content.

## Notes

- Iterate sets with [foreach](loops.md).
- Sets do not preserve insertion order.
- To add items, always use `set_add` — `append` only works on lists.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
