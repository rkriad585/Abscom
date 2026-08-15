#include "abscom/ac.h"
#include "abscom/ac_py.h"

#include <stdio.h>
#include <string.h>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static var double_it(var x) {
    if (x->type == ABS_INT) return abs_new_int(x->val.i * 2);
    return x;
}

static var is_even(var x) {
    if (x->type == ABS_INT) return abs_new_bool(x->val.i % 2 == 0);
    return abs_new_bool(false);
}

static bool is_even_b(var x) {
    return x->type == ABS_INT && x->val.i % 2 == 0;
}

int main(void) {
    abs_init();

    var x = v(10);
    var y = v(3.14);
    var name = v("Abscom");
    var flag = v(True);
    var n = None;

    CHECK(x != NULL && x->type == ABS_INT && x->val.i == 10);
    CHECK(y != NULL && y->type == ABS_FLOAT && y->val.f == 3.14);
    CHECK(name != NULL && name->type == ABS_STR && strcmp(name->val.s, "Abscom") == 0);
    CHECK(flag != NULL && flag->type == ABS_BOOL && flag->val.b == true);
    CHECK(n != NULL && n->type == ABS_NONE);
    CHECK(v(False)->type == ABS_BOOL);

    CHECK(len(v("abcd"))->val.i == 4);
    CHECK(strcmp(str(v(42))->val.s, "42") == 0);
    CHECK(strcmp(to_str(v(2.5))->val.s, "2.50") == 0);
    CHECK(to_str(name) == name);
    CHECK(strcmp(type(v(42))->val.s, "<class 'int'>") == 0);
    CHECK(strcmp(type(name)->val.s, "<class 'str'>") == 0);

    {
        var r = randint(1, 6);
        CHECK(r != NULL && r->val.i >= 1 && r->val.i <= 6);
    }
    {
        var r = random_float();
        CHECK(r != NULL && r->val.f >= 0.0 && r->val.f < 1.0);
    }

    {
        var u = uniform(10.5, 20.5);
        CHECK(u->val.f >= 10.5 && u->val.f <= 20.5);
        CHECK(randint(7, 7)->val.i == 7);

        seed(v(42));
        var f1 = random_float();
        seed(v(42));
        CHECK(f1->val.f == random_float()->val.f);
        seed(v("AbscomRulez"));
        var s1 = random_float();
        seed(v("AbscomRulez"));
        CHECK(s1->val.f == random_float()->val.f);

        var chars = v("ABC");
        var ch = choice(chars);
        CHECK(ch->type == ABS_STR && strlen(ch->val.s) == 1);
        CHECK(strchr("ABC", ch->val.s[0]) != NULL);

        var deck = List();
        for (int i = 0; i < 10; i++) append(deck, v(i));
        CHECK(is_err(choice(List())));
        CHECK(is_err(choice(v(""))));
        CHECK(is_err(choice(v(5))));

        var picks = choices(deck, 3);
        CHECK(picks->type == ABS_LIST && picks->val.list.size == 3);
        CHECK(is_err(choices(List(), 3)));
        CHECK(is_err(choices(v(5), 3)));

        var hand = sample(deck, 5);
        CHECK(hand->type == ABS_LIST && hand->val.list.size == 5);
        int dup = 0;
        for (int i = 0; i < 5; i++)
            for (int j = i + 1; j < 5; j++)
                if (get(hand, i)->val.i == get(hand, j)->val.i) dup++;
        CHECK(dup == 0);
        CHECK(is_err(sample(deck, 20)));
        CHECK(is_err(sample(deck, -1)));
        CHECK(is_err(sample(v("abc"), 2)));

        var sdeck = List();
        for (int i = 0; i < 6; i++) append(sdeck, v(i));
        shuffle(sdeck);
        CHECK(sdeck->val.list.size == 6);
        CHECK(len(sdeck)->val.i == 6);
    }

    {
        var r = range(0, 5);
        CHECK(r != NULL && r->type == ABS_LIST && r->val.list.size == 5);
        CHECK(get(r, 0)->val.i == 0);
        CHECK(get(r, 4)->val.i == 4);
        CHECK(is_err(get(r, 5)));
        CHECK(is_err(get(r, -6)));
        CHECK(len(r)->val.i == 5);
        CHECK(choice(r) != NULL);
        CHECK(strcmp(type(r)->val.s, "<class 'list'>") == 0);
    }

    {
        var l = List();
        CHECK(l->type == ABS_LIST && l->val.list.size == 0);
        append(l, v("Apple"));
        append(l, v(42));
        append(l, v(3.14));
        append(l, v(True));
        CHECK(l->val.list.size == 4);
        CHECK(strcmp(get(l, 0)->val.s, "Apple") == 0);
        CHECK(get(l, 1)->val.i == 42);
        CHECK(get(l, 3)->val.b == true);
        CHECK(get(l, -1)->val.b == true);
        CHECK(get(l, -4)->type == ABS_STR && strcmp(get(l, -4)->val.s, "Apple") == 0);
        CHECK(len(l)->val.i == 4);
        CHECK(strcmp(to_str(l)->val.s, "[Apple, 42, 3.14, True]") == 0);
    }

    {
        CHECK(strcmp(add(v("Hello "), v("World"))->val.s, "Hello World") == 0);
        CHECK(add(v(10), v(20))->val.i == 30);
        CHECK(add(v(1), v(0.5))->val.f == 1.5);
        CHECK(add(v("a"), v(1))->type == ABS_NONE);

        CHECK(sub(v(10), v(4))->val.i == 6);
        CHECK(sub(v(5.5), v(0.5))->val.f == 5.0);
        CHECK(sub(v(1), v("a"))->type == ABS_NONE);

        CHECK(mul(v(6), v(7))->val.i == 42);
        CHECK(strcmp(mul(v(3), v("ab"))->val.s, "ababab") == 0);
        CHECK(strcmp(mul(v("ab"), v(2))->val.s, "abab") == 0);

        CHECK(eq(v(1), v(1.0))->val.b == true);
        CHECK(eq(v(1), v(2))->val.b == false);
        CHECK(eq(v("a"), v("a"))->val.b == true);
        CHECK(eq(v("a"), v("b"))->val.b == false);
        CHECK(eq(None, None)->val.b == true);
    }

    {
        var a = List();
        append(a, v(1));
        append(a, v(2));
        var b = List();
        append(b, v(3));
        var c = add(a, b);
        CHECK(c->type == ABS_LIST && c->val.list.size == 3);
        CHECK(get(c, 2)->val.i == 3);
    }

    {
        CHECK(strcmp(upper(v("make me uppercase"))->val.s, "MAKE ME UPPERCASE") == 0);
        CHECK(strcmp(lower(v("SHOUT"))->val.s, "shout") == 0);
        CHECK(upper(v(5))->type == ABS_NONE);
        CHECK(strcmp(upper(v("Abc"))->val.s, "ABC") == 0);
    }

    {
        CHECK(to_int(v("123"))->val.i == 123);
        CHECK(to_int(v(2.9))->val.i == 2);
        CHECK(to_int(v(True))->val.i == 1);
        CHECK(to_int(v("abc"))->type == ABS_NONE);
        CHECK(to_float(v(2))->val.f == 2.0);
        CHECK(to_float(v("1.5"))->val.f == 1.5);
    }

    {
        var user = Dict();
        CHECK(user->type == ABS_DICT);
        dset(user, "name", v("John Doe"));
        dset(user, "role", v("Admin"));
        dset(user, "age", v(30));
        CHECK(strcmp(dget(user, "name")->val.s, "John Doe") == 0);
        CHECK(strcmp(dget(user, "role")->val.s, "Admin") == 0);
        CHECK(dget(user, "age")->val.i == 30);
        CHECK(dget(user, "missing")->type == ABS_NONE);
        dset(user, "name", v("Jane"));
        CHECK(strcmp(dget(user, "name")->val.s, "Jane") == 0);
        CHECK(user->val.dict.count == 3);
        CHECK(is_err(dget(v(5), "x")));
        CHECK(strcmp(type(user)->val.s, "<class 'dict'>") == 0);
        CHECK(len(user)->val.i == 3);
        char *ds = to_str(user)->val.s;
        CHECK(strstr(ds, "name: Jane") != NULL);
        CHECK(strstr(ds, "role: Admin") != NULL);
    }

    {
        var fruits = split(v("apple,banana,grape"), ",");
        CHECK(fruits->val.list.size == 3);
        CHECK(strcmp(get(fruits, 0)->val.s, "apple") == 0);
        CHECK(strcmp(get(fruits, 1)->val.s, "banana") == 0);
        var joined = join(v(" | "), fruits);
        CHECK(strcmp(joined->val.s, "apple | banana | grape") == 0);

        var spaced = split(v("a b c"), " ");
        CHECK(spaced->val.list.size == 3);

        var mixed = List();
        append(mixed, v("a"));
        append(mixed, v(1));
        append(mixed, v("b"));
        CHECK(strcmp(join(v("-"), mixed)->val.s, "a--b") == 0);

        CHECK(is_err(split(v(5), ",")));
    }

    {
        var nums = List();
        for (int i = 0; i < 10; i++) append(nums, v(i));
        var s = slice(nums, 2, 5);
        CHECK(s->val.list.size == 3);
        CHECK(get(s, 0)->val.i == 2);
        CHECK(get(s, 2)->val.i == 4);
        var s2 = slice(nums, -3, -1);
        CHECK(s2->val.list.size == 2);
        CHECK(get(s2, 0)->val.i == 7);
        CHECK(get(s2, 1)->val.i == 8);
        CHECK(get(nums, -1)->val.i == 9);
        CHECK(get(nums, -2)->val.i == 8);
        CHECK(is_err(get(nums, 100)));
        CHECK(is_err(slice(v("x"), 0, 1)));
    }

    {
        var msg = fmt("Hello {}, your ID is {}", v("Bob"), v(42));
        CHECK(strcmp(msg->val.s, "Hello Bob, your ID is 42") == 0);
        CHECK(strcmp(fmt("plain {}", v(1))->val.s, "plain 1") == 0);
        var l = List();
        append(l, v(1));
        append(l, v(2));
        CHECK(strcmp(fmt("list = {}", l)->val.s, "list = [1, 2]") == 0);
    }

    {
        var nums = range(0, 10);
        var doubled = map_func(nums, double_it);
        CHECK(doubled->val.list.size == 10);
        CHECK(get(doubled, 3)->val.i == 6);
        var evens = filter_func(nums, is_even);
        CHECK(evens->val.list.size == 5);
        CHECK(get(evens, 0)->val.i == 0);
        CHECK(get(evens, 4)->val.i == 8);
        CHECK(is_err(map_func(v("x"), double_it)));
    }

    {
        var f = fopen_safe("test_py_tmp.txt", "w");
        CHECK(f != NULL && f->type == ABS_FILE);
        write_file(f, v("hello file\n"));
        write_file(f, v("second line"));
        close_file(f);
        var f2 = fopen_safe("test_py_tmp.txt", "r");
        var content = read_file(f2);
        CHECK(strcmp(content->val.s, "hello file\nsecond line") == 0);
        close_file(f2);
        remove("test_py_tmp.txt");
        var bad = fopen_safe("no_such_file_xyz_abscom.txt", "r");
        CHECK(is_err(bad));
        CHECK(is_err(read_file(v(5))));
    }

    {
        var data = json_parse("{\"id\": 101, \"scores\": [10, 20, 30], \"active\": true, \"name\": \"john\\\"ny\", \"pi\": 3.14, \"nested\": {\"k\": [1, 2]}}");
        CHECK(!is_err(data));
        CHECK(dget(data, "id")->val.i == 101);
        var scores = dget(data, "scores");
        CHECK(scores->type == ABS_LIST && scores->val.list.size == 3);
        CHECK(get(scores, 1)->val.i == 20);
        CHECK(dget(data, "active")->val.b == true);
        CHECK(strcmp(dget(data, "name")->val.s, "john\"ny") == 0);
        CHECK(dget(data, "pi")->val.f == 3.14);
        var nested = dget(data, "nested");
        CHECK(nested->type == ABS_DICT);
        var arr = dget(nested, "k");
        CHECK(arr->type == ABS_LIST && get(arr, 1)->val.i == 2);

        var top = json_parse("[1, 2.5, \"three\", true, null]");
        CHECK(top->type == ABS_LIST && top->val.list.size == 5);
        CHECK(get(top, 1)->val.f == 2.5);
        CHECK(get(top, 3)->val.b == true);
        CHECK(get(top, 4)->type == ABS_NONE);

        CHECK(is_err(json_parse("{\"a\": }")));
        CHECK(is_err(json_parse("")));
        CHECK(is_err(json_parse("123abc")));
    }

    {
        var doomed = v(111);
        CHECK(doomed->val.i == 111);
        del(doomed);
        var alive = v(222);
        CHECK(alive->val.i == 222);
        var l2 = List();
        append(l2, v(1));
        append(l2, v(2));
        del(l2);
        var after = v(333);
        CHECK(after->val.i == 333);
        var doomed_dict = Dict();
        dset(doomed_dict, "k", v(1));
        del(doomed_dict);
        CHECK(v(444)->val.i == 444);
    }

    {
        var e = get(range(0, 3), 100);
        CHECK(is_err(e));
        CHECK(strstr(e->val.error_msg, "out of bounds") != NULL);
        CHECK(strcmp(type(e)->val.s, "<class 'error'>") == 0);
    }

    {
        var nums = List();
        append(nums, v(10));
        append(nums, v(5));
        append(nums, v(20));
        append(nums, v(2));
        CHECK(max_val(nums)->val.i == 20);
        CHECK(min_val(nums)->val.i == 2);
        CHECK(sum_val(nums)->val.i == 37);
        CHECK(max_val(List())->type == ABS_NONE);
        CHECK(min_val(List())->type == ABS_NONE);

        var floats = List();
        append(floats, v(1.5));
        append(floats, v(2));
        CHECK(sum_val(floats)->val.f == 3.5);

        var words = List();
        append(words, v("banana"));
        append(words, v("apple"));
        append(words, v("cherry"));
        CHECK(strcmp(max_val(words)->val.s, "cherry") == 0);
        CHECK(strcmp(min_val(words)->val.s, "apple") == 0);
    }

    {
        CHECK(!is_true(v(0)));
        CHECK(is_true(v(1)));
        CHECK(!is_true(None));
        CHECK(!is_true(List()));
        CHECK(!is_true(v("")));
        CHECK(is_true(v("x")));
        CHECK(!is_true(v(0.0)));
        CHECK(not_(v(0))->val.b == true);
        CHECK(not_(v(5))->val.b == false);

        var l = List();
        append(l, v(0));
        append(l, v(""));
        append(l, v(1));
        CHECK(any(l)->val.b == true);
        CHECK(all(l)->val.b == false);
        var t = List();
        append(t, v(1));
        append(t, v("x"));
        append(t, v(True));
        CHECK(all(t)->val.b == true);
        CHECK(any(List())->val.b == false);
        CHECK(all(List())->val.b == true);
    }

    {
        CHECK(abs_val(v(-5))->val.i == 5);
        CHECK(abs_val(v(-3.5))->val.f == 3.5);
        CHECK(pow_val(v(2), v(3))->val.f == 8.0);
        CHECK(round_val(v(3.14159), 2)->val.f == 3.14);
        CHECK(round_val(v(3.7), 0)->val.i == 4);
        var five = v(5);
        CHECK(round_val(five, 2) == five);
    }

    {
        var l = List();
        append(l, v(3));
        append(l, v(1));
        append(l, v(4));
        append(l, v(1));
        append(l, v(5));
        var asc = sorted(l, false);
        CHECK(asc->val.list.size == 5);
        CHECK(get(asc, 0)->val.i == 1);
        CHECK(get(asc, 4)->val.i == 5);
        var desc = sorted(l, true);
        CHECK(get(desc, 0)->val.i == 5);
        CHECK(get(desc, 4)->val.i == 1);
        CHECK(l->val.list.size == 5);
        CHECK(get(l, 0)->val.i == 3);

        var r = reversed_seq(l);
        CHECK(r->val.list.size == 5);
        CHECK(get(r, 0)->val.i == 5);
        CHECK(get(r, 4)->val.i == 3);

        var names = List();
        append(names, v("banana"));
        append(names, v("apple"));
        append(names, v("cherry"));
        CHECK(strcmp(get(sorted(names, false), 0)->val.s, "apple") == 0);
        CHECK(strcmp(get(sorted(names, true), 0)->val.s, "cherry") == 0);

        var a = List();
        append(a, v("x"));
        append(a, v("y"));
        var b = List();
        append(b, v(1));
        append(b, v(2));
        var z = zip_lists(a, b);
        CHECK(z->val.list.size == 2);
        var p0 = get(z, 0);
        CHECK(p0->type == ABS_LIST && p0->val.list.size == 2);
        CHECK(strcmp(get(p0, 0)->val.s, "x") == 0);
        CHECK(get(p0, 1)->val.i == 1);
        CHECK(zip_lists(a, List())->val.list.size == 0);
    }

    {
        var s = v("   Abscom Library   ");
        CHECK(strcmp(strip(s)->val.s, "Abscom Library") == 0);
        CHECK(strcmp(strip(v(""))->val.s, "") == 0);
        CHECK(strip(v(5))->type == ABS_NONE);

        CHECK(startswith(v("Abscom"), v("Abs"))->val.b == true);
        CHECK(startswith(v("Abscom"), v("X"))->val.b == false);
        CHECK(startswith(v(5), v("A"))->val.b == false);
        CHECK(endswith(v("script.py"), v(".py"))->val.b == true);
        CHECK(endswith(v("script.py"), v("sh"))->val.b == false);
        CHECK(endswith(v("ab"), v("abc"))->val.b == false);

        var l = List();
        append(l, v("a"));
        append(l, v("b"));
        append(l, v("a"));
        CHECK(count(l, v("a"))->val.i == 2);
        CHECK(count(l, v("z"))->val.i == 0);
        CHECK(count(v("banana"), v("an"))->val.i == 2);
        CHECK(count(v("banana"), v("x"))->val.i == 0);
    }

    {
        var t = time_now();
        CHECK(t->type == ABS_FLOAT && t->val.f > 0);
        CHECK(is_int(exec_cmd("echo test")));
        sleep_sec(0.001);
    }

    {
        CHECK(is_int(v(1)));
        CHECK(!is_int(v(1.0)));
        CHECK(is_float(v(1.0)));
        CHECK(is_str(v("x")));
        CHECK(is_list(List()));
        CHECK(is_dict(Dict()));
        CHECK(is_none(None));
        CHECK(!is_none(v(0)));
        CHECK(is_none(NULL));
        CHECK(compare_objs(v(1), v(2)) < 0);
        CHECK(compare_objs(v("b"), v("a")) > 0);
    }

    {
        var s = abs_new_set();
        CHECK(s != NULL && is_set(s));
        CHECK(strcmp(type(s)->val.s, "<class 'set'>") == 0);
        set_add(s, v(3));
        set_add(s, v(1));
        set_add(s, v(3));
        set_add(s, v("x"));
        set_add(s, v("x"));
        CHECK(s->val.list.size == 3);
        CHECK(get_len_fast(s) == 3);
        CHECK(strcmp(str(s)->val.s, "{3, 1, x}") == 0);
        CHECK(set_contains(s, v(1)));
        CHECK(!set_contains(s, v(99)));

        var s2 = abs_new_set();
        set_add(s2, v(1));
        set_add(s2, v(2));
        var u = set_union(s, s2);
        CHECK(u->type == ABS_SET && u->val.list.size == 4);
        var d = set_diff(s, s2);
        CHECK(d->type == ABS_SET && d->val.list.size == 2);
        var e = set_diff(s2, s);
        CHECK(e->val.list.size == 1);
        CHECK(set_contains(u, v(2)));
        CHECK(set_contains(d, v(3)));
        CHECK(!set_contains(d, v(1)));
    }

    {
        var nums = range(0, 10);
        long total = 0;
        var item;
        foreach (item, nums) {
            total += item->val.i;
        }
        CHECK(total == 45);
        CHECK(get_len_fast(nums) == 10);
        CHECK(get_len_fast(v(5)) == 0);
        CHECK(get_len_fast(NULL) == 0);
    }

    {
        var squares = list_comp(range(0, 10), double_it, is_even_b);
        CHECK(squares->type == ABS_LIST && squares->val.list.size == 5);
        CHECK(get(squares, 0)->val.i == 0);
        CHECK(get(squares, 1)->val.i == 4);
        CHECK(get(squares, 4)->val.i == 16);
        var doubles = list_comp(range(0, 3), double_it, NULL);
        CHECK(doubles->val.list.size == 3);
        CHECK(get(doubles, 2)->val.i == 4);
        var copies = list_comp(range(0, 2), NULL, NULL);
        CHECK(copies->val.list.size == 2);
        CHECK(get(copies, 1)->val.i == 1);
        CHECK(list_comp(v("x"), double_it, is_even_b)->val.list.size == 0);
    }

    {
        var ev = range_step(0, 10, 2);
        CHECK(ev->val.list.size == 5);
        CHECK(get(ev, 0)->val.i == 0);
        CHECK(get(ev, 4)->val.i == 8);
        var down = range_step(10, 0, -2);
        CHECK(down->val.list.size == 5);
        CHECK(get(down, 0)->val.i == 10);
        CHECK(get(down, 4)->val.i == 2);
        CHECK(range_step(1, 5, 0)->val.list.size == 0);
    }

    {
        var Dog = Class("Dog");
        CHECK(Dog != NULL && Dog->type == ABS_CLASS);
        CHECK(strcmp(type(Dog)->val.s, "<class 'class'>") == 0);
        var d = New(Dog);
        CHECK(d != NULL && d->type == ABS_INSTANCE);
        CHECK(strcmp(type(d)->val.s, "<class 'instance'>") == 0);
        CHECK(strcmp(str(d)->val.s, "<Dog object>") == 0);
        set_attr(d, "name", v("Rex"));
        set_attr(d, "age", v(3));
        CHECK(strcmp(get_attr(d, "name")->val.s, "Rex") == 0);
        CHECK(get_attr(d, "age")->val.i == 3);
        CHECK(get_attr(d, "missing")->type == ABS_NONE);
        CHECK(New(v(5))->type == ABS_NONE);
        CHECK(New(NULL)->type == ABS_NONE);
    }

    {
        CHECK(strcmp(json_dump(v(42))->val.s, "42") == 0);
        CHECK(strcmp(json_dump(v(-7))->val.s, "-7") == 0);
        CHECK(strcmp(json_dump(v(3.5))->val.s, "3.5") == 0);
        CHECK(strcmp(json_dump(v(True))->val.s, "true") == 0);
        CHECK(strcmp(json_dump(v(False))->val.s, "false") == 0);
        CHECK(strcmp(json_dump(None)->val.s, "null") == 0);
        CHECK(strcmp(json_dump(v("hi"))->val.s, "\"hi\"") == 0);
        CHECK(strcmp(json_dump(v("a\"b\\c\n"))->val.s, "\"a\\\"b\\\\c\\n\"") == 0);

        var l = List();
        append(l, v(1));
        append(l, v("x"));
        append(l, v(2.5));
        CHECK(strcmp(json_dump(l)->val.s, "[1, \"x\", 2.5]") == 0);

        var nested = List();
        append(nested, l);
        append(nested, None);
        CHECK(strcmp(json_dump(nested)->val.s, "[[1, \"x\", 2.5], null]") == 0);

        var dd = Dict();
        dset(dd, "k", v(7));
        CHECK(strcmp(json_dump(dd)->val.s, "{\"k\": 7}") == 0);
    }

    {
        var e = http_get(NULL);
        CHECK(is_err(e));
        var e2 = http_get("");
        CHECK(is_err(e2));
    }

    print(v("Hello"), name, v("from the tests"));
    print_end("", v("no newline"));

    abs_cleanup();
    printf("test_py: OK\n");
    return 0;
}
