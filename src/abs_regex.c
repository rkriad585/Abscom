#include "abscom/abs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Minimal regex engine in the style of Rob Pike's classic matcher.
 * Supported syntax: . (any character), * (closure), ^ (anchors to the start
 * of the text) and $ (anchors to the end). Everything is plain C code, so it
 * works on Windows without external libraries.
 */

/* Returns 1 if regexp matches text starting at the very beginning of text. */
static int match_here(const char *regexp, const char *text);

static int match_star(int c, const char *regexp, const char *text) {
    do {
        if (match_here(regexp, text)) return 1;
    } while (*text != '\0' && (*text++ == c || c == '.'));
    return 0;
}

static int match_here(const char *regexp, const char *text) {
    if (regexp[0] == '\0') return 1;
    if (regexp[1] == '*') return match_star(regexp[0], regexp + 2, text);
    if (regexp[0] == '$' && regexp[1] == '\0') return *text == '\0';
    if (*text != '\0' && (regexp[0] == '.' || regexp[0] == *text))
        return match_here(regexp + 1, text + 1);
    return 0;
}

static int match(const char *regexp, const char *text) {
    if (regexp[0] == '^') return match_here(regexp + 1, text);
    do {
        if (match_here(regexp, text)) return 1;
    } while (*text++ != '\0');
    return 0;
}

bool re_match(var pattern, var text) {
    if (!pattern || !text || pattern->type != ABS_STR || text->type != ABS_STR)
        return false;
    return match(pattern->val.s, text->val.s);
}

/* Length-reporting variant: returns how many characters a match consumes when
 * the pattern matches at the very beginning of text, or -1 for no match. */
static long match_len(const char *regexp, const char *text) {
    if (regexp[0] == '\0') return 0;
    if (regexp[1] == '*') {
        long best = -1;
        for (long used = 0;; used++) {
            long rest = match_len(regexp + 2, text + used);
            if (rest >= 0 && (best < 0 || used + rest > best))
                best = used + rest;
            if (text[used] == '\0') break;
            if (text[used] != regexp[0] && regexp[0] != '.') break;
        }
        return best;
    }
    if (regexp[0] == '$' && regexp[1] == '\0') return *text == '\0' ? 0 : -1;
    if (*text != '\0' && (regexp[0] == '.' || regexp[0] == *text)) {
        long rest = match_len(regexp + 1, text + 1);
        if (rest < 0) return -1;
        return rest + 1;
    }
    return -1;
}

/* Length of a match anchored at the given position, honouring a ^ anchor. */
static long match_at(const char *regexp, const char *text) {
    if (regexp[0] == '^') return match_len(regexp + 1, text);
    return match_len(regexp, text);
}

static var str_n(const char *s, size_t n) {
    char *buf = (char *)malloc(n + 1);
    if (!buf) return None;
    memcpy(buf, s, n);
    buf[n] = '\0';
    var res = abs_new_str(buf);
    free(buf);
    return res;
}

var re_findall(var pattern, var text) {
    if (!pattern || !text || pattern->type != ABS_STR || text->type != ABS_STR)
        return abs_new_error("re_findall expects two strings");
    var out = abs_new_list();
    if (!out) return None;
    const char *re = pattern->val.s;
    const char *t = text->val.s;
    while (*t != '\0') {
        long mlen = match_at(re, t);
        if (mlen >= 0) {
            append(out, str_n(t, (size_t)mlen));
            if (mlen == 0) t++; /* avoid infinite loop on empty matches */
            else t += mlen;
        } else {
            t++;
        }
    }
    return out;
}

var re_sub(var pattern, var repl, var text) {
    if (!pattern || !text || pattern->type != ABS_STR || text->type != ABS_STR)
        return abs_new_error("re_sub expects three strings");
    if (!repl || repl->type != ABS_STR) repl = v("");
    abs_string_t out;
    abs_string_init(&out);
    const char *re = pattern->val.s;
    const char *t = text->val.s;
    const char *r = repl->val.s;
    while (*t != '\0') {
        long mlen = match_at(re, t);
        if (mlen >= 0) {
            abs_string_append_cstr(&out, r);
            if (mlen == 0) {
                abs_string_append_char(&out, *t);
                t++;
            } else {
                t += mlen;
            }
        } else {
            abs_string_append_char(&out, *t);
            t++;
        }
    }
    var res = abs_new_str(abs_string_c_str(&out));
    abs_string_destroy(&out);
    return res;
}
