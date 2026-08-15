# Loops

The runtime ships a `foreach` macro that walks the items of a list or set without you touching indexes or lengths.

```c
#include "abscom/abs.h"
```

## `foreach`

```c
foreach (var item, var collection) {
    /* body */
}
```

`foreach` works on **lists** and **sets**. Inside the body, `item` is bound to each element in order. The macro expands to a `for` loop built on `get_len_fast` and `get`.

## Example

```c
abs_init();

var xs = range(0, 5);
foreach (var x, xs) {
    print(x);
}

/* 0 1 2 3 4 (one per line) */
```

Iterating a set visits each member once, in no guaranteed order (same as Python):

```c
var s = Set();
set_add(s, v("a"));
set_add(s, v("b"));
foreach (var item, s) {
    print(item);
}
```

## Non-iterables

`foreach` relies on `get_len_fast`, which returns 0 for anything that is not a list or set — so passing an int, string, dict, or `None` simply iterates zero times rather than raising an error.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
