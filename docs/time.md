# Time

`abs_time` exposes monotonic and wall-clock timers behind four simple functions. On Windows it uses `QueryPerformanceCounter`/`GetSystemTimeAsFileTime`; on POSIX it uses `clock_gettime`. No configuration is required.

```c
#include "abscom/abs.h"
```

## API

| Function | Description |
| --- | --- |
| `double abs_time_now(void)` | Monotonic time in seconds (safe for elapsed-time measurement). |
| `double abs_time_wall(void)` | Wall-clock time in seconds since the Unix epoch. |
| `uint64_t abs_time_now_ns(void)` | Monotonic time in nanoseconds. |
| `uint64_t abs_time_wall_ms(void)` | Wall-clock time in milliseconds. |

`abs_time_now` is the right choice for benchmarking and measuring intervals because it never jumps (unlike wall time, which can change via NTP or manual adjustment).

## Example

```c
#include "abscom/abs.h"
#include <stdio.h>

int main(void) {
    double start = abs_time_now();

    volatile double acc = 0.0;
    for (int i = 0; i < 1000000; i++) acc += (double)i;
    ABS_UNUSED(acc);

    double elapsed = abs_time_now() - start;
    printf("loop took %.6f s\n", elapsed);
    printf("wall clock: %.3f\n", abs_time_wall());
    printf("now_ns:     %llu\n", (unsigned long long)abs_time_now_ns());
    printf("wall_ms:    %llu\n", (unsigned long long)abs_time_wall_ms());
    return 0;
}
```

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
