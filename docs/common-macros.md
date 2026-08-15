# Common Macros

The header `abs_common.h` defines the small set of macros shared by every other header. You rarely include it directly — `abs.h` pulls it in for you.

```c
#include "abscom/abs.h"        /* brings in abs_common.h */
```

## ABI and export macros

| Macro | Purpose |
| --- | --- |
| `ABS_API` | Export/import annotation on every public function. On Windows it expands to `__declspec(dllexport)` when the library itself is being built and `__declspec(dllimport)` for consumers; on GCC/Clang it expands to `__attribute__((visibility("default")))`. |
| `ABS_BUILDING_LIBRARY` | Define this when compiling the library sources so `ABS_API` becomes `dllexport`. `meson.build` sets it via `c_args`. |
| `ABS_USE_LIBRARY` | Define this in your own build when linking against the shared library on Windows so `ABS_API` becomes `dllimport`. |

If neither `ABS_BUILDING_LIBRARY` nor `ABS_USE_LIBRARY` is defined, `ABS_API` expands to nothing and you link the static library.

## C++ interop

| Macro | Purpose |
| --- | --- |
| `ABS_BEGIN_C_DECLS` | Expands to `extern "C" {` when compiling as C++, otherwise nothing. |
| `ABS_END_C_DECLS` | Expands to `}` when compiling as C++, otherwise nothing. |

Every public header wraps its declarations in these two macros so the library links cleanly from C++ programs.

## Utilities

| Macro | Purpose |
| --- | --- |
| `ABS_UNUSED(x)` | Casts `x` to `void`, suppressing unused-variable warnings. Useful in callback parameters you do not use. |

```c
static int visit(const char *key, void *value, void *user) {
    ABS_UNUSED(key);
    ABS_UNUSED(value);
    (*(size_t *)user)++;
    return 0;
}
```

## Include guards

Each header uses a unique guard of the form `ABSCOM_ABS_<NAME>_H`, e.g. `ABSCOM_ABS_STRING_H`. You never need to touch these; they only prevent double inclusion.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
