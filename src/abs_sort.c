/* Algorithm suite: sorting, benchmarking, and binary search.
 *
 *   sort_bubble / sort_selection / sort_insertion   O(n^2), educational
 *   sort_shell   / sort_heap     / sort_merge       O(n log n), efficient
 *   sort_quick   / sort_c_qsort                      divide & conquer
 *   sort_counting/ sort_radix   / sort_bucket        linear, integers only
 *   sort_bogo                                          joke sort (shuffles)
 *   sort_bubble_visual                                bubble sort with hooks
 *   is_sorted / list_copy / timeit / binary_search     helpers
 *
 * All comparisons go through compare_objs(), so ints, floats, and strings
 * sort uniformly (mixed numerics are compared as doubles).
 */

#include "abscom/abs.h"

#include <stdio.h>
#include <stdlib.h>

static void swap_items(var list, long i, long j) {
    var temp = list->val.list.items[i];
    list->val.list.items[i] = list->val.list.items[j];
    list->val.list.items[j] = temp;
}

/* --- O(n^2): educational sorts --- */

void sort_bubble(var list) {
    if (!list || list->type != ABS_LIST) return;
    long n = (long)list->val.list.size;
    for (long i = 0; i + 1 < n; i++)
        for (long j = 0; j + 1 < n - i; j++)
            if (compare_objs(list->val.list.items[j], list->val.list.items[j + 1]) > 0)
                swap_items(list, j, j + 1);
}

void sort_selection(var list) {
    if (!list || list->type != ABS_LIST) return;
    long n = (long)list->val.list.size;
    for (long i = 0; i + 1 < n; i++) {
        long min_idx = i;
        for (long j = i + 1; j < n; j++)
            if (compare_objs(list->val.list.items[j], list->val.list.items[min_idx]) < 0)
                min_idx = j;
        if (min_idx != i) swap_items(list, min_idx, i);
    }
}

void sort_insertion(var list) {
    if (!list || list->type != ABS_LIST) return;
    long n = (long)list->val.list.size;
    for (long i = 1; i < n; i++) {
        var key = list->val.list.items[i];
        long j = i - 1;
        while (j >= 0 && compare_objs(list->val.list.items[j], key) > 0) {
            list->val.list.items[j + 1] = list->val.list.items[j];
            j--;
        }
        list->val.list.items[j + 1] = key;
    }
}

/* --- O(n log n): efficient sorts --- */

void sort_shell(var list) {
    if (!list || list->type != ABS_LIST) return;
    long n = (long)list->val.list.size;
    for (long gap = n / 2; gap > 0; gap /= 2) {
        for (long i = gap; i < n; i++) {
            var temp = list->val.list.items[i];
            long j;
            for (j = i; j >= gap &&
                 compare_objs(list->val.list.items[j - gap], temp) > 0; j -= gap)
                list->val.list.items[j] = list->val.list.items[j - gap];
            list->val.list.items[j] = temp;
        }
    }
}

static void heapify(var list, long n, long i) {
    long largest = i;
    long left = 2 * i + 1;
    long right = 2 * i + 2;
    if (left < n && compare_objs(list->val.list.items[left], list->val.list.items[largest]) > 0)
        largest = left;
    if (right < n && compare_objs(list->val.list.items[right], list->val.list.items[largest]) > 0)
        largest = right;
    if (largest != i) {
        swap_items(list, i, largest);
        heapify(list, n, largest);
    }
}

void sort_heap(var list) {
    if (!list || list->type != ABS_LIST) return;
    long n = (long)list->val.list.size;
    for (long i = n / 2 - 1; i >= 0; i--) heapify(list, n, i);
    for (long i = n - 1; i > 0; i--) {
        swap_items(list, 0, i);
        heapify(list, i, 0);
    }
}

static void merge(var list, long l, long m, long r) {
    long n1 = m - l + 1;
    long n2 = r - m;
    var *L = (var *)malloc((size_t)n1 * sizeof(var));
    var *R = (var *)malloc((size_t)n2 * sizeof(var));
    if (!L || !R) {
        free(L);
        free(R);
        return;
    }
    for (long i = 0; i < n1; i++) L[i] = list->val.list.items[l + i];
    for (long j = 0; j < n2; j++) R[j] = list->val.list.items[m + 1 + j];
    long i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        if (compare_objs(L[i], R[j]) <= 0) list->val.list.items[k++] = L[i++];
        else list->val.list.items[k++] = R[j++];
    }
    while (i < n1) list->val.list.items[k++] = L[i++];
    while (j < n2) list->val.list.items[k++] = R[j++];
    free(L);
    free(R);
}

static void merge_sort_rec(var list, long l, long r) {
    if (l < r) {
        long m = l + (r - l) / 2;
        merge_sort_rec(list, l, m);
        merge_sort_rec(list, m + 1, r);
        merge(list, l, m, r);
    }
}

void sort_merge(var list) {
    if (!list || list->type != ABS_LIST || list->val.list.size < 2) return;
    merge_sort_rec(list, 0, (long)list->val.list.size - 1);
}

static long partition(var list, long low, long high) {
    var pivot = list->val.list.items[high];
    long i = low - 1;
    for (long j = low; j <= high - 1; j++) {
        if (compare_objs(list->val.list.items[j], pivot) < 0) {
            i++;
            swap_items(list, i, j);
        }
    }
    swap_items(list, i + 1, high);
    return i + 1;
}

static void quick_sort_rec(var list, long low, long high) {
    if (low < high) {
        long pi = partition(list, low, high);
        quick_sort_rec(list, low, pi - 1);
        quick_sort_rec(list, pi + 1, high);
    }
}

