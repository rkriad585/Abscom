# Object-Oriented Light

The runtime has a minimal OOP-lite layer: create a named class, instantiate it with `New`, and store attributes on the instance.

```c
#include "abscom/abs.h"
```

## API

| Function | Description |
| --- | --- |
| `var Class(const char *name)` | A new class object with the given name. |
| `var New(var cls)` | A new instance of the class. |
| `void set_attr(var obj, const char *key, var val)` | Store an attribute (C-string key). |
| `var get_attr(var obj, const char *key)` | Read an attribute; `None` if missing. |

Class objects and instances are both `var` values; instances remember their class.

## Example

```c
abs_init();

var Point = Class("Point");

var p = New(Point);
set_attr(p, "x", v(3));
set_attr(p, "y", v(4));

print(get_attr(p, "x"));      /* 3   */
print(get_attr(p, "y"));      /* 4   */
print(p);                     /* <Point object> */
print(Point);                 /* <class 'Point'> */
```

## Reading missing attributes

`get_attr` on a key that was never set returns `None`:

```c
var p = New(Class("C"));
get_attr(p, "missing");       /* None */
```

## Notes

- Attributes live on the instance, keyed by C-string keys.
- Instances work with the normal value rules — store them in lists, dicts, and sets.
- `set_attr`/`get_attr` on a non-instance are a no-op / `None`, respectively.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
