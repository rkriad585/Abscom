#include "abscom/abs_string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int abs_string_init(abs_string_t *s) {
    if (!s) return -1;
    s->data = NULL;
    s->len = 0;
    s->cap = 0;
    return 0;
}

int abs_string_init_n(abs_string_t *s, const char *str, size_t n) {
    if (!s || (n > 0 && !str)) return -1;
    s->data = NULL;
    s->len = 0;
    s->cap = 0;
    if (n == 0) return 0;
    return abs_string_append_n(s, str, n);
}

int abs_string_init_cstr(abs_string_t *s, const char *str) {
    return abs_string_init_n(s, str, str ? strlen(str) : 0);
}

void abs_string_destroy(abs_string_t *s) {
    if (!s) return;
    free(s->data);
    s->data = NULL;
    s->len = 0;
    s->cap = 0;
}

void abs_string_clear(abs_string_t *s) {
    if (!s) return;
    s->len = 0;
    if (s->data) s->data[0] = '\0';
}

static int abs_string_grow(abs_string_t *s, size_t need_len) {
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

int abs_string_reserve(abs_string_t *s, size_t cap) {
    if (!s) return -1;
    if (cap <= s->cap) return 0;
    char *new_data = realloc(s->data, cap + 1);
    if (!new_data) return -1;
    s->data = new_data;
    s->cap = cap;
    return 0;
}

int abs_string_shrink_to_fit(abs_string_t *s) {
    if (!s) return -1;
    if (s->data && s->len < s->cap) {
        char *new_data = realloc(s->data, s->len + 1);
        if (!new_data) return -1;
        s->data = new_data;
        s->cap = s->len;
    }
    return 0;
}

int abs_string_append_n(abs_string_t *s, const char *str, size_t n) {
    if (!s || (n > 0 && !str)) return -1;
    if (n == 0) return 0;
    if (abs_string_grow(s, s->len + n) != 0) return -1;
    memcpy(s->data + s->len, str, n);
    s->len += n;
    s->data[s->len] = '\0';
    return 0;
}

int abs_string_append_cstr(abs_string_t *s, const char *str) {
    return abs_string_append_n(s, str, str ? strlen(str) : 0);
}

int abs_string_append_char(abs_string_t *s, char c) {
    return abs_string_append_n(s, &c, 1);
}

int abs_string_append_fmt(abs_string_t *s, const char *fmt, ...) {
    if (!s || !fmt) return -1;
    va_list ap;
    va_start(ap, fmt);
    int needed = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (needed < 0) return -1;
    if (abs_string_grow(s, s->len + (size_t)needed) != 0) return -1;
    va_start(ap, fmt);
    vsnprintf(s->data + s->len, (size_t)needed + 1, fmt, ap);
    va_end(ap);
    s->len += (size_t)needed;
    return 0;
}

int abs_string_set_cstr(abs_string_t *s, const char *str) {
    abs_string_clear(s);
    return abs_string_append_cstr(s, str);
}

const char *abs_string_c_str(const abs_string_t *s) {
    return (s && s->data) ? s->data : "";
}

size_t abs_string_len(const abs_string_t *s) {
    return s ? s->len : 0;
}

char *abs_string_take(abs_string_t *s) {
    if (!s) return NULL;
    char *data = s->data;
    s->data = NULL;
    s->len = 0;
    s->cap = 0;
    return data;
}
