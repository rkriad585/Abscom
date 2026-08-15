#ifndef ABSCOM_ABS_HASHMAP_H
#define ABSCOM_ABS_HASHMAP_H

#include "abs_common.h"

ABS_BEGIN_C_DECLS

typedef struct abs_hashmap abs_hashmap_t;

typedef void (*abs_hashmap_free_fn)(void *value);
typedef int (*abs_hashmap_visit_fn)(const char *key, void *value, void *user);

ABS_API abs_hashmap_t *abs_hashmap_create(abs_hashmap_free_fn free_value);
ABS_API void abs_hashmap_destroy(abs_hashmap_t *m);
ABS_API void abs_hashmap_clear(abs_hashmap_t *m);
ABS_API int abs_hashmap_set(abs_hashmap_t *m, const char *key, void *value);
ABS_API void *abs_hashmap_get(const abs_hashmap_t *m, const char *key);
ABS_API int abs_hashmap_contains(const abs_hashmap_t *m, const char *key);
ABS_API int abs_hashmap_remove(abs_hashmap_t *m, const char *key);
ABS_API size_t abs_hashmap_size(const abs_hashmap_t *m);
ABS_API void abs_hashmap_foreach(abs_hashmap_t *m, abs_hashmap_visit_fn fn, void *user);

ABS_END_C_DECLS

#endif
