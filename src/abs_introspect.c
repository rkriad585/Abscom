/* Introspection helpers: id() returns the object's memory address as an int,
 * repr() returns a debug string, and dir() lists dict keys (and list indices).
 */

#include "abscom/abs.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

var id(var obj) {
    if (!obj) return abs_new_int(0);
    return abs_new_int((long)(intptr_t)(void *)obj);
}

var repr(var obj) {
    char buf[128];
    if (!obj) {
        snprintf(buf, sizeof(buf), "<AbsObj Type=none Addr=0x0>");
    } else {
        snprintf(buf, sizeof(buf), "<AbsObj Type=%d Addr=%p>", (int)obj->type,
                 (void *)obj);
    }
    return abs_new_str(buf);
}

var dir(var obj) {
    var out = abs_new_list();
    if (!out) return None;
    if (!obj) return out;
    if (obj->type == ABS_DICT) {
        for (size_t i = 0; i < obj->val.dict.capacity; i++) {
            DictNode *node = obj->val.dict.buckets[i];
            while (node) {
                append(out, abs_new_str(node->key));
                node = node->next;
            }
        }
    } else if (obj->type == ABS_LIST) {
        for (size_t i = 0; i < obj->val.list.size; i++) append(out, abs_new_int((long)i));
    }
    return out;
}
