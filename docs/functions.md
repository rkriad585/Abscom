# Functions, Decorators, and Memoization

First-class function objects (`ABS_FUNC`) let you wrap C functions as `var` values, memoize expensive calls, and swap in decorated bodies.

```c
#include "abscom/abs.h"
```

## API

| Function | Description |
| --- | --- |
| `var make_func(AbsFunc f)` | Wrap `var f(var)` into an `ABS_FUNC` object. |
| `var call_func(var func_obj, var arg)` | Call a function object with one argument. |
| `var memoize(AbsFunc f)` | Wrap `f` with a result cache. |
| `var call_memoized(var func_obj, var arg)` | Call through the cache (memoized functions only). |
| `var decorate(var func_obj, AbsFunc wrapper_logic)` | New function whose body is `wrapper_logic`; the original is kept as metadata. |
| `var func_meta(var func_obj)` | The original function kept by `decorate`, or `None`. |

`call_func` rejects non-function objects with an `ABS_ERROR`, and automatically dispatches through the cache when one is present (so a memoized function can be called with either `call_func` or `call_memoized`).

```c
static int calls = 0;

static var heavy(var n) {
    calls++;
    return v(n->val.i * 2);
}

var f = memoize(heavy);
print(call_memoized(f, v(100)));   /* 200, body runs once */
print(call_memoized(f, v(100)));   /* 200, cached */
print(call_func(f, v(100)));       /* 200, cached too */
print(v("Body calls:"), v(calls)); /* 1 */
```

## Cache keys

The cache dictionary is keyed by the argument:

- `ABS_INT` → the integer value (e.g. `"100"`).
- `ABS_STR` → the string contents.
- Anything else → the object's address (`"ptr_0x..."`).

So identical ints and strings share cache entries across calls.

## Decorators

`decorate(original, wrapper)` returns a **new** function object whose body is `wrapper`. The original function is preserved in the new function's metadata and is reachable via `func_meta`:

```c
static var shout(var s) { (void)s; return v("SHOUT"); }

var plain = make_func(heavy);
var loud = decorate(plain, shout);

print(call_func(loud, v(1)));      /* SHOUT */
print(func_meta(loud) == plain);   /* True: original is kept */
print(func_meta(plain));           /* None: plain functions have no metadata */
```

See `tests/test_func.c` for the full test.
