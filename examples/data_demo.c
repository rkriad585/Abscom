#include "abscom/abs.h"

static var int_list(const long *vals, int n) {
    var l = List();
    for (int i = 0; i < n; i++) append(l, abs_new_int(vals[i]));
    return l;
}

int main(void) {
    abs_init();

    print(v("=== 1. Math & Aggregates ==="));
    var nums = int_list((long[]){10, 5, 20, 2}, 4);
    print(v("List:"), nums);
    print(v("Min:"), min_val(nums));
    print(v("Max:"), max_val(nums));
    print(v("Sum:"), sum_val(nums));

    var f = v(-3.14159);
    print(v("Abs:"), abs_val(f));
    print(v("Round:"), round_val(abs_val(f), 2));
    print(v("Pow:"), pow_val(v(2), v(8)));

    print(v("=== 2. Sorting & Sequences ==="));
    var l = int_list((long[]){3, 1, 4, 1, 5}, 5);
    print(v("Original:"), l);
    print(v("Sorted:  "), sorted(l, false));
    print(v("Reverse: "), sorted(l, true));
    print(v("Reversed:"), reversed_seq(l));

    var names = List();
    append(names, v("cherry"));
    append(names, v("apple"));
    append(names, v("banana"));
    print(v("Names sorted:"), sorted(names, false));

    var zipped = zip_lists(int_list((long[]){1, 2, 3}, 3), names);
    print(v("Zip:"), zipped);

    print(v("=== 3. Strings ==="));
    var s = v("   Abscom Library   ");
    print(v("Stripped:"), fmt("'{}'", strip(s)));

    var fname = v("script.py");
    if (is_true(endswith(fname, v(".py")))) {
        print(v("This is a Python file."));
    }
    if (is_true(startswith(strip(s), v("Abscom")))) {
        print(v("It starts with Abscom!"));
    }
    print(v("Count of 'an' in banana:"), count(v("banana"), v("an")));

    print(v("=== 4. Truthiness ==="));
    var empty_l = List();
    var full_l = List();
    append(full_l, v(1));
    if (!is_true(empty_l)) print(v("Empty list is False"));
    if (is_true(full_l)) print(v("Full list is True"));
    print(v("not_(0):"), not_(v(0)));
    print(v("Any (in empty):"), any(empty_l));
    print(v("All (in empty):"), all(empty_l));

    print(v("=== 5. System ==="));
    print(v("Time timestamp:"), time_now());
    print(v("Sleeping for 1 second..."));
    sleep_sec(1.0);
    print(v("Awake!"));
    print(v("Exec exit code:"), exec_cmd("echo hello from exec_cmd"));

    abs_cleanup();
    return 0;
}
