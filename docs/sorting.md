# Sorting, Search, and Benchmarking

The Algorithm Suite turns Abscom into a small Computer Science teaching kit: twelve sorting algorithms from `O(n^2)` to `O(n log n)` and `O(n)`, a swap-hook visualizer, a `timeit` benchmark helper, and binary search. All comparisons go through `compare_objs()`, so a single sort works on ints, floats, and strings alike (mixed numerics are compared as doubles).

```c
#include "abscom/abs.h"
```

## Sorting

| Function | Complexity | Notes |
| --- | --- | --- |
| `void sort_bubble(var list)` | O(n²) | Repeatedly swap adjacent out-of-order pairs. |
| `void sort_selection(var list)` | O(n²) | Pick the smallest remaining element each pass. |
| `void sort_insertion(var list)` | O(n²) | Insert each element into the sorted prefix. |
| `void sort_shell(var list)` | O(n log n) avg | Insertion sort over shrinking gaps. |
| `void sort_heap(var list)` | O(n log n) | Heapify, then extract the max. |
| `void sort_merge(var list)` | O(n log n) | Stable top-down divide-and-conquer merge. |
| `void sort_quick(var list)` | O(n log n) avg | Hoare/Lomuto partition, last-element pivot. |
| `void sort_counting(var list)` | O(n + k) | Integers only; works with negative values. |
| `void sort_radix(var list)` | O(nk) | LSD radix on **non-negative** integers. |
| `void sort_bucket(var list)` | O(n + k) | Delegates to counting sort. |
| `void sort_bogo(var list)` | unbounded | Joke sort — shuffles until sorted. Only for tiny lists. |
| `void sort_c_qsort(var list)` | O(n log n) | Thin wrapper over the C library's `qsort`. |

```c
var data = List();
append(data, v(5)); append(data, v(1)); append(data, v(4));
sort_quick(data);
print(data);                      /* [1, 4, 5] */

var words = List();
append(words, v("banana")); append(words, v("apple"));
sort_c_qsort(words);
print(words);                     /* [apple, banana] */
```

## Visualization hooks

`sort_bubble_visual` runs bubble sort and calls an `AbsSortVis` callback after every swap. Wire it to a logger, a GUI, or an animation:

```c
typedef void (*AbsSortVis)(var list, int idx_a, int idx_b);

static void on_swap(var list, int idx_a, int idx_b) {
    (void)idx_a; (void)idx_b;
    printf("step: "); print(list);
}

sort_bubble_visual(data, on_swap);
```

The callback receives the list and the two swapped indices after the swap, so you can render the new state.

## Benchmarking

`timeit` sorts a **copy** of the list (primitive elements are deep-copied) and returns the wall-clock time in seconds, leaving the original untouched:

```c
double t = timeit(sort_bubble, data);      /* seconds */
printf("bubble took %.3fs\n", t);
```

Related helpers:

- `var list_copy(var list)` — copies the list container and its primitive elements; nested containers are shared.
- `bool is_sorted(var list)` — true when every adjacent pair is in non-decreasing order (an empty list is vacuously sorted).

## Binary search

`binary_search` requires a sorted list and returns the index of the target, or `-1`:

```c
long idx = binary_search(data, v(500));
print(idx);                                /* index or -1 */
```

Notes:

- `timeit(NULL, list)` and `timeit(f, non_list)` return `-1.0`; `binary_search` on an empty or non-list object returns `-1`.
- Counting, radix, and bucket sorts read values as integers; pass them integer lists only.
- Bogo sort's expected runtime is factorial — keep it to lists of 8 or fewer elements.

See `tests/test_sort.c` for the full test, and `examples/sort_demo.c` for a benchmark + visualizer showcase.
