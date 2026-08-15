# Hashing

`abs_hash` provides two classic string-friendly hash algorithms: FNV-1a (32- and 64-bit) and djb2. The hash map module (`abs_hashmap`) is built on the 64-bit FNV-1a variant for its string keys.

```c
#include "abscom/abs.h"
```

## API

| Function | Description |
| --- | --- |
| `uint32_t abs_hash_fnv1a32(const void *data, size_t len)` | FNV-1a 32-bit hash over `len` bytes. |
| `uint64_t abs_hash_fnv1a64(const void *data, size_t len)` | FNV-1a 64-bit hash over `len` bytes. |
| `uint64_t abs_hash_fnv1a64_str(const char *str)` | FNV-1a 64-bit hash of a NUL-terminated string. |
| `uint32_t abs_hash_djb2(const char *str)` | djb2 32-bit hash of a NUL-terminated string. |

## Example

```c
#include "abscom/abs.h"
#include <stdio.h>

int main(void) {
    printf("fnv1a32(\"hello\") = %u\n", abs_hash_fnv1a32("hello", 5));
    printf("fnv1a64(\"hello\") = %llu\n",
           (unsigned long long)abs_hash_fnv1a64_str("hello"));
    printf("djb2(\"hello\")    = %u\n", abs_hash_djb2("hello"));
    return 0;
}
```

## Known test vectors

The test suite pins these values so the algorithms never silently change:

```c
abs_hash_fnv1a32("hello", 5) == 0x4f9f2cabu
abs_hash_djb2("")            == 5381u
```

Hashing the same bytes always yields the same value; these are not cryptographic hashes, so do not use them for security purposes.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
