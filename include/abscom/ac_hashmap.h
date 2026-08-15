#ifndef ABSCOM_AC_HASHMAP_H
#define ABSCOM_AC_HASHMAP_H

#include "ac_common.h"

AC_BEGIN_C_DECLS

typedef struct ac_hashmap ac_hashmap_t;

typedef void (*ac_hashmap_free_fn)(void *value);
typedef int (*ac_hashmap_visit_fn)(const char *key, void *value, void *user);

AC_API ac_hashmap_t *ac_hashmap_create(ac_hashmap_free_fn free_value);
AC_API void ac_hashmap_destroy(ac_hashmap_t *m);
AC_API void ac_hashmap_clear(ac_hashmap_t *m);
AC_API int ac_hashmap_set(ac_hashmap_t *m, const char *key, void *value);
AC_API void *ac_hashmap_get(const ac_hashmap_t *m, const char *key);
AC_API int ac_hashmap_contains(const ac_hashmap_t *m, const char *key);
AC_API int ac_hashmap_remove(ac_hashmap_t *m, const char *key);
AC_API size_t ac_hashmap_size(const ac_hashmap_t *m);
AC_API void ac_hashmap_foreach(ac_hashmap_t *m, ac_hashmap_visit_fn fn, void *user);

AC_END_C_DECLS

#endif
