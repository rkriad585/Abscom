#include "abscom/ac_string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ac_string_init(ac_string_t *s) {
    if (!s) return -1;
    s->data = NULL;
    s->len = 0;
    s->cap = 0;
    return 0;
}

int ac_string_init_n(ac_string_t *s, const char *str, size_t n) {
    if (!s || (n > 0 && !str)) return -1;
    s->data = NULL;
    s->len = 0;
    s->cap = 0;
    if (n == 0) return 0;
    return ac_string_append_n(s, str, n);
}

int ac_string_init_cstr(ac_string_t *s, const char *str) {
    return ac_string_init_n(s, str, str ? strlen(str) : 0);
}

void ac_string_destroy(ac_string_t *s) {
    if (!s) return;
    free(s->data);
    s->data = NULL;
    s->len = 0;
    s->cap = 0;
}

void ac_string_clear(ac_string_t *s) {
    if (!s) return;
    s->len = 0;
    if (s->data) s->data[0] = '\0';
}

static int ac_string_grow(ac_string_t *s, size_t need_len) {
    if (need_len <= s->cap) return 0;
    size_t new_cap = s->cap ? s->cap : 16;
    while (new_cap < need_len) {
        if (new_cap > SIZE_MAX / 2) {
            new_cap = need_len;
            break;
        }
        new_cap *= 2;
    }
    char *new_data = realloc(s->data, new_cap + 1);
    if (!new_data) return -1;
    s->data = new_data;
    s->cap = new_cap;
    return 0;
}

int ac_string_reserve(ac_string_t *s, size_t cap) {
    if (!s) return -1;
    if (cap <= s->cap) return 0;
    char *new_data = realloc(s->data, cap + 1);
    if (!new_data) return -1;
    s->data = new_data;
    s->cap = cap;
    return 0;
}

int ac_string_shrink_to_fit(ac_string_t *s) {
    if (!s) return -1;
    if (s->data && s->len < s->cap) {
        char *new_data = realloc(s->data, s->len + 1);
        if (!new_data) return -1;
        s->data = new_data;
        s->cap = s->len;
    }
    return 0;
}

int ac_string_append_n(ac_string_t *s, const char *str, size_t n) {
    if (!s || (n > 0 && !str)) return -1;
    if (n == 0) return 0;
    if (ac_string_grow(s, s->len + n) != 0) return -1;
    memcpy(s->data + s->len, str, n);
    s->len += n;
    s->data[s->len] = '\0';
    return 0;
}

int ac_string_append_cstr(ac_string_t *s, const char *str) {
    return ac_string_append_n(s, str, str ? strlen(str) : 0);
}

int ac_string_append_char(ac_string_t *s, char c) {
    return ac_string_append_n(s, &c, 1);
}

int ac_string_append_fmt(ac_string_t *s, const char *fmt, ...) {
    if (!s || !fmt) return -1;
    va_list ap;
    va_start(ap, fmt);
    int needed = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (needed < 0) return -1;
    if (ac_string_grow(s, s->len + (size_t)needed) != 0) return -1;
    va_start(ap, fmt);
    vsnprintf(s->data + s->len, (size_t)needed + 1, fmt, ap);
    va_end(ap);
    s->len += (size_t)needed;
    return 0;
}

int ac_string_set_cstr(ac_string_t *s, const char *str) {
    ac_string_clear(s);
    return ac_string_append_cstr(s, str);
}

const char *ac_string_c_str(const ac_string_t *s) {
    return (s && s->data) ? s->data : "";
}

size_t ac_string_len(const ac_string_t *s) {
    return s ? s->len : 0;
}

char *ac_string_take(ac_string_t *s) {
    if (!s) return NULL;
    char *data = s->data;
    s->data = NULL;
    s->len = 0;
    s->cap = 0;
    return data;
}
