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

int main(void) {
    abs_init();

    /* id() returns a stable, non-zero identity for every object. */
    var s = v("abc");
    var t = v("xyz");
    CHECK(id(s)->val.i != 0);
    CHECK(id(s)->val.i == id(s)->val.i);
    CHECK(id(s)->val.i != id(t)->val.i);

    /* repr() round-trips the type and address. */
    var r = repr(s);
    CHECK(r != NULL && r->type == ABS_STR);
    CHECK(strstr(r->val.s, "<AbsObj Type=") == r->val.s);
    CHECK(strstr(r->val.s, "Addr=") != NULL);

    /* dir() lists dictionary keys. */
    var d = abs_new_dict();
    dset(d, "name", v("Alice"));
    dset(d, "age", v(30));
    var keys = dir(d);
    CHECK(keys != NULL && keys->type == ABS_LIST);
    CHECK(keys->val.list.size == 2);
    int seen_name = 0;
    for (size_t i = 0; i < keys->val.list.size; i++) {
        var k = keys->val.list.items[i];
        if (k->type == ABS_STR && strcmp(k->val.s, "name") == 0) seen_name = 1;
    }
    CHECK(seen_name == 1);

    /* dir() on a list yields its indices. */
    var l = abs_new_list();
    append(l, v(10));
    append(l, v(20));
    var idx = dir(l);
    CHECK(idx != NULL && idx->type == ABS_LIST);
    CHECK(idx->val.list.size == 2);
    CHECK(idx->val.list.items[0]->val.i == 0);
    CHECK(idx->val.list.items[1]->val.i == 1);

    /* dir() on an int is an empty list. */
    var empty = dir(v(7));
    CHECK(empty != NULL && empty->type == ABS_LIST && empty->val.list.size == 0);

    abs_cleanup();
    return 0;
}
