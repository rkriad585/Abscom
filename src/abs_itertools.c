/* Itertools: lazy iterators built on the ABS_ITERATOR type.
 *
 *   chain(a, b)   yields every element of a, then every element of b
 *   cycle(a)      yields a's elements forever (wraps at the end)
 *   iter_next(i)  returns the next element, or None once exhausted
 */

#include "abscom/abs.h"

#include <stdio.h>
#include <stdlib.h>

var chain(var list_a, var list_b) {
    var o = abs_new_obj(ABS_ITERATOR);
    if (!o) return abs_new_error("Out of memory");
    o->val.iter.source_a = list_a;
    o->val.iter.source_b = list_b ? list_b : None;
    o->val.iter.index = 0;
    abs_gc_track(o);
    return o;
}

var cycle(var list) {
    var o = abs_new_obj(ABS_ITERATOR);
    if (!o) return abs_new_error("Out of memory");
    o->val.iter.source_a = (list && list->type == ABS_LIST) ? list : NULL;
    o->val.iter.source_b = NULL; /* marks "cycle" mode */
    o->val.iter.index = 0;
    abs_gc_track(o);
    return o;
}

var iter_next(var iter) {
    if (!iter || iter->type != ABS_ITERATOR)
        return abs_new_error("Not an iterator");
    var a = iter->val.iter.source_a;
    var b = iter->val.iter.source_b;
    if (b != NULL) {
        /* chain mode: source_a first, then source_b (non-list => empty). */
        long n = (a && a->type == ABS_LIST) ? (long)a->val.list.size : 0;
        if (iter->val.iter.index < n)
            return a->val.list.items[iter->val.iter.index++];
        if (b->type == ABS_LIST) {
            long b_idx = iter->val.iter.index - n;
            if (b_idx < (long)b->val.list.size) {
                iter->val.iter.index++;
                return b->val.list.items[b_idx];
            }
        }
        return None; /* both exhausted */
    }
    /* cycle mode (or empty/plain iterator) */
    if (!a || a->val.list.size == 0) return None;
    if (iter->val.iter.index >= (long)a->val.list.size)
        iter->val.iter.index = 0;
    return a->val.list.items[iter->val.iter.index++];
}
