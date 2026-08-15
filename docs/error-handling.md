# Error Handling

Runtime operations that cannot succeed produce an `ABS_ERROR` value instead of crashing. You test for it with `is_err` and inspect the message.

```c
#include "abscom/abs.h"
```

## API

| Function | Description |
| --- | --- |
| `var abs_new_error(const char *msg)` | Build a new error value. |
| `bool is_err(var obj)` | 1 if the object is an error. |

Error values carry a message string; printing one shows `Error: <msg>`, and `to_str` gives just `<msg>`.

## Checking results

```c
abs_init();

var xs = List();
var too_far = get(xs, 5);            /* empty list, index 5 */

if (is_err(too_far)) {
    print(abs_new_str("lookup failed"));
    print(too_far);                  /* Error: Index out of bounds */
}
```

## Common failure points

| Operation | Fails when |
| --- | --- |
| `get` / `slice` | index out of bounds, or object is not a list/set |
| `split` | object is not a string |
| `join` | object is not a list |
| `choice` / `choices` | sequence is empty |
| `sample` | `k` larger than the list, or non-list |
| `json_parse` | input is not valid JSON or has trailing garbage |
| `fopen_safe`, `read_file` | file cannot be opened/read |
| `http_get` | socket, host, or connect failure |
| `dget` | object is not a dict |

## Successful but "missing" results

Some lookups return `None` rather than an error on missing data:
- `dget` on a missing key → `None`
- `get_attr` on a missing attribute → `None`
- `to_int`/`to_float` on an unparseable string → `None`
- `add`/`sub`/`mul` on incompatible types → `None`

## Creating errors

You can raise your own errors too:

```c
var val = v(-3);
if (val->val.i < 0) {
    var e = abs_new_error("value must be non-negative");
    print(e);                    /* Error: value must be non-negative */
}
```

## Errors are falsey

`is_true` returns false for `ABS_ERROR` values, so they work naturally as guard conditions in [functional helpers](functional-helpers.md).

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
