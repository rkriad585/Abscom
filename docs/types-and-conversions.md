# Types and Conversions

Every runtime value is a `var` (`AbsObj *`) tagged with an `AbsType`. These helpers inspect and convert between types.

```c
#include "abscom/abs.h"
```

## The `AbsType` enum

See [literals-and-constructors.md](literals-and-constructors.md) for the full enum: `ABS_INT`, `ABS_FLOAT`, `ABS_STR`, `ABS_BOOL`, `ABS_LIST`, `ABS_DICT`, `ABS_SET`, `ABS_FILE`, `ABS_ERROR`, `ABS_NONE`, `ABS_CLASS`, `ABS_INSTANCE`.

## Type inspection

| Function | Description |
| --- | --- |
| `var type(var obj)` | The type name as a string, e.g. `"<class 'int'>"`. |
| `bool is_int(var o)` | True for `ABS_INT`. |
| `bool is_float(var o)` | True for `ABS_FLOAT`. |
| `bool is_str(var o)` | True for `ABS_STR`. |
| `bool is_list(var o)` | True for `ABS_LIST`. |
| `bool is_dict(var o)` | True for `ABS_DICT`. |
| `bool is_set(var o)` | True for `ABS_SET`. |
| `bool is_none(var o)` | True for `ABS_NONE` (also true for `NULL`). |
| `bool is_err(var o)` | True for `ABS_ERROR`. |

## Conversions

| Function | Description |
| --- | --- |
| `var to_str(var obj)` | Stringified value (see [formatting.md](formatting.md)). Strings pass through unchanged. |
| `var to_int(var obj)` | Int: floats truncate, bools map to 0/1, strings parse (`None` if unparseable). |
| `var to_float(var obj)` | Float: ints widen, bools map to 0.0/1.0, strings parse (`None` if unparseable). |

```c
abs_init();

var n = v(42);
print(is_int(n));               /* True  */
print(type(n));                 /* <class 'int'> */

var s = to_str(n);              /* "42"  */
print(type(s));                 /* <class 'str'> */

var f = to_float(v("3.25"));    /* 3.25  */
print(type(f));                 /* <class 'float'> */
```

## Reading payloads

The `AbsObj` union is public: `obj->val.i` (int), `obj->val.f` (float), `obj->val.s` (string), `obj->val.b` (bool). Check the type with `is_*` first, or use the conversions above — reading the wrong member is undefined behavior.

## The `str()` macro

`str(o)` is an alias for `to_str(o)`, handy inside `fmt` calls:

```c
var msg = fmt("sum = {}", str(sum_val(range(0, 6))));
```

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
