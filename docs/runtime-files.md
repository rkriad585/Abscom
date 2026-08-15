# File Handles

Var-returning helpers for working with files. These wrap `FILE *` in an `ABS_FILE` object (see [file-io.md](file-io.md) for the raw C versions).

```c
#include "abscom/abs.h"
```

## API

| Function | Description |
| --- | --- |
| `var fopen_safe(const char *filename, const char *mode)` | Open a file and return a handle `var` (or an `ABS_ERROR`). |
| `var read_file(var file_obj)` | Read from the current position to end of file, as a string. |
| `void write_file(var file_obj, var content)` | Write `content` (stringified if not a string) at the current position. |
| `void close_file(var file_obj)` | Close the handle (sets the stored pointer to `NULL`). |

```c
abs_init();

var f = fopen_safe("out.txt", "w");
write_file(f, v("line one\n"));
close_file(f);

var g = fopen_safe("out.txt", "r");
var contents = read_file(g);
close_file(g);
print(contents);               /* line one */
```

## Notes

- `read_file` seeks to the end before reading, so it reads the whole remaining file from wherever the cursor currently is.
- `fopen_safe` returns `ABS_ERROR` ("Could not open file") when `fopen` fails.
- `read_file` on a non-file returns `ABS_ERROR`; `write_file`/`close_file` on non-files are silent no-ops.
- Forgetting `close_file` leaks the underlying `FILE *` — close handles when done.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
