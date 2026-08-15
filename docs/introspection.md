# Introspection

Small helpers for inspecting objects at runtime: identity, a debug representation, and a list of keys.

```c
#include "abscom/abs.h"
```

| Function | Description |
| --- | --- |
| `var id(var obj)` | The object's memory address as an int. |
| `var repr(var obj)` | A debug string: `<AbsObj Type=<n> Addr=<p>>`. |
| `var dir(var obj)` | Dictionary keys, or list indices, as a list (empty for everything else). |

```c
var d = Dict();
dset(d, "name", v("abscom"));
dset(d, "level", v(9));

print(dir(d));        /* [name, level] */
print(id(d));         /* e.g. 696235936 */
print(repr(d));       /* <AbsObj Type=5 Addr=00000205...> */
```

Notes:

- `id` is the object pointer cast to an int — stable while the object lives, unique per live object.
- `repr` round-trips the `AbsType` value and the address; combine it with `type()` for a readable name.
- `dir` on a list yields `0, 1, 2, ...`; on a dict it yields the keys (insertion order is not guaranteed).
- Each `id()`/`repr()`/`dir()` call returns a fresh object, so compare values (e.g. `id(a)->val.i == id(b)->val.i`), not pointers.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
