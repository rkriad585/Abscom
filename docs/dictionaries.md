# Dictionaries

Dictionaries map **C-string keys** to arbitrary `var` values, backed by a hash table with chaining.

```c
#include "abscom/abs.h"
```

## Operations

| Function | Description |
| --- | --- |
| `var Dict()` | An empty dict. |
| `void dset(var dict, const char *key, var val)` | Insert `key -> val`. |
| `var dget(var dict, const char *key)` | Look up `key`; returns `None` if missing, an error if `dict` is not a dict. |
| `var len(var dict)` | Number of keys. |

Keys are plain C strings (`const char *`), copied on insert. Duplicate keys overwrite the previous value.

## Example

```c
abs_init();

var d = Dict();
dset(d, "name", abs_new_str("Ada"));
dset(d, "age",  v(36));

print(dget(d, "name"));   /* Ada */
print(dget(d, "age"));    /* 36  */
print(len(d));            /* 2   */
```

## Missing keys

`dget` on a missing key returns `None`, not an error:

```c
var d = Dict();
dset(d, "a", v(1));
dget(d, "b");             /* None */
```

## Wrong types

`dget` on a non-dict returns an `ABS_ERROR` value; `dset` on a non-dict is a silent no-op.

## Strings as keys

Since keys are C strings, pass literals directly — no `abs_new_str` needed for the key:

```c
dset(d, "score", v(42));
```

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
