# File I/O

`abs_fs` is a thin set of helpers around `fopen`, `remove`, and `rename`: check a file exists, read or write a whole file, delete it, or move it. All functions take platform-native paths.

```c
#include "abscom/abs.h"
```

## API

| Function | Description |
| --- | --- |
| `int abs_fs_exists(const char *path)` | 1 if the file can be opened for reading, 0 otherwise. |
| `int abs_fs_read_file(const char *path, char **out_data, size_t *out_size)` | Read a whole file into a NUL-terminated buffer (caller frees). |
| `int abs_fs_write_file(const char *path, const void *data, size_t size)` | Write `size` bytes to a file (creates or truncates). |
| `int abs_fs_remove(const char *path)` | Delete a file; 0 on success, -1 on failure. |
| `int abs_fs_rename(const char *from, const char *to)` | Rename/move a file; 0 on success, -1 on failure. |

Functions return `0` on success and `-1` on error. For `abs_fs_read_file`, `*out_data` is always NUL-terminated (`buf[n] == '\0'`), and the caller is responsible for `free(*out_data)`.

## Example

```c
#include "abscom/abs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char *data = "hello abscom file\nsecond line";

    if (abs_fs_write_file("notes.txt", data, strlen(data)) != 0) return 1;
    if (!abs_fs_exists("notes.txt")) return 1;

    char *buf = NULL;
    size_t n = 0;
    if (abs_fs_read_file("notes.txt", &buf, &n) == 0) {
        printf("read %zu bytes\n%s", n, buf);
        free(buf);
    }

    abs_fs_rename("notes.txt", "notes-copy.txt");
    abs_fs_remove("notes-copy.txt");
    return 0;
}
```

> For the Python-style `fopen_safe` / `read_file` / `write_file` / `close_file` helpers that return `var` objects, see [runtime-files.md](runtime-files.md).

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
