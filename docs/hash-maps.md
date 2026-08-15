# Hash Maps

`abs_hashmap` is an open-addressing, string-keyed hash map with linear probing, tombstone deletion, and automatic resizing when the load factor passes 70%. It stores arbitrary `void *` values and can call an optional destructor for each value when entries are removed.

```c
#include "abscom/abs.h"
```

## Types

```c
typedef struct abs_hashmap abs_hashmap_t;

typedef void (*abs_hashmap_free_fn)(void *value);
typedef int  (*abs_hashmap_visit_fn)(const char *key, void *value, void *user);
```

`abs_hashmap_free_fn` is called for a value when its entry is removed, cleared, or the map is destroyed. `abs_hashmap_visit_fn` is the callback passed to `abs_hashmap_foreach`; return non-zero to stop iterating early.

## API

| Function | Description |
| --- | --- |
| `abs_hashmap_t *abs_hashmap_create(abs_hashmap_free_fn free_value)` | Create a map; `free_value` is optional. |
| `void abs_hashmap_destroy(abs_hashmap_t *m)` | Remove every entry and free the map. |
| `void abs_hashmap_clear(abs_hashmap_t *m)` | Remove every entry, keeping the map. |
| `int abs_hashmap_set(abs_hashmap_t *m, const char *key, void *value)` | Insert or replace a key. |
| `void *abs_hashmap_get(const abs_hashmap_t *m, const char *key)` | Value for `key`, or `NULL` if absent. |
| `int abs_hashmap_contains(const abs_hashmap_t *m, const char *key)` | 1 if present, 0 otherwise. |
| `int abs_hashmap_remove(abs_hashmap_t *m, const char *key)` | Remove a key; 0 on success, -1 if missing. |
| `size_t abs_hashmap_size(const abs_hashmap_t *m)` | Number of entries. |
| `void abs_hashmap_foreach(abs_hashmap_t *m, abs_hashmap_visit_fn fn, void *user)` | Visit each entry; stops early if `fn` returns non-zero. |

`abs_hashmap_set` returns `0` on success and `-1` on allocation failure or bad arguments. Keys are copied internally; values are stored by pointer.

## Example

```c
#include "abscom/abs.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct { int v; } box_t;

static void free_box(void *p) { free(p); }

static int count_visit(const char *key, void *value, void *user) {
    ABS_UNUSED(key);
    ABS_UNUSED(value);
    (*(size_t *)user)++;
    return 0;
}

int main(void) {
    abs_hashmap_t *m = abs_hashmap_create(free_box);

    for (int i = 0; i < 10; i++) {
        char key[32];
        snprintf(key, sizeof(key), "key-%d", i);
        box_t *b = malloc(sizeof(*b));
        b->v = i;
        abs_hashmap_set(m, key, b);
    }

    box_t *b = (box_t *)abs_hashmap_get(m, "key-4");
    printf("key-4 -> %d\n", b->v);                 /* 4 */
    printf("contains key-4: %d\n", abs_hashmap_contains(m, "key-4"));

    size_t seen = 0;
    abs_hashmap_foreach(m, count_visit, &seen);
    printf("visited %zu entries\n", seen);         /* 10 */

    abs_hashmap_destroy(m);                        /* frees all box_t values */
    return 0;
}
```

Back to [README](https://github.com/rkriad585/Abscom/blob/main/README.md).
