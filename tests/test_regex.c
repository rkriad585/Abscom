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

    CHECK(re_match(v("^.*@.*$"), v("admin@abscom.org")) == true);
    CHECK(re_match(v("^.*@.*$"), v("no-at-sign")) == false);

    CHECK(re_match(v("abc"), v("xxabcyy")) == true);
    CHECK(re_match(v("^abc"), v("abcxx")) == true);
    CHECK(re_match(v("^abc"), v("xabc")) == false);
    CHECK(re_match(v("abc$"), v("xxabc")) == true);
    CHECK(re_match(v("abc$"), v("abcx")) == false);

    CHECK(re_match(v("a.c"), v("abc")) == true);
    CHECK(re_match(v("a.c"), v("axc")) == true);
    CHECK(re_match(v("a.c"), v("ac")) == false);

    CHECK(re_match(v("ab*c"), v("ac")) == true);
    CHECK(re_match(v("ab*c"), v("abbbc")) == true);
    CHECK(re_match(v("ab*c"), v("abx")) == false);

    CHECK(re_match(v(123), v("abc")) == false);
    CHECK(re_match(v("a"), v(42)) == false);

    var finds = re_findall(v("a"), v("banana"));
    CHECK(finds->type == ABS_LIST && finds->val.list.size == 3);
    CHECK(strcmp(finds->val.list.items[0]->val.s, "a") == 0);

    var pairs = re_findall(v("ab"), v("ababab"));
    CHECK(pairs->type == ABS_LIST && pairs->val.list.size == 3);

    var anchored = re_findall(v("^a"), v("abc"));
    CHECK(anchored->type == ABS_LIST && anchored->val.list.size == 1);

    var subs = re_sub(v("a"), v("o"), v("banana"));
    CHECK(subs->type == ABS_STR && strcmp(subs->val.s, "bonono") == 0);

    var subs2 = re_sub(v("l"), v("L"), v("hello"));
    CHECK(subs2->type == ABS_STR && strcmp(subs2->val.s, "heLLo") == 0);

    var subs3 = re_sub(v("ab"), v("-"), v("ababab"));
    CHECK(subs3->type == ABS_STR && strcmp(subs3->val.s, "---") == 0);

    CHECK(is_err(re_findall(v(1), v("x"))));
    CHECK(is_err(re_sub(v("a"), v("b"), v(5))));

    abs_cleanup();
    return 0;
}
