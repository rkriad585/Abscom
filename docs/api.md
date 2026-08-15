# API Reference

Abscom exposes two public headers. This reference mirrors the declarations in the repository; where behavior is documented, it matches the implementation in `src/`.

- [Core library (`ac.h`)](#core-library-ach)
  - [Common macros (`ac_common.h`)](#common-macros-ac_commonh)
  - [Dynamic array (`ac_dynarray.h`)](#dynamic-array-ac_dynarrayh)
  - [String buffer (`ac_string.h`)](#string-buffer-ac_stringh)
  - [Hashing (`ac_hash.h`)](#hashing-ac_hashh)
  - [Hash map (`ac_hashmap.h`)](#hash-map-ac_hashmaph)
  - [Time (`ac_time.h`)](#time-ac_timeh)
  - [File I/O (`ac_fs.h`)](#file-io-ac_fsh)
- [Python-like runtime (`ac_py.h`)](#python-like-runtime-ac_pyh)
  - [Types and objects](#types-and-objects)
  - [Lifecycle and constructors](#lifecycle-and-constructors)
  - [Literals and macros](#literals-and-macros)
  - [I/O and printing](#io-and-printing)
  - [Arithmetic](#arithmetic)
  - [Conversions](#conversions)
  - [Dictionaries](#dictionaries)
  - [Lists, ranges, slices](#lists-ranges-slices)
  - [Strings](#strings)
  - [Functional helpers](#functional-helpers)
  - [Formatting](#formatting)
  - [Files](#files)
  - [JSON](#json)
  - [Random](#random)
  - [Type introspection](#type-introspection)
  - [Aggregates and math](#aggregates-and-math)
  - [Sequences](#sequences)
  - [System](#system)
  - [OOP-lite](#oop-lite)
  - [Sets](#sets)

## Core library (`ac.h`)

`#include <abscom/ac.h>` includes all core modules. Core functions return `0` on success and non-zero on failure unless noted.

### Common macros (`ac_common.h`)

| Declaration | Purpose |
| --- | --- |
| `AC_API` | Export/import annotation. `__declspec(dllexport)` when building the library (`AC_BUILDING_LIBRARY`), `__declspec(dllimport)` for consumers (`AC_USE_LIBRARY`) on Windows; `__attribute__((visibility("default")))` on GCC/Clang. |
| `AC_BEGIN_C_DECLS` / `AC_END_C_DECLS` | Wraps declarations in `extern "C"` when compiled as C++. |
| `AC_UNUSED(x)` | Suppresses unused-variable warnings. |

### Dynamic array (`ac_dynarray.h`)

```c
typedef struct ac_dynarray {
    void  *data;
    size_t elem_size;
    size_t len;
    size_t cap;
} ac_dynarray_t;
```

| Function | Description |
| --- | --- |
| `int ac_dynarray_init(ac_dynarray_t *arr, size_t elem_size)` | Initialize an array with a fixed element size. |
| `void ac_dynarray_destroy(ac_dynarray_t *arr)` | Free the backing buffer. |
| `void ac_dynarray_clear(ac_dynarray_t *arr)` | Set length to zero (keeps capacity). |
| `int ac_dynarray_reserve(ac_dynarray_t *arr, size_t cap)` | Ensure capacity for `cap` elements. |
| `int ac_dynarray_push(ac_dynarray_t *arr, const void *elem)` | Append a copy of `elem`. |
| `void ac_dynarray_pop(ac_dynarray_t *arr)` | Remove the last element. |
| `int ac_dynarray_resize(ac_dynarray_t *arr, size_t new_len)` | Resize; new slots are zero-filled. |
| `void *ac_dynarray_at(ac_dynarray_t *arr, size_t index)` | Pointer to element, or `NULL` if out of range. |
| `const void *ac_dynarray_at_const(const ac_dynarray_t *arr, size_t index)` | Const variant of `ac_dynarray_at`. |
| `void *ac_dynarray_data(ac_dynarray_t *arr)` | Raw backing buffer. |
| `size_t ac_dynarray_len(const ac_dynarray_t *arr)` | Number of elements. |
| `size_t ac_dynarray_cap(const ac_dynarray_t *arr)` | Allocated capacity. |

### String buffer (`ac_string.h`)

```c
typedef struct ac_string {
    char  *data;
    size_t len;
    size_t cap;
} ac_string_t;
```

The buffer is always NUL-terminated; `data` may be `NULL` for an empty string.

| Function | Description |
| --- | --- |
| `int ac_string_init(ac_string_t *s)` | Initialize an empty string. |
| `int ac_string_init_cstr(ac_string_t *s, const char *str)` | Initialize from a C string. |
| `int ac_string_init_n(ac_string_t *s, const char *str, size_t n)` | Initialize from `n` bytes. |
| `void ac_string_destroy(ac_string_t *s)` | Free the buffer. |
| `void ac_string_clear(ac_string_t *s)` | Empty the string (keeps capacity). |
| `int ac_string_reserve(ac_string_t *s, size_t cap)` | Reserve capacity. |
| `int ac_string_shrink_to_fit(ac_string_t *s)` | Trim capacity to the current length. |
| `int ac_string_append_cstr(ac_string_t *s, const char *str)` | Append a C string. |
| `int ac_string_append_n(ac_string_t *s, const char *str, size_t n)` | Append `n` bytes. |
| `int ac_string_append_char(ac_string_t *s, char c)` | Append one character. |
| `int ac_string_append_fmt(ac_string_t *s, const char *fmt, ...)` | Append `printf`-style formatted output. |
| `int ac_string_set_cstr(ac_string_t *s, const char *str)` | Replace contents. |
| `const char *ac_string_c_str(const ac_string_t *s)` | NUL-terminated contents (never `NULL`). |
| `size_t ac_string_len(const ac_string_t *s)` | Current length. |
| `char *ac_string_take(ac_string_t *s)` | Transfer ownership of the buffer to the caller. |

### Hashing (`ac_hash.h`)

| Function | Description |
| --- | --- |
| `uint32_t ac_hash_fnv1a32(const void *data, size_t len)` | FNV-1a 32-bit over `len` bytes. |
| `uint64_t ac_hash_fnv1a64(const void *data, size_t len)` | FNV-1a 64-bit over `len` bytes. |
| `uint64_t ac_hash_fnv1a64_str(const char *str)` | FNV-1a 64-bit of a NUL-terminated string. |
| `uint32_t ac_hash_djb2(const char *str)` | djb2 32-bit of a NUL-terminated string. |

### Hash map (`ac_hashmap.h`)

Open-addressing, string-keyed map using FNV-1a 64 hashing with linear probing, tombstone deletion, and automatic resizing.

```c
typedef struct ac_hashmap ac_hashmap_t;
typedef void (*ac_hashmap_free_fn)(void *value);
typedef int  (*ac_hashmap_visit_fn)(const char *key, void *value, void *user);
```

| Function | Description |
| --- | --- |
| `ac_hashmap_t *ac_hashmap_create(ac_hashmap_free_fn free_value)` | Create a map; `free_value` (optional) is called on stored values when removed/cleared/destroyed. |
| `void ac_hashmap_destroy(ac_hashmap_t *m)` | Free everything. |
| `void ac_hashmap_clear(ac_hashmap_t *m)` | Remove all entries. |
| `int ac_hashmap_set(ac_hashmap_t *m, const char *key, void *value)` | Insert or replace a key. |
| `void *ac_hashmap_get(const ac_hashmap_t *m, const char *key)` | Value for `key`, or `NULL`. |
| `int ac_hashmap_contains(const ac_hashmap_t *m, const char *key)` | 1 if present, 0 otherwise. |
| `int ac_hashmap_remove(ac_hashmap_t *m, const char *key)` | Remove a key; 0 on success, -1 if missing. |
| `size_t ac_hashmap_size(const ac_hashmap_t *m)` | Number of entries. |
| `void ac_hashmap_foreach(ac_hashmap_t *m, ac_hashmap_visit_fn fn, void *user)` | Visit each entry; stops early if `fn` returns non-zero. |

### Time (`ac_time.h`)

| Function | Description |
| --- | --- |
| `double ac_time_now(void)` | Monotonic time in seconds (QPC on Windows, `CLOCK_MONOTONIC` on POSIX). |
| `double ac_time_wall(void)` | Wall-clock time in seconds since the Unix epoch (`FILETIME`/`CLOCK_REALTIME`). |
| `uint64_t ac_time_now_ns(void)` | Monotonic time in nanoseconds. |
| `uint64_t ac_time_wall_ms(void)` | Wall-clock time in milliseconds. |

### File I/O (`ac_fs.h`)

| Function | Description |
| --- | --- |
| `int ac_fs_exists(const char *path)` | 1 if the file can be opened for reading, 0 otherwise. |
| `int ac_fs_read_file(const char *path, char **out_data, size_t *out_size)` | Read whole file into a NUL-terminated buffer (caller frees). |
| `int ac_fs_write_file(const char *path, const void *data, size_t size)` | Write bytes to a file. |
| `int ac_fs_remove(const char *path)` | Delete a file. |
| `int ac_fs_rename(const char *from, const char *to)` | Rename a file. |

## Python-like runtime (`ac_py.h`)

`#include <abscom/ac_py.h>` requires the full library (it uses `ac_string` internally).

### Types and objects

```c
typedef enum {
    ABS_INT, ABS_FLOAT, ABS_STR, ABS_BOOL,
    ABS_LIST, ABS_DICT, ABS_FILE, ABS_ERROR,
    ABS_NONE, ABS_SET, ABS_CLASS, ABS_INSTANCE
} AbsType;

typedef struct AbsObj AbsObj;
typedef AbsObj *var;
```

`AbsObj` stores a type tag plus a union of the value (int `i`, float `f`, string `s`, bool `b`, list/set storage, dict buckets, class/instance storage, `FILE *`, or error message). `var` is a shorthand for `AbsObj *`.

Callbacks:

```c
typedef var  (*AbsFunc)(var);         /* map/filter style callback   */
typedef var  (*AbsMapFunc)(var);      /* list_comp map callback      */
typedef bool (*AbsFilterFunc)(var);   /* list_comp filter callback   */
```

### Lifecycle and constructors

| Function | Description |
| --- | --- |
| `void abs_init(void)` | Initialize the runtime: seed the RNG, reset the memory pool, and start Winsock on Windows. |
| `void abs_cleanup(void)` | Free all runtime memory (pool blocks and dynamic objects) and stop Winsock. |
| `void del(var obj)` | Mark an object deleted (frees its internals and sets its type to `ABS_NONE`). |
| `var abs_new_int(long v)` | New `ABS_INT`. |
| `var abs_new_float(double v)` | New `ABS_FLOAT`. |
| `var abs_new_str(const char *v)` | New `ABS_STR` (duplicates the string). |
| `var abs_new_bool(bool v)` | New `ABS_BOOL`. |
| `var abs_new_none(void)` | New `ABS_NONE`. |
| `var abs_new_list(void)` | New empty `ABS_LIST`. |
| `var abs_new_dict(void)` | New empty `ABS_DICT`. |
| `var abs_new_set(void)` | New empty `ABS_SET`. |
| `var abs_new_error(const char *msg)` | New `ABS_ERROR` with a message. |

### Literals and macros

| Macro | Expands to |
| --- | --- |
| `v(X)` | `_Generic` literal: `int`/`long` → int, `float`/`double` → float, `char *`/`const char *` → string, `bool` → bool. |
| `None` | `abs_new_none()` |
| `True` / `False` | `((bool)1)` / `((bool)0)` |
| `List()` / `Dict()` / `Set()` | `abs_new_list()` / `abs_new_dict()` / `abs_new_set()` |
| `print(...)` | `abs_print_impl("\n", __VA_ARGS__, (var)0)` — prints args with spaces, then a newline. |
| `print_end(e, ...)` | Same, but uses `e` as the terminator. |
| `fmt(f, ...)` | `fmt_impl(f, __VA_ARGS__, (var)0)` — `{}` substitution. |
| `str(o)` | `to_str(o)` |
| `foreach(VAR, LIST)` | Iterates `LIST` (list or set) assigning each element to `VAR`. |

### I/O and printing

| Function | Description |
| --- | --- |
| `void abs_print_impl(const char *end, ...)` | Print each `var` argument (until the `(var)0` sentinel) with spaces; terminates with `end`. |
| `var input(const char *prompt)` | Print `prompt` and read one line (newline stripped). |

`print_single` formatting rules (used by `print` and `to_str`): ints print as `%ld`, floats as `%.2f`, strings raw, booleans as `True`/`False`, `None` as `None`, lists as `[a, b]`, dicts as `{key: value}`, sets as `{a, b}`, classes as `<class 'name'>`, instances as `<Name object>`, files as `<file>`, errors as `Error: <msg>`.

### Arithmetic

| Function | Description |
| --- | --- |
| `var add(var a, var b)` | Numbers add; strings concatenate; lists concatenate; otherwise `None`. |
| `var sub(var a, var b)` | Number subtraction; otherwise `None`. |
| `var mul(var a, var b)` | Numbers multiply; `str * int` repeats the string; otherwise `None`. |
| `var eq(var a, var b)` | Equality as a `ABS_BOOL`: numbers compare numerically, lists compare element-wise. |

### Conversions

| Function | Description |
| --- | --- |
| `var to_str(var obj)` | String representation (identity for strings). |
| `var to_int(var obj)` | Int conversion for int/float/bool/parsable string; otherwise `None`. |
| `var to_float(var obj)` | Float conversion for int/float/bool/parsable string; otherwise `None`. |

### Dictionaries

| Function | Description |
| --- | --- |
| `void dset(var dict, const char *key, var val)` | Insert or replace `key` (djb2 bucket hashing). |
| `var dget(var dict, const char *key)` | Value for `key`, `None` if missing, or an `ABS_ERROR` for non-dicts. |

### Lists, ranges, slices

| Function | Description |
| --- | --- |
| `void append(var list, var item)` | Append to a list (doubling growth). |
| `var get(var obj, long index)` | Element at `index` (negative wraps from the end); `ABS_ERROR` if out of range or not a list/set. |
| `var len(var obj)` | Length as an int (string, list, set, or dict). |
| `long get_len_fast(var obj)` | Fast raw length for lists/sets, else `0`. |
| `var range(int start, int stop)` | List of ints `[start, stop)`. |
| `var range_step(int start, int stop, int step)` | List of ints with a step (`0` step yields an empty list). |
| `var slice(var obj, int start, int stop)` | List slice with negative-index support. |

### Strings

| Function | Description |
| --- | --- |
| `var upper(var obj)` / `var lower(var obj)` | Case conversion of a string; `None` otherwise. |
| `var split(var str_obj, const char *delimiter)` | Split into a list; empty delimiter splits per character. |
| `var join(var delimiter, var list_obj)` | Join list items with a delimiter string. |
| `var strip(var str_obj)` | Trim leading/trailing whitespace. |
| `var startswith(var str, var prefix)` / `var endswith(var str, var suffix)` | Prefix/suffix checks as `ABS_BOOL`. |
| `var count(var container, var item)` | Occurrences in a list (element compare) or string (substring). |

### Functional helpers

| Function | Description |
| --- | --- |
| `var map_func(var list, AbsFunc f)` | New list with `f` applied to each element. |
| `var filter_func(var list, AbsFunc f)` | New list with elements for which `is_true(f(item))`. |
| `var list_comp(var list, AbsMapFunc mapf, AbsFilterFunc filterf)` | Map and/or filter in one pass; either callback may be `NULL`. |

### Formatting

| Function | Description |
| --- | --- |
| `var fmt_impl(const char *format, ...)` | Replaces each `{}` with the next `var` argument's string form. Use the `fmt(...)` macro. |

### Files

| Function | Description |
| --- | --- |
| `var fopen_safe(const char *filename, const char *mode)` | Wraps `fopen` in an `ABS_FILE` object; `ABS_ERROR` on failure. |
| `var read_file(var file_obj)` | Read the whole file as a string; `ABS_ERROR` for non-files. |
| `void write_file(var file_obj, var content)` | Write a value (stringified) to the file. |
| `void close_file(var file_obj)` | Close the underlying `FILE *`. |

### JSON

| Function | Description |
| --- | --- |
| `var json_parse(const char *json_str)` | Parse JSON into objects/lists/numbers/booleans/`None`; `ABS_ERROR` on invalid input. |
| `var json_dump(var obj)` | Serialize an object to a JSON string (escapes strings; emits `true`/`false`/`null`). |

### Random

| Function | Description |
| --- | --- |
| `void random_seed(void)` | Reseed from the current time. |
| `void seed(var obj)` | Seed from an int, a string hash, or the current time. |
| `var randint(int min, int max)` | Int in `[min, max]` (swaps bounds if needed). |
| `var random_float(void)` | Float in `[0, 1)`. |
| `var uniform(double a, double b)` | Float in `[a, b]`. |
| `var choice(var seq)` | Random element of a list or character of a string. |
| `var choices(var seq, int k)` | `k` picks with replacement. |
| `var sample(var seq, int k)` | `k` picks without replacement (lists only). |
| `void shuffle(var list)` | In-place Fisher-Yates shuffle. |

### Type introspection

| Function | Description |
| --- | --- |
| `var type(var obj)` | String like `"<class 'int'>"`, `"<class 'list'>"`, `"<class 'set'>"`, `"<class 'error'>"`, etc. |
| `bool is_err(var obj)` | True for `ABS_ERROR`. |
| `bool is_int/is_float/is_str/is_list/is_dict/is_set/is_none(var o)` | Type predicates (null-safe; `is_none(NULL)` is true). |
| `bool is_true(var obj)` | Truthiness: zero/false/empty/`None`/error are false. |

### Aggregates and math

| Function | Description |
| --- | --- |
| `var min_val(var list)` / `var max_val(var list)` | Min/max element (empty list → `None`). |
| `var sum_val(var list)` | Numeric sum (int/float aware). |
| `var not_(var obj)` | Logical negation as `ABS_BOOL`. |
| `var any(var list)` / `var all(var list)` | Boolean aggregators. |
| `var abs_val(var obj)` | Absolute value of int/float. |
| `var pow_val(var base, var exp)` | `pow(base, exp)` as a float. |
| `var round_val(var obj, int digits)` | Rounded value (`digits == 0` returns an int). |
| `int compare_objs(var a, var b)` | `-1`/`0`/`1` comparison for numbers and strings. |

### Sequences

| Function | Description |
| --- | --- |
| `var sorted(var list, bool reverse)` | Sorted copy (bubble sort on `compare_objs`). |
| `var reversed_seq(var list)` | Reversed copy. |
| `var zip_lists(var list1, var list2)` | List of 2-element pair lists, truncated to the shorter input. |

### System

| Function | Description |
| --- | --- |
| `void sleep_sec(double seconds)` | Sleep (Windows `Sleep`, POSIX `usleep`). |
| `var time_now(void)` | Current Unix timestamp as a float. |
| `var exec_cmd(const char *cmd)` | Run a shell command via `system()`, returning its exit code as an int. |
| `var http_get(const char *url)` | HTTP/1.0 GET of `http://host/path`, returning the body as a string (or `ABS_ERROR`). |

`http_get` requires a network connection; on Windows it uses Winsock (initialized by `abs_init`). Only port 80 (`http://`) is used.

### OOP-lite

| Function | Description |
| --- | --- |
| `var Class(const char *name)` | Create an `ABS_CLASS` with a methods dictionary. |
| `var New(var cls)` | Create an `ABS_INSTANCE` of a class (`None` for non-classes). |
| `void set_attr(var obj, const char *key, var val)` | Set an instance attribute. |
| `var get_attr(var obj, const char *key)` | Get an instance attribute (`None` if missing). |

### Sets

| Function | Description |
| --- | --- |
| `void set_add(var set_obj, var item)` | Insert if not already present. |
| `bool set_contains(var set_obj, var item)` | Membership test (type-aware, element-wise for nested lists/sets). |
| `var set_union(var a, var b)` | New set with all elements of both. |
| `var set_diff(var a, var b)` | New set with elements of `a` not in `b`. |

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
