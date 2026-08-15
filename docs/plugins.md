# Plugins

Load a shared library at runtime (`LoadLibrary` on Windows, `dlopen` on POSIX) and call exported functions that follow the standard `var (*)(var)` signature.

```c
#include "abscom/abs.h"
```

| Function | Description |
| --- | --- |
| `var load_library(const char *path)` | Load a `.dll`/`.so`/`.dylib`; returns an `ABS_LIB` object or `ABS_ERROR`. |
| `var call_lib_func(var lib, const char *func_name, var arg)` | Call an exported function with one argument; returns its result. |

`call_lib_func` returns `None` when `lib` is not an `ABS_LIB` object, and `ABS_ERROR` if the library is unloaded or the symbol is missing.

```c
var lib = load_library("./math_plugins.dll");     /* or .so / .dylib */
if (is_err(lib)) {
    print(v("Could not load:"), lib);             /* includes dlerror() detail */
} else {
    var result = call_lib_func(lib, "plugin_square", v(7));
    print(result);                                /* 49 */
}
```

## Writing a plugin

The exported function must have the Abscom callable signature:

```c
#include "abscom/abs.h"

var plugin_square(var x) {
    return v(x->val.i * x->val.i);
}
```

- On Windows, build the plugin as a DLL that exports `plugin_square` (e.g. `__declspec(dllexport)` or a `.def` file).
- On POSIX, build it with `gcc -shared -fPIC plugin.c -o plugin.so`.
- On Linux the host must link `-ldl`; the Meson, CMake, and Make builds add this automatically. macOS provides `dlopen`/`dlsym` via libSystem — no extra flag.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
