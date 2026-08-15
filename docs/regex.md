# Regular Expressions

A small, self-contained regex engine built on the `var` object model: `re_match`, `re_findall`, and `re_sub`.

```c
#include "abscom/abs.h"
```

## Supported syntax

| Token | Meaning |
| --- | --- |
| `.` | Any single character. |
| `*` | Zero or more of the preceding element (greedy). |
| `^` | Anchor to the start of the input. |
| `$` | Anchor to the end of the input. |
| `abc` | Literal characters match themselves. |

`re_findall` reports every non-overlapping match. Zero-length matches advance by one character so a search always makes progress, and `^` only matches at the very start of the haystack.

## API

| Function | Description |
| --- | --- |
| `bool re_match(var pattern, var text)` | True if the whole text matches the pattern. |
| `var re_findall(var pattern, var text)` | List of all matched substrings. |
| `var re_sub(var pattern, var repl, var text)` | New string with every match replaced. |

Pattern, replacement, and text are given as `var` strings. Non-string inputs are treated as non-matches (returning `false` or an empty list).

```c
var email = v("admin@abscom.org");

if (re_match(v(".*@.*\\..*"), email)) {
    print(v("Email format valid"));        /* Email format valid */
}

print(re_findall(v("m"), v("mamma")));     /* [m, m] */

print(re_sub(v("o"), v("0"), v("hello"))); /* hell0 */
```

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
