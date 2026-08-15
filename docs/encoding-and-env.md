# Encoding and Environment

Base64 encoding, random UUID v4 generation, and environment-variable access — the last pieces of the language layer.

```c
#include "abscom/abs.h"
```

## Base64

| Function | Description |
| --- | --- |
| `var base64_encode(var str_obj)` | Base64-encode a string with standard `=` padding. |

```c
var b = base64_encode(v("hello"));
print(b);   /* aGVsbG8= */
```

## UUIDs

| Function | Description |
| --- | --- |
| `var uuid4(void)` | A random version-4 UUID string (36 characters, `8-4-4-4-12` shape). |

```c
print(uuid4());
/* 3c836059-65e7-45fd-87ac-4038737841fe */
```

The version nibble is always `4` and the variant nibble is one of `8`/`9`/`a`/`b`, matching RFC 4122.

## Environment variables

| Function | Description |
| --- | --- |
| `var os_getenv(const char *key)` | The value of the environment variable, or `None` if unset. |
| `void os_setenv(const char *key, const char *val)` | Set (or overwrite) an environment variable. |

```c
os_setenv("ABS_MODE", "Pro");
var mode = os_getenv("ABS_MODE");
print(v("Mode:"), mode);   /* Mode: Pro */
```

On Windows, `os_setenv` uses a heap-allocated `putenv` so the value outlives the call; on POSIX it uses `setenv(key, val, 1)`.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
