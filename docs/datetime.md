# Date and Time

Wall-clock date/time helpers built on the C standard library's `struct tm`, exposed as `ABS_TIME` objects.

```c
#include "abscom/abs.h"
```

| Function | Description |
| --- | --- |
| `var datetime_now(void)` | Current local time as an `ABS_TIME` object. |
| `var strftime_val(const char *fmt, var time_obj)` | Format a time object with `strftime`-style codes; `None` on non-time input. |
| `var timedelta(int days, int seconds)` | The time `days` and `seconds` from now (accepts negative values). |

```c
var now = datetime_now();
print(strftime_val("%Y-%m-%d %H:%M:%S", now));   /* 2026-08-15 14:40:14 */

var tomorrow = timedelta(1, 0);
print(strftime_val("%Y-%m-%d", tomorrow));       /* 2026-08-16 */
```

`strftime_val` accepts the usual `strftime` conversion specifiers (`%Y` year, `%m` month, `%d` day, `%H` hour, `%M` minute, `%S` second, and friends), so you can render dates however you like.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
