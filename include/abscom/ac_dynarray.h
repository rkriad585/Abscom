#ifndef ABSCOM_AC_DYNARRAY_H
#define ABSCOM_AC_DYNARRAY_H

#include "ac_common.h"

AC_BEGIN_C_DECLS

typedef struct ac_dynarray {
    void  *data;
    size_t elem_size;
    size_t len;
    size_t cap;
} ac_dynarray_t;

AC_API int ac_dynarray_init(ac_dynarray_t *arr, size_t elem_size);
AC_API void ac_dynarray_destroy(ac_dynarray_t *arr);
AC_API void ac_dynarray_clear(ac_dynarray_t *arr);
AC_API int ac_dynarray_reserve(ac_dynarray_t *arr, size_t cap);
AC_API int ac_dynarray_push(ac_dynarray_t *arr, const void *elem);
AC_API void ac_dynarray_pop(ac_dynarray_t *arr);
AC_API int ac_dynarray_resize(ac_dynarray_t *arr, size_t new_len);
AC_API void *ac_dynarray_at(ac_dynarray_t *arr, size_t index);
AC_API const void *ac_dynarray_at_const(const ac_dynarray_t *arr, size_t index);
AC_API void *ac_dynarray_data(ac_dynarray_t *arr);
AC_API size_t ac_dynarray_len(const ac_dynarray_t *arr);
AC_API size_t ac_dynarray_cap(const ac_dynarray_t *arr);

AC_END_C_DECLS

#endif
