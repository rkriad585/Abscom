#ifndef ABSCOM_ABS_DYNARRAY_H
#define ABSCOM_ABS_DYNARRAY_H

#include "abs_common.h"

ABS_BEGIN_C_DECLS

typedef struct abs_dynarray {
    void  *data;
    size_t elem_size;
    size_t len;
    size_t cap;
} abs_dynarray_t;

ABS_API int abs_dynarray_init(abs_dynarray_t *arr, size_t elem_size);
ABS_API void abs_dynarray_destroy(abs_dynarray_t *arr);
ABS_API void abs_dynarray_clear(abs_dynarray_t *arr);
ABS_API int abs_dynarray_reserve(abs_dynarray_t *arr, size_t cap);
ABS_API int abs_dynarray_push(abs_dynarray_t *arr, const void *elem);
ABS_API void abs_dynarray_pop(abs_dynarray_t *arr);
ABS_API int abs_dynarray_resize(abs_dynarray_t *arr, size_t new_len);
ABS_API void *abs_dynarray_at(abs_dynarray_t *arr, size_t index);
ABS_API const void *abs_dynarray_at_const(const abs_dynarray_t *arr, size_t index);
ABS_API void *abs_dynarray_data(abs_dynarray_t *arr);
ABS_API size_t abs_dynarray_len(const abs_dynarray_t *arr);
ABS_API size_t abs_dynarray_cap(const abs_dynarray_t *arr);

ABS_END_C_DECLS

#endif
