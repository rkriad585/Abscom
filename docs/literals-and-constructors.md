# Literals and Constructors

The dynamic runtime gives you a Python-flavoured API. Every value is a `var` (`AbsObj *`), created through constructor functions. Call `abs_init()` once before using anything from the runtime (see [lifecycle.md](lifecycle.md)).

```c
#include "abscom/abs.h"
```

## The `v()` literal macro

`v(x)` is a type-generic constructor that turns C values into `var` objects:

| C argument | Result |
| --- | --- |
| `int`, `long` | int value |
| `double`, `float` | float value |
| `char *`, `const char *` | string value |
| `bool` | bool value |

```c
var a = v(42);          /* int   */
var b = v(3.14);        /* float */
var c = v("hello");     /* str   */
var d = v(true);        /* bool  */
```

## Explicit constructors

| Function | Description |
| --- | --- |
| `var abs_new_int(long v)` | Int value. |
| `var abs_new_float(double v)` | Float value. |
| `var abs_new_str(const char *v)` | String value (copied). |
| `var abs_new_bool(bool v)` | Bool value. |
| `var abs_new_none(void)` | The `None` value. |
| `var abs_new_list(void)` | Empty list. |
| `var abs_new_dict(void)` | Empty dict. |
| `var abs_new_set(void)` | Empty set. |
| `var abs_new_error(const char *msg)` | Error value (see [error-handling.md](error-handling.md)). |

## Convenience macros

| Macro | Expands to |
| --- | --- |
| `None` | `abs_new_none()` |
| `True` | `((bool)1)` |
| `False` | `((bool)0)` |
| `List()` | `abs_new_list()` |
| `Dict()` | `abs_new_dict()` |
| `Set()` | `abs_new_set()` |

```c
abs_init();

var n = None;      /* ABS_NONE     */
var t = v(True);   /* ABS_BOOL 1   */   /* v() wraps the bool literal */
```

`True`/`False` are raw C `bool`s, handy for bool parameters (like `sorted(xs, True)`). Wrap them with `v(...)` when you need a `var` bool.

## The `AbsType` enum

Every object carries a type tag:

| Value | Meaning |
| --- | --- |
| `ABS_INT` | 64-bit integer (`long`). |
| `ABS_FLOAT` | Double-precision float. |
| `ABS_STR` | String. |
| `ABS_BOOL` | Boolean. |
| `ABS_LIST` | Mutable list. |
| `ABS_DICT` | String-keyed dictionary. |
| `ABS_SET` | Unique-item set. |
| `ABS_FILE` | Open file handle. |
| `ABS_ERROR` | Error value. |
| `ABS_NONE` | The `None` value. |
| `ABS_CLASS` | A class object. |
| `ABS_INSTANCE` | An instance of a class. |

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
