# JSON

The runtime can serialize values to JSON and parse JSON strings back into `var` values.

```c
#include "abscom/abs.h"
```

## API

| Function | Description |
| --- | --- |
| `var json_dump(var obj)` | Serialize any value to a JSON string. |
| `var json_parse(const char *json_str)` | Parse a JSON C-string into a value. |

## Dumping

```c
abs_init();

var d = Dict();
dset(d, "name", abs_new_str("Ada"));
dset(d, "age",  v(36));
dset(d, "scores", range(0, 3));          /* [0, 1, 2] */

print(json_dump(d));
/* {"age":36,"name":"Ada","scores":[0,1,2]} */
```

Formatting rules:
- Ints and floats are unquoted; floats use full precision (`%.17g`).
- Strings are double-quoted and escaped (`\n`, `\"`, etc.).
- `None` becomes `null`, bools become `true`/`false`.
- Lists and sets become arrays; dicts become objects.

## Parsing

```c
var d = json_parse("{\"name\":\"Ada\",\"age\":36}");

print(dget(d, "name"));   /* Ada */
print(dget(d, "age"));    /* 36  */
```

The parser produces the equivalent `var` objects for objects, arrays, strings, numbers, booleans, and `null`. Unparseable input (or trailing garbage after the value) returns an `ABS_ERROR`.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
