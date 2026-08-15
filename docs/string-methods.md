# String Methods

Strings carry a small set of text helpers. Most return new `var` objects; the source string is never modified.

```c
#include "abscom/abs.h"
```

## Case

| Function | Description |
| --- | --- |
| `var upper(var obj)` | Uppercase copy (non-strings return `None`). |
| `var lower(var obj)` | Lowercase copy (non-strings return `None`). |

## Splitting and joining

| Function | Description |
| --- | --- |
| `var split(var str_obj, const char *delimiter)` | Split on a C-string delimiter, returning a list of parts. |
| `var join(var delimiter, var list_obj)` | Join the string items of a list with a delimiter (delimiter first!). |

`split` with an empty delimiter splits into single characters. An empty string splits into `[""]`. `join` concatenates the string items of a list, inserting `delimiter` (a `var` string) between them; non-string items are skipped.

## Trimming and searching

| Function | Description |
| --- | --- |
| `var strip(var str_obj)` | Copy with leading/trailing whitespace removed. |
| `var startswith(var str, var prefix)` | `True`/`False` (both arguments are `var` strings). |
| `var endswith(var str, var suffix)` | `True`/`False` (both arguments are `var` strings). |
| `var count(var container, var item)` | Number of non-overlapping occurrences: substring count for strings, equal-item count for lists. |

## Example

```c
abs_init();

var s = abs_new_str("Hello, World");
print(upper(s));                    /* HELLO, WORLD */
print(lower(s));                    /* hello, world */

var parts = split(s, ", ");         /* ["Hello", "World"]  */
var back  = join(v("|"), parts);    /* "Hello|World"       */

print(startswith(s, v("Hello")));   /* True  */
print(endswith(s, v("World")));     /* True  */
print(count(s, v("l")));            /* 3     */
print(strip(v("  padded  ")));      /* padded */
```

## Formatting strings

For building strings with placeholders, use [fmt](formatting.md) — the `str()` alias for `to_str` is handy for converting values:

```c
var msg = fmt("result: {}", str(sum_val(range(0, 5))));
```

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
