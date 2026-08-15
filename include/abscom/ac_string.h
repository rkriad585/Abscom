#ifndef ABSCOM_AC_STRING_H
#define ABSCOM_AC_STRING_H

#include "ac_common.h"

AC_BEGIN_C_DECLS

typedef struct ac_string {
    char  *data;
    size_t len;
    size_t cap;
} ac_string_t;

AC_API int ac_string_init(ac_string_t *s);
AC_API int ac_string_init_cstr(ac_string_t *s, const char *str);
AC_API int ac_string_init_n(ac_string_t *s, const char *str, size_t n);
AC_API void ac_string_destroy(ac_string_t *s);
AC_API void ac_string_clear(ac_string_t *s);
AC_API int ac_string_reserve(ac_string_t *s, size_t cap);
AC_API int ac_string_shrink_to_fit(ac_string_t *s);
AC_API int ac_string_append_cstr(ac_string_t *s, const char *str);
AC_API int ac_string_append_n(ac_string_t *s, const char *str, size_t n);
AC_API int ac_string_append_char(ac_string_t *s, char c);
AC_API int ac_string_append_fmt(ac_string_t *s, const char *fmt, ...);
AC_API int ac_string_set_cstr(ac_string_t *s, const char *str);
AC_API const char *ac_string_c_str(const ac_string_t *s);
AC_API size_t ac_string_len(const ac_string_t *s);
AC_API char *ac_string_take(ac_string_t *s);

AC_END_C_DECLS

#endif
