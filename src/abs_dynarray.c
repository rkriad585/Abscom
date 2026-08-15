#include "abscom/abs_dynarray.h"

#include <stdlib.h>
#include <string.h>

int abs_dynarray_init(abs_dynarray_t *arr, size_t elem_size) {
    if (!arr || elem_size == 0) return -1;
    arr->data = NULL;
    arr->elem_size = elem_size;
    arr->len = 0;
    arr->cap = 0;
    return 0;
}

void abs_dynarray_destroy(abs_dynarray_t *arr) {
    if (!arr) return;
    free(arr->data);
    arr->data = NULL;
    arr->len = 0;
    arr->cap = 0;
}

void abs_dynarray_clear(abs_dynarray_t *arr) {
    if (arr) arr->len = 0;
}

int abs_dynarray_reserve(abs_dynarray_t *arr, size_t cap) {
    if (!arr) return -1;
    if (cap <= arr->cap) return 0;
    if (arr->elem_size != 0 && cap > SIZE_MAX / arr->elem_size) return -1;
    void *new_data = realloc(arr->data, cap * arr->elem_size);
    if (!new_data) return -1;
    arr->data = new_data;
    arr->cap = cap;
    return 0;
}

int abs_dynarray_push(abs_dynarray_t *arr, const void *elem) {
    if (!arr) return -1;
    if (arr->len == arr->cap) {
        size_t need = arr->cap ? arr->cap * 2 : 8;
        if (arr->cap > SIZE_MAX / 2) need = arr->cap + 1;
        if (abs_dynarray_reserve(arr, need) != 0) return -1;
    }
    memcpy((char *)arr->data + arr->len * arr->elem_size, elem, arr->elem_size);
    arr->len++;
    return 0;
}

void abs_dynarray_pop(abs_dynarray_t *arr) {
    if (arr && arr->len > 0) arr->len--;
}

int abs_dynarray_resize(abs_dynarray_t *arr, size_t new_len) {
    if (!arr) return -1;
    if (new_len > arr->cap) {
        if (abs_dynarray_reserve(arr, new_len) != 0) return -1;
    }
    if (new_len > arr->len) {
        memset((char *)arr->data + arr->len * arr->elem_size, 0,
               (new_len - arr->len) * arr->elem_size);
    }
    arr->len = new_len;
    return 0;
}

void *abs_dynarray_at(abs_dynarray_t *arr, size_t index) {
    if (!arr || index >= arr->len) return NULL;
    return (char *)arr->data + index * arr->elem_size;
}

const void *abs_dynarray_at_const(const abs_dynarray_t *arr, size_t index) {
    if (!arr || index >= arr->len) return NULL;
    return (const char *)arr->data + index * arr->elem_size;
}

void *abs_dynarray_data(abs_dynarray_t *arr) {
    return arr ? arr->data : NULL;
}

size_t abs_dynarray_len(const abs_dynarray_t *arr) {
    return arr ? arr->len : 0;
}

size_t abs_dynarray_cap(const abs_dynarray_t *arr) {
    return arr ? arr->cap : 0;
}
