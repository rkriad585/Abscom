#include "abscom/abs.h"

static var double_it(var x) {
    if (x->type == ABS_INT) return abs_new_int(x->val.i * 2);
    return x;
}

static var is_even(var x) {
    if (x->type == ABS_INT) return abs_new_bool(x->val.i % 2 == 0);
    return abs_new_bool(false);
}

int main(void) {
    abs_init();

    print(v("--- 1. Dictionaries (Hash Maps) ---"));
    var user = Dict();
    dset(user, "name", v("John Doe"));
    dset(user, "role", v("Admin"));
    dset(user, "age", v(30));
    print(v("User Name:"), dget(user, "name"));
    print(v("User Role:"), dget(user, "role"));
    print(v("Full Dict:"), user);
    print(v("Missing key:"), dget(user, "nope"));

    print(v("--- 2. JSON Parsing ---"));
    var data = json_parse("{\"id\": 101, \"scores\": [10, 20, 30], \"active\": true, \"note\": null}");
    print(v("Parsed ID:"), dget(data, "id"));
    print(v("Parsed Scores:"), dget(data, "scores"));
    print(v("Parsed Active:"), dget(data, "active"));
    print(v("Parsed Note:"), dget(data, "note"));

    print(v("--- 3. Split & Join ---"));
    var csv = v("apple,banana,grape");
    var fruits = split(csv, ",");
    print(v("Split List:"), fruits);
    var clean = join(v(" | "), fruits);
    print(v("Joined Str:"), clean);

    print(v("--- 4. Slicing & Negative Index ---"));
    var nums = List();
    for (int i = 0; i < 10; i++) append(nums, v(i));
    print(v("Original:"), nums);
    print(v("Slice [2:5]:"), slice(nums, 2, 5));
    print(v("Slice [-3:-1]:"), slice(nums, -3, -1));
    print(v("Get Last (-1):"), get(nums, -1));

    print(v("--- 5. Functional Map / Filter ---"));
    print(v("Doubled:"), map_func(nums, double_it));
    print(v("Evens:"), filter_func(nums, is_even));

    print(v("--- 6. Formatted Strings ---"));
    var msg = fmt("Hello {}, your ID is {}", dget(user, "name"), dget(data, "id"));
    print(msg);

    print(v("--- 7. Error Handling ---"));
    var err_val = get(nums, 100);
    if (is_err(err_val)) {
        print(v("Caught Exception:"), err_val);
    }

    print(v("--- 8. File I/O ---"));
    var f = fopen_safe("demo_output.txt", "w");
    write_file(f, v("Abscom file I/O works.\n"));
    write_file(f, add(v("Pi = "), to_str(v(3.14159))));
    close_file(f);
    var rf = fopen_safe("demo_output.txt", "r");
    var contents = read_file(rf);
    print(v("File contents:"), contents);
    close_file(rf);

    print(v("--- 9. Random Module ---"));
    seed(v("AbscomRulez"));
    print(v("Numbers:"), random_float(), uniform(10.5, 20.5), randint(1, 100));

    var suits = List();
    append(suits, v("Hearts"));
    append(suits, v("Diamonds"));
    append(suits, v("Clubs"));
    append(suits, v("Spades"));
    var ranks = List();
    append(ranks, v("A"));
    append(ranks, v("2"));
    append(ranks, v("3"));
    append(ranks, v("4"));
    append(ranks, v("5"));
    append(ranks, v("6"));
    append(ranks, v("7"));
    append(ranks, v("8"));
    append(ranks, v("9"));
    append(ranks, v("10"));
    append(ranks, v("J"));
    append(ranks, v("Q"));
    append(ranks, v("K"));

    var deck = List();
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 13; j++)
            append(deck, fmt("{} of {}", get(ranks, j), get(suits, i)));
    print(v("Deck Size:"), len(deck));
    print(v("Pick a card:"), choice(deck));
    print(v("Shuffling..."));
    shuffle(deck);
    print(v("Top card is now:"), get(deck, 0));
    print(v("Dealt Hand (Sample):"), sample(deck, 5));

    var die_faces = List();
    for (int i = 1; i <= 6; i++) append(die_faces, v(i));
    print(v("Rolled 3 dice (Choices):"), choices(die_faces, 3));
    print(v("Random char from 'ABC':"), choice(v("ABC")));

    print(v("--- 10. Classic v2 Features ---"));
    print(v("Math:"), add(v(10), v(20)), sub(v(10), v(4)), mul(v(3), v("ab")));
    print(v("Upper / Lower:"), upper(v("hello")), lower(v("WORLD")));
    print(v("Equalities:"), eq(v(1), v(1.0)), eq(v("a"), v("b")));
    print(v("Random Choice:"), choice(nums));

    abs_cleanup();
    return 0;
}
