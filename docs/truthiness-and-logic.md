# Truthiness, Logic, and Equality

Python-style truthiness applied to `var` values, plus the core boolean helpers.

```c
#include "abscom/abs.h"
```

## Truthiness rules

A value is **false** when it is any of:
- `False` (a bool with value 0)
- int or float zero (`v(0)`, `v(0.0)`)
- `None`
- an empty string, list, dict, or set
- any `ABS_ERROR` value

Everything else is **true**.

## API

| Function | Description |
| --- | --- |
| `bool is_true(var obj)` | 1 if the value is truthy. |
| `var not_(var obj)` | Logical negation (`True`/`False`). |
| `var any(var list)` | `True` if any item is truthy. |
| `var all(var list)` | `True` if every item is truthy. |
| `var eq(var a, var b)` | Equality (`True`/`False`). |

## Example

```c
abs_init();

print(v(is_true(v(1))));        /* True  */
print(v(is_true(None)));        /* False */
print(v(is_true(List())));      /* False (empty list) */

print(not_(v(0)));              /* True  */

var xs = List();
append(xs, v(0));
append(xs, v(0));
append(xs, v(1));
print(any(xs));                 /* True  */
print(all(xs));                 /* False */

print(eq(v(1), v(1)));          /* True  */
print(eq(v(1), v(1.0)));        /* True  (numbers compare numerically) */
print(eq(abs_new_str("a"), abs_new_str("a")));   /* True  */
```

## Equality semantics

- Numbers compare across int/float: `v(1)` equals `v(1.0)`.
- Bools compare with bools; strings compare by content; `None` equals `None`.
- Lists compare element-wise (element equality is recursive).
- Dicts and sets compare by structure only when both are the same kind.
- Mixed types are never equal.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
