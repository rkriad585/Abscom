#include "abscom/ac_py.h"

static var square_it(var x) {
    if (x->type == ABS_INT) return abs_new_int(x->val.i * x->val.i);
    return x;
}

static bool is_odd_b(var x) {
    return x->type == ABS_INT && x->val.i % 2 != 0;
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    abs_init();

    print(v("--- 1. Memory Pool & Sets ---"));
    var s = Set();
    set_add(s, v(3));
    set_add(s, v(1));
    set_add(s, v(3));
    set_add(s, v("hi"));
    set_add(s, v("hi"));
    print(v("Set (deduped):"), s);
    print(v("Contains 1?"), v(set_contains(s, v(1))));
    print(v("Contains 9?"), v(set_contains(s, v(9))));

    var s2 = Set();
    set_add(s2, v(1));
    set_add(s2, v(2));
    set_add(s2, v(9));
    print(v("Union:"), set_union(s, s2));
    print(v("Diff (s - s2):"), set_diff(s, s2));

    print(v("--- 2. foreach Macro ---"));
    var nums = List();
    for (int i = 0; i < 5; i++) append(nums, v(i));
    var item;
    long total = 0;
    foreach (item, nums) {
        total += item->val.i;
    }
    print(v("foreach sum ="), abs_new_int(total));

    print(v("--- 3. List Comprehension ---"));
    print(v("Squares of evens:"), list_comp(range(0, 10), square_it, is_odd_b));
    print(v("Doubles (no filter):"), list_comp(range(0, 4), square_it, NULL));

    print(v("--- 4. range_step ---"));
    print(v("Even numbers:"), range_step(0, 10, 2));
    print(v("Countdown:"), range_step(10, 0, -3));

    print(v("--- 5. OOP: Classes & Instances ---"));
    var Dog = Class("Dog");
    var rex = New(Dog);
    set_attr(rex, "name", v("Rex"));
    set_attr(rex, "age", v(3));
    print(v("Instance:"), rex);
    print(v("Name attr:"), get_attr(rex, "name"));
    print(v("Age attr:"), get_attr(rex, "age"));
    print(v("Missing attr:"), get_attr(rex, "breed"));

    print(v("--- 6. JSON Stringify ---"));
    var user = Dict();
    dset(user, "name", v("John \"JD\" Doe"));
    dset(user, "age", v(30));
    dset(user, "active", v(True));
    var scores = List();
    append(scores, v(10));
    append(scores, v(20.5));
    append(scores, None);
    dset(user, "scores", scores);
    var dumped = json_dump(user);
    print(v("JSON:"), dumped);

    print(v("--- 7. HTTP/1.0 GET ---"));
    var html = http_get("http://example.com/");
    if (is_err(html)) {
        print(v("HTTP error:"), html);
    } else if (html->type == ABS_STR) {
        print(v("HTTP body length:"), len(html));
    }

    abs_cleanup();
    return 0;
}