void sort_quick(var list) {
    if (!list || list->type != ABS_LIST || list->val.list.size < 2) return;
    quick_sort_rec(list, 0, (long)list->val.list.size - 1);
}

/* --- Linear sorts (integers only) --- */

void sort_counting(var list) {
    if (!list || list->type != ABS_LIST) return;
    long n = (long)list->val.list.size;
    if (n == 0) return;
    long min = list->val.list.items[0]->val.i;
    long max = min;
    for (long i = 1; i < n; i++) {
        long val = list->val.list.items[i]->val.i;
        if (val > max) max = val;
        if (val < min) min = val;
    }
    long range = max - min + 1;
    var *output = (var *)malloc((size_t)n * sizeof(var));
    int *count = (int *)calloc((size_t)range, sizeof(int));
    if (!output || !count) {
        free(output);
        free(count);
        return;
    }
    for (long i = 0; i < n; i++) count[list->val.list.items[i]->val.i - min]++;
    for (long i = 1; i < range; i++) count[i] += count[i - 1];
    for (long i = n - 1; i >= 0; i--) {
        long val = list->val.list.items[i]->val.i;
        output[count[val - min] - 1] = list->val.list.items[i];
        count[val - min]--;
    }
    for (long i = 0; i < n; i++) list->val.list.items[i] = output[i];
    free(output);
    free(count);
}

static void count_sort_exp(var list, long n, long exp) {
    var *output = (var *)malloc((size_t)n * sizeof(var));
    int count[10] = {0};
    if (!output) return;
    for (long i = 0; i < n; i++) count[(list->val.list.items[i]->val.i / exp) % 10]++;
    for (int i = 1; i < 10; i++) count[i] += count[i - 1];
    for (long i = n - 1; i >= 0; i--) {
        int idx = (int)((list->val.list.items[i]->val.i / exp) % 10);
        output[count[idx] - 1] = list->val.list.items[i];
        count[idx]--;
    }
    for (long i = 0; i < n; i++) list->val.list.items[i] = output[i];
    free(output);
}

void sort_radix(var list) {
    if (!list || list->type != ABS_LIST) return;
    long n = (long)list->val.list.size;
    long m = 0;
    for (long i = 0; i < n; i++) {
        if (list->val.list.items[i]->val.i > m) m = list->val.list.items[i]->val.i;
    }
    for (long exp = 1; m / exp > 0; exp *= 10) count_sort_exp(list, n, exp);
}

void sort_bucket(var list) {
    sort_counting(list);
}

/* --- Novelty sorts --- */

bool is_sorted(var list) {
    if (!list || list->type != ABS_LIST) return false;
    for (size_t i = 1; i < list->val.list.size; i++)
        if (compare_objs(list->val.list.items[i - 1], list->val.list.items[i]) > 0)
            return false;
    return true;
}

void sort_bogo(var list) {
    if (!list || list->type != ABS_LIST) return;
    while (!is_sorted(list)) shuffle(list);
}

static int qsort_cmp(const void *a, const void *b) {
    var va = *(const var *)a;
    var vb = *(const var *)b;
    return compare_objs(va, vb);
}

void sort_c_qsort(var list) {
    if (!list || list->type != ABS_LIST || list->val.list.size < 2) return;
    qsort(list->val.list.items, list->val.list.size, sizeof(var), qsort_cmp);
}

/* --- Visualization hook --- */

void sort_bubble_visual(var list, AbsSortVis vis_func) {
    if (!list || list->type != ABS_LIST) return;
    long n = (long)list->val.list.size;
    for (long i = 0; i + 1 < n; i++)
        for (long j = 0; j + 1 < n - i; j++)
            if (compare_objs(list->val.list.items[j], list->val.list.items[j + 1]) > 0) {
                swap_items(list, j, j + 1);
                if (vis_func) vis_func(list, (int)j, (int)(j + 1));
            }
}

/* --- Benchmarking --- */

var list_copy(var list) {
    if (!list || list->type != ABS_LIST) return abs_new_error("Object is not a list");
    var n = abs_new_list();
    if (!n) return None;
    for (size_t i = 0; i < list->val.list.size; i++) {
        var it = list->val.list.items[i];
        var copy;
        switch (it->type) {
            case ABS_INT:   copy = abs_new_int(it->val.i); break;
            case ABS_FLOAT: copy = abs_new_float(it->val.f); break;
            case ABS_STR:   copy = abs_new_str(it->val.s); break;
            case ABS_BOOL:  copy = abs_new_bool(it->val.b); break;
            default:        copy = it; /* share nested containers */
        }
        append(n, copy);
    }
    return n;
}

double timeit(void (*sort_func)(var), var list) {
    if (!sort_func) return -1.0;
    var cp = list_copy(list);
    if (!cp || is_err(cp)) return -1.0;
    clock_t start = clock();
    sort_func(cp);
    clock_t end = clock();
    return ((double)(end - start)) / CLOCKS_PER_SEC;
}

/* --- Search --- */

long binary_search(var sorted_list, var target) {
    if (!sorted_list || sorted_list->type != ABS_LIST ||
        sorted_list->val.list.size == 0)
        return -1;
    long l = 0, r = (long)sorted_list->val.list.size - 1;
    while (l <= r) {
        long m = l + (r - l) / 2;
        int c = compare_objs(sorted_list->val.list.items[m], target);
        if (c == 0) return m;
        if (c < 0) l = m + 1;
        else r = m - 1;
    }
    return -1;
}
