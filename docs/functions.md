# Functions, Decorators, and Memoization

First-class function objects (`ABS_FUNC`) let you wrap C functions as `var` values, memoize expensive calls, and swap in decorated bodies.

```c
#include "abscom/abs.h"
```

## API

| Function | Description |
| --- | --- |
| `var make_func(AbsFunc f)` | Wrap `var f(var)` into an `ABS_FUNC` object. |
| `var def(AbsFunc f, const char *name)` | `make_func` plus a display name (shown by `print`/`str`). |
| `var call_func(var func_obj, var arg)` | Call a function object with one argument. |
| `var memoize(AbsFunc f)` | Wrap `f` with a result cache. |
| `var call_memoized(var func_obj, var arg)` | Call through the cache (memoized functions only). |
| `var decorate(var func_obj, AbsFunc wrapper_logic)` | New function whose body is `wrapper_logic`; the original is kept as metadata. |
| `var decorate_func(var target, AbsWrapperFunc wrapper)` | Target-aware decorator: the wrapper receives `(target, args)` and calls the original back via `call_func`. |
| `var func_meta(var func_obj)` | The original function kept by `decorate`/`decorate_func`, or `None`. |
| `var func_name(var func_obj)` | The display name set by `def`, or `None`. |

`call_func` rejects non-function objects with an `ABS_ERROR`, and automatically dispatches through the cache when one is present (so a memoized function can be called with either `call_func` or `call_memoized`). A target-aware decorator (`decorate_func`) is also dispatched transparently by `call_func`.

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

### Swap-body decorator

`decorate(original, wrapper)` returns a **new** function object whose body is `wrapper`. The original function is preserved in the new function's metadata and is reachable via `func_meta`:

```c
static var shout(var s) { (void)s; return v("SHOUT"); }

var plain = make_func(heavy);
var loud = decorate(plain, shout);

print(call_func(loud, v(1)));      /* SHOUT */
print(func_meta(loud) == plain);   /* True: original is kept */
print(func_meta(plain));           /* None: plain functions have no metadata */
```

### Target-aware decorator (Python `@decorator` style)

`decorate_func(target, wrapper)` is the wrapper pattern from the paste-style API: the wrapper receives **both** the original target and the call argument, so it can run pre/post logic around the original and decide whether to call it at all.

```c
static var slow_square(var x) { return v(x->val.i * x->val.i); }

static var timing_wrapper(var target, var args) {
    clock_t start = clock();
    var result = call_func(target, args);   /* run the original */
    clock_t end = clock();
    printf("[timing] %.4f sec\n",
           (double)(end - start) / CLOCKS_PER_SEC);
    return result;
}

var sq = def(slow_square, "square");        /* named function */
var timed = decorate_func(sq, timing_wrapper);

print(call_func(timed, v(9)));              /* [timing] ... then 81 */
print(func_name(timed));                    /* "square" (inherited) */
print(func_meta(timed) == sq);              /* True */
```

Notes:

- `def(f, name)` is just `make_func(f)` plus a display name; `print` and `str` render it as `<function name>`, and `func_name(f)` reads it back.
- Inside a target-aware wrapper, call the original with `call_func(target, args)`; the wrapper can return early and never call the target (guards, caching, short-circuiting).
- `call_func` dispatches plain functions, memoized functions (via their cache), and target-aware decorators transparently, so callers never need to know which kind they hold.

See `tests/test_func.c` for the full test.
