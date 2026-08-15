#include "abscom/abs.h"

#include <stdio.h>
#include <stdlib.h>

/* Visualization hook: called on every swap by sort_bubble_visual(). */
static void on_swap(var list, int idx_a, int idx_b) {
    printf("  [SWAP %d <-> %d] First 5: ", idx_a, idx_b);
    size_t n = list->val.list.size > 5 ? 5 : list->val.list.size;
    for (size_t i = 0; i < n; i++)
        printf("%ld ", list->val.list.items[i]->val.i);
    printf("...\n");
    sleep_sec(0.1);
}

int main(void) {
    abs_init();

    /* 1. GENERATE DATA */
    print(v("Generating 2000 random integers..."));
    var data = List();
    for (int i = 0; i < 2000; i++) append(data, v(rand() % 10000));

    /* 2. BENCHMARKING */
    print(v("\n--- Benchmarking (seconds) ---"));
    print(v("Bubble Sort (O(n^2)):  "), v(timeit(sort_bubble, data)), v("sec"));
    print(v("Insertion Sort:        "), v(timeit(sort_insertion, data)), v("sec"));
    print(v("Shell Sort:            "), v(timeit(sort_shell, data)), v("sec"));
    print(v("Merge Sort (O(nlogn)): "), v(timeit(sort_merge, data)), v("sec"));
    print(v("Radix Sort (O(nk)):    "), v(timeit(sort_radix, data)), v("sec"));
    print(v("C Stdlib Qsort:        "), v(timeit(sort_c_qsort, data)), v("sec"));

    /* 3. VISUALIZER */
    print(v("\n--- Visualizer (Bubble Sort on a Small List) ---"));
    var small = List();
    append(small, v(50)); append(small, v(10)); append(small, v(40));
    append(small, v(20)); append(small, v(30));
    print(v("Before:"), small);
    sort_bubble_visual(small, on_swap);
    print(v("After: "), small);

    /* 4. RANDOMIZER (Bogo Sort) */
    print(v("\n--- Bogo Sort (The Worst Sort) ---"));
    var tiny = List();
    append(tiny, v(3)); append(tiny, v(1)); append(tiny, v(2));
    print(v("Sorting [3, 1, 2]..."));
    sort_bogo(tiny);
    print(v("Success:"), tiny);

    /* 5. SEARCH */
    print(v("\n--- Binary Search ---"));
    sort_quick(data);
    var target = v(500);
    long idx = binary_search(data, target);
    if (idx != -1)
        print(v("Found 500 at index:"), v(idx));
    else
        print(v("500 not found in random data."));

    abs_cleanup();
    return 0;
}
