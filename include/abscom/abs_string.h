#ifndef ABSCOM_ABS_STRING_H
#define ABSCOM_ABS_STRING_H

#include "abs_common.h"

ABS_BEGIN_C_DECLS

typedef struct abs_string {
    char  *data;
    size_t len;
    size_t cap;
} abs_string_t;

ABS_API int abs_string_init(abs_string_t *s);
ABS_API int abs_string_init_cstr(abs_string_t *s, const char *str);
ABS_API int abs_string_init_n(abs_string_t *s, const char *str, size_t n);
ABS_API void abs_string_destroy(abs_string_t *s);
ABS_API void abs_string_clear(abs_string_t *s);
ABS_API int abs_string_reserve(abs_string_t *s, size_t cap);
ABS_API int abs_string_shrink_to_fit(abs_string_t *s);
ABS_API int abs_string_append_cstr(abs_string_t *s, const char *str);
ABS_API int abs_string_append_n(abs_string_t *s, const char *str, size_t n);
ABS_API int abs_string_append_char(abs_string_t *s, char c);
ABS_API int abs_string_append_fmt(abs_string_t *s, const char *fmt, ...);
ABS_API int abs_string_set_cstr(abs_string_t *s, const char *str);
ABS_API const char *abs_string_c_str(const abs_string_t *s);
ABS_API size_t abs_string_len(const abs_string_t *s);
ABS_API char *abs_string_take(abs_string_t *s);

ABS_END_C_DECLS

#endif
