#include "abscom/abs.h"

#include <stdio.h>
#include <string.h>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static var list_of(const long *vals, size_t n) {
    var list = abs_new_list();
    for (size_t i = 0; i < n; i++) append(list, v(vals[i]));
    return list;
}

static var perm(int n) {
    var list = abs_new_list();
    for (int i = 0; i < n; i++) append(list, v((long)i));
    shuffle(list);
    return list;
}

static int check_identity(var list, int n) {
    for (int i = 0; i < n; i++)
        CHECK(list->val.list.items[i]->val.i == (long)i);
    return 0;
}

static int vis_swaps = 0;
static void vis_hook(var list, int idx_a, int idx_b) {
    (void)list;
    (void)idx_a;
    (void)idx_b;
    vis_swaps++;
}

int main(void) {
    abs_init();

    /* is_sorted. */
    {
        long a[] = {1, 2, 3};
        var l = list_of(a, 3);
        CHECK(is_sorted(l));
        long b[] = {3, 1, 2};
        var u = list_of(b, 3);
        CHECK(!is_sorted(u));
        CHECK(!is_sorted(v(1)));
    }

    /* Every sort must reorder a shuffled 0..49 permutation back to identity. */
    {
        void (*sorts[])(var) = { sort_bubble, sort_selection, sort_insertion,
                                 sort_shell,   sort_heap,     sort_merge,
                                 sort_quick,   sort_counting, sort_radix,
                                 sort_bucket,  sort_c_qsort };
        size_t n = sizeof(sorts) / sizeof(sorts[0]);
        for (size_t s = 0; s < n; s++) {
            var l = perm(50);
            sorts[s](l);
            check_identity(l, 50);
        }
    }

    /* Bogo sort on a tiny list. */
    {
        long a[] = {3, 1, 2};
        var l = list_of(a, 3);
        sort_bogo(l);
        CHECK(l->val.list.items[0]->val.i == 1);
        CHECK(l->val.list.items[1]->val.i == 2);
        CHECK(l->val.list.items[2]->val.i == 3);
    }

    /* Mixed numeric comparisons. */
    {
        long a[] = {3, 2, 1};
        var l = list_of(a, 3);
        append(l, abs_new_float(2.5));
        sort_quick(l);
        CHECK(l->val.list.size == 4);
        CHECK(l->val.list.items[0]->val.i == 1);
        CHECK(l->val.list.items[1]->val.i == 2);
        CHECK(l->val.list.items[2]->type == ABS_FLOAT && l->val.list.items[2]->val.f == 2.5);
        CHECK(l->val.list.items[3]->val.i == 3);
    }

    /* String comparisons. */
    {
        var words = abs_new_list();
        append(words, v("banana"));
        append(words, v("apple"));
        append(words, v("cherry"));
        sort_c_qsort(words);
        CHECK(strcmp(words->val.list.items[0]->val.s, "apple") == 0);
        CHECK(strcmp(words->val.list.items[1]->val.s, "banana") == 0);
        CHECK(strcmp(words->val.list.items[2]->val.s, "cherry") == 0);
    }

    /* list_copy: independent container, copied primitive elements. */
    {
        long a[] = {7, 8, 9};
        var src = list_of(a, 3);
        var cp = list_copy(src);
        CHECK(cp != NULL && cp->type == ABS_LIST);
        CHECK(cp->val.list.size == 3);
        CHECK(cp->val.list.items[0] != src->val.list.items[0]);
        append(cp, v(10));
        CHECK(src->val.list.size == 3);
        CHECK(cp->val.list.size == 4);
        var bad = list_copy(v(1));
        CHECK(is_err(bad));
    }

    /* sort_bubble_visual: hook fires on every swap, list ends sorted. */
    {
        long a[] = {50, 10, 40, 20, 30};
        var l = list_of(a, 5);
        vis_swaps = 0;
        sort_bubble_visual(l, vis_hook);
        CHECK(vis_swaps > 0);
        CHECK(is_sorted(l));
    }

    /* timeit: leaves the original list untouched, returns a sane duration. */
    {
        var l = perm(50);
        var before = list_copy(l);
        double t = timeit(sort_bubble, l);
        CHECK(t >= 0.0);
        for (size_t i = 0; i < l->val.list.size; i++)
            CHECK(l->val.list.items[i]->val.i == before->val.list.items[i]->val.i);
        CHECK(timeit(NULL, l) == -1.0);
        CHECK(timeit(sort_bubble, v(1)) == -1.0);
    }

    /* binary_search. */
    {
        long a[] = {1, 3, 5, 7, 9};
        var l = list_of(a, 5);
        CHECK(binary_search(l, v(5)) == 2);
        CHECK(binary_search(l, v(1)) == 0);
        CHECK(binary_search(l, v(9)) == 4);
        CHECK(binary_search(l, v(4)) == -1);
        CHECK(binary_search(l, v(0)) == -1);
        CHECK(binary_search(l, v(10)) == -1);
        var empty = abs_new_list();
        CHECK(binary_search(empty, v(1)) == -1);
    }

    /* Empty and single-element lists must not crash. */
    {
        var empty = abs_new_list();
        sort_bubble(empty);
        sort_selection(empty);
        sort_insertion(empty);
        sort_shell(empty);
        sort_heap(empty);
        sort_merge(empty);
        sort_quick(empty);
        sort_counting(empty);
        sort_radix(empty);
        sort_c_qsort(empty);
        CHECK(is_sorted(empty)); /* an empty list is vacuously sorted */
        var one = abs_new_list();
        append(one, v(42));
        sort_merge(one);
        sort_quick(one);
        CHECK(one->val.list.items[0]->val.i == 42);
    }

    abs_cleanup();
    return 0;
}
