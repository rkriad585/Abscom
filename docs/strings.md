# Strings

`abs_string` is a growable, NUL-terminated string buffer. It is the low-level building block that the Python-inspired runtime uses under the hood for all its text handling.

```c
#include "abscom/abs.h"
```

## Types

```c
typedef struct abs_string {
    char  *data;
    size_t len;
    size_t cap;
} abs_string_t;
```

The buffer is always NUL-terminated: `data[len]` is `'\0'`. `data` may be `NULL` for a completely empty string (in which case `abs_string_c_str` returns `""`).

## API

| Function | Description |
| --- | --- |
| `int abs_string_init(abs_string_t *s)` | Initialize an empty string. |
| `int abs_string_init_cstr(abs_string_t *s, const char *str)` | Initialize from a C string. |
| `int abs_string_init_n(abs_string_t *s, const char *str, size_t n)` | Initialize from the first `n` bytes. |
| `void abs_string_destroy(abs_string_t *s)` | Free the buffer. |
| `void abs_string_clear(abs_string_t *s)` | Empty the string, keeping capacity. |
| `int abs_string_reserve(abs_string_t *s, size_t cap)` | Reserve capacity for at least `cap` bytes. |
| `int abs_string_shrink_to_fit(abs_string_t *s)` | Trim capacity down to the current length. |
| `int abs_string_append_cstr(abs_string_t *s, const char *str)` | Append a C string. |
| `int abs_string_append_n(abs_string_t *s, const char *str, size_t n)` | Append `n` bytes. |
| `int abs_string_append_char(abs_string_t *s, char c)` | Append a single character. |
| `int abs_string_append_fmt(abs_string_t *s, const char *fmt, ...)` | Append `printf`-style formatted output. |
| `int abs_string_set_cstr(abs_string_t *s, const char *str)` | Replace the contents entirely. |
| `const char *abs_string_c_str(const abs_string_t *s)` | The NUL-terminated contents (never `NULL`). |
| `size_t abs_string_len(const abs_string_t *s)` | Current length in bytes. |
| `char *abs_string_take(abs_string_t *s)` | Transfer ownership of the buffer to the caller. |

Functions that can fail return `0` on success and `-1` on error.

## Example

```c
#include "abscom/abs.h"
#include <stdio.h>

int main(void) {
    abs_string_t s;
    abs_string_init(&s);

    abs_string_append_cstr(&s, "Hello, ");
    abs_string_append_fmt(&s, "world %d!", 42);
    abs_string_append_char(&s, '\n');

    printf("%s", abs_string_c_str(&s));          /* Hello, world 42! */
    printf("len=%zu\n", abs_string_len(&s));     /* 16 */

    char *mine = abs_string_take(&s);            /* ownership moves out */
    printf("taken: %s", mine);
    free(mine);

    abs_string_destroy(&s);
    return 0;
}
```

## `take` ownership rules

After `abs_string_take(&s)`, the buffer belongs to the caller (free it with `free()`), and `s` is reset to an empty state with `data == NULL`. If the string was empty, `take` returns `NULL`.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
