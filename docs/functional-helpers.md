# Functional Helpers

The runtime offers a small higher-order toolkit: map, filter, and list comprehensions, driven by C callbacks.

```c
#include "abscom/abs.h"
```

## Callback types

```c
typedef var  (*AbsFunc)(var);
typedef var  (*AbsMapFunc)(var);        /* same shape as AbsFunc */
typedef bool (*AbsFilterFunc)(var);
```

## API

| Function | Description |
| --- | --- |
| `var map_func(var list, AbsFunc f)` | New list with `f` applied to each item. |
| `var filter_func(var list, AbsFunc f)` | New list with items for which `f(item)` is truthy. |
| `var list_comp(var list, AbsMapFunc mapf, AbsFilterFunc filterf)` | `[map(x) for x in list if filter(x)]`; map first, filter second — either may be `NULL`. |

`filter_func` treats the callback result with the standard [truthiness rules](truthiness-and-logic.md). `list_comp` works on lists and sets; `map_func`/`filter_func` require lists.

## Example

```c
#include "abscom/abs.h"

static var double_it(var x) {
    return v(x->val.i * 2);
}

static var square_map(var x) {
    return v(x->val.i * x->val.i);
}

static var is_even(var x) {
    return (x->val.i % 2 == 0) ? v(True) : v(False);
}

int main(void) {
    abs_init();

    var xs = range(0, 6);                      /* [0,1,2,3,4,5] */

    var doubled = map_func(xs, double_it);     /* [0,2,4,6,8,10] */
    var evens   = filter_func(xs, is_even);    /* [0,2,4]        */
    var squares = list_comp(xs, square_map, is_even);  /* [0,4,16] */

    print(doubled);
    print(evens);
    print(squares);
    return 0;
}
```

## Notes

- Callbacks receive the element and must return a `var`. Returning `None` is fine.
- `filter_func` and `list_comp` decide membership with `is_true` on the filter result — returning `v(True)`/`v(False)` is the idiomatic way.
- Read a callback argument's payload with `obj->val.i`, `obj->val.f`, or `obj->val.s` after an `is_*` check.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
