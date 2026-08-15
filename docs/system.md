# System and Processes

Platform helpers for sleeping, timing, running shell commands, and making simple HTTP requests.

```c
#include "abscom/abs.h"
```

## Sleep

| Function | Description |
| --- | --- |
| `void sleep_sec(double seconds)` | Block the calling thread for `seconds`. |

```c
print(v("waiting..."));
sleep_sec(1.0);
print(v("done"));
```

## Time

| Function | Description |
| --- | --- |
| `var time_now(void)` | Wall-clock seconds since the Unix epoch as a float. |

```c
var t0 = time_now();
sleep_sec(0.1);
var t1 = time_now();
print(sub(t1, t0));            /* ≈ 0.10 */
```

## Shell commands

| Function | Description |
| --- | --- |
| `var exec_cmd(const char *cmd)` | Run `cmd` through the platform shell; returns the exit status as an int. |

```c
var rc = exec_cmd("echo hello from abscom");
print(rc);                     /* 0 on success */
```

The command's stdout/stderr pass through to the process; the return value is the exit code.

## HTTP GET

| Function | Description |
| --- | --- |
| `var http_get(const char *url)` | GET a URL over plain HTTP and return the response body as a string. |

```c
var page = http_get("http://example.com/");
print(len(page));              /* body length in chars */
```

Constraints:
- Plain HTTP only: the client always connects on port 80 (no `https`).
- Blocking and synchronous.
- Returns the body after the first `\r\n\r\n`; no redirect following.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
