/* Event emitter: an EventBus is an ABS_DICT mapping event names to a list of
 * ABS_FUNC handlers. on() registers a handler, emit() runs every handler for
 * an event in registration order.
 */

#include "abscom/abs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

var EventBus(void) {
    return abs_new_dict();
}

void on(var bus, const char *event_name, AbsFunc handler) {
    if (!bus || bus->type != ABS_DICT || !event_name || !handler) return;
    var list = dget(bus, event_name);
    if (list->type != ABS_LIST) {
        list = abs_new_list();
        dset(bus, event_name, list);
    }
    append(list, make_func(handler));
}

void emit(var bus, const char *event_name, var data) {
    if (!bus || bus->type != ABS_DICT || !event_name) return;
    var list = dget(bus, event_name);
    if (list->type != ABS_LIST) return;
    for (size_t i = 0; i < list->val.list.size; i++) {
        var f = list->val.list.items[i];
        if (f->type == ABS_FUNC) call_func(f, data);
    }
}
