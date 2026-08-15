# Usage

This page collects practical `ac_py` recipes. Every example is verified against the library's tests (`tests/test_py.c`) or the bundled examples.

## Table of Contents

- [Literals and constructors](#literals-and-constructors)
- [Lists and ranges](#lists-and-ranges)
- [Dictionaries](#dictionaries)
- [Sets](#sets)
- [Strings](#strings)
- [Formatting](#formatting)
- [JSON](#json)
- [Functional helpers](#functional-helpers)
- [Random utilities](#random-utilities)
- [Aggregates and math](#aggregates-and-math)
- [Sequences](#sequences)
- [OOP-lite](#oop-lite)
- [Files and system](#files-and-system)

## Literals and constructors

```c
var x   = v(42);        /* ABS_INT  */
var y   = v(3.14);      /* ABS_FLOAT */
var s   = v("hi");      /* ABS_STR  */
var b   = v(True);      /* ABS_BOOL */
var n   = None;         /* ABS_NONE */
var l   = List();       /* ABS_LIST */
var d   = Dict();       /* ABS_DICT */
var set = Set();        /* ABS_SET  */
```

`v()` is a `_Generic` macro and returns objects of type `var` (`AbsObj *`). `None`, `True`, and `False` are macros; `List()`, `Dict()`, and `Set()` construct empty containers.

## Lists and ranges

```c
var nums = List();
for (int i = 0; i < 5; i++) append(nums, v(i));

get(nums, 0);            /* first element */
get(nums, -1);           /* last element (negative indexing) */
slice(nums, 1, 3);       /* elements 1..2 */
len(nums);               /* 5 */

range(0, 5);             /* [0, 1, 2, 3, 4] */
range_step(10, 0, -3);   /* [10, 7, 4, 1] */
```

Iterate without manual indexing using the `foreach` macro:

```c
var item;
long total = 0;
foreach (item, nums) total += item->val.i;
```

## Dictionaries

```c
var user = Dict();
dset(user, "name", v("John Doe"));
dset(user, "age", v(30));

dget(user, "name");      /* "John Doe" */
dget(user, "missing");   /* None (ABS_NONE) */
len(user);               /* 2 */
```

`dset` overwrites existing keys; `dget` on a missing key returns `None`. Passing a non-dict to `dget` returns an error object.

## Sets

```c
var s = Set();
set_add(s, v(3));
set_add(s, v(1));
set_add(s, v(3));        /* duplicate: ignored */

set_contains(s, v(1));   /* true  */
set_contains(s, v(9));   /* false */

set_union(s, other);     /* union set  */
set_diff(s, other);      /* difference */
```

Sets deduplicate by type and value; integers, floats, strings, booleans, `None`, and nested lists/sets are compared element-wise.

## Strings

```c
split(v("apple,banana,grape"), ",");   /* ["apple", "banana", "grape"] */
join(v(" | "), fruits);                /* "apple | banana | grape"      */
strip(v("   Abscom   "));              /* "Abscom"                      */
upper(v("hello"));                     /* "HELLO"                       */
lower(v("WORLD"));                     /* "world"                       */
startswith(v("script.py"), v(".py"));  /* false                         */
endswith(v("script.py"), v(".py"));    /* true                          */
count(v("banana"), v("an"));           /* 2                             */
```

## Formatting

`fmt` substitutes `{}` placeholders using each argument's string representation:

```c
var msg = fmt("Hello {}, your ID is {}", v("Bob"), v(42));
/* "Hello Bob, your ID is 42" */
```

## JSON

```c
var data = json_parse("{\"id\": 101, \"scores\": [10, 20, 30]}");
dget(data, "id");        /* 101 */

json_dump(data);         /* {"id": 101, "scores": [10, 20, 30]} */
json_dump(v(3.5));       /* "3.5"  */
json_dump(v(True));      /* "true" */
json_dump(None);         /* "null" */
```

`json_parse` handles objects, arrays, numbers, booleans, `null`, and nested values, and returns an `ABS_ERROR` object for malformed input. `json_dump` escapes strings and emits `true`/`false`/`null` for booleans and `None`.

## Functional helpers

```c
var doubled = map_func(nums, double_it);        /* map */
var evens   = filter_func(nums, is_even);       /* filter (truthy callback result) */
var squares = list_comp(nums, square_it, NULL); /* map + filter in one pass */
```

`list_comp(list, mapf, filterf)` accepts `AbsMapFunc` for mapping and `AbsFilterFunc` (returning `bool`) for filtering. Either callback may be `NULL`.

## Random utilities

```c
seed(v("AbscomRulez"));    /* seed with a string or int; random_seed() reseeds from time */
randint(1, 100);           /* random integer in [1, 100] */
random_float();            /* float in [0, 1) */
uniform(10.5, 20.5);       /* float in [10.5, 20.5] */
choice(deck);              /* random element of a list/string */
choices(deck, 3);          /* 3 picks with replacement */
sample(deck, 5);           /* 5 picks without replacement */
shuffle(deck);             /* shuffle a list in place */
```

`choice` on an empty sequence and `sample` with a negative or over-sized `k` return an `ABS_ERROR` object.

## Aggregates and math

```c
min_val(nums);             /* smallest element */
max_val(nums);             /* largest element  */
sum_val(nums);             /* numeric sum      */
abs_val(v(-3.5));          /* 3.5  */
pow_val(v(2), v(8));       /* 256.0 */
round_val(v(3.14159), 2);  /* 3.14 */
```

## Sequences

```c
sorted(nums, false);       /* ascending copy  */
sorted(nums, true);        /* descending copy */
reversed_seq(nums);        /* reversed copy   */
zip_lists(list1, list2);   /* list of pairs   */
```

`sorted` and `reversed_seq` return new lists and do not modify the input.

## OOP-lite

```c
var Dog = Class("Dog");
var rex = New(Dog);
set_attr(rex, "name", v("Rex"));
set_attr(rex, "age", v(3));

get_attr(rex, "name");     /* "Rex" */
get_attr(rex, "breed");    /* None (missing attribute) */

str(rex);                  /* "<Dog object>" */
```

## Files and system

```c
var f = fopen_safe("out.txt", "w");
write_file(f, v("hello\n"));
close_file(f);

var rf = fopen_safe("out.txt", "r");
var contents = read_file(rf);
close_file(rf);

sleep_sec(1.0);            /* sleep 1 second   */
time_now();                /* Unix timestamp   */
exec_cmd("echo hi");       /* system() exit code */
```

`fopen_safe` returns an `ABS_ERROR` object when a file cannot be opened; check it with `is_err()`.

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
