#include "abscom/ac_hashmap.h"

#include <stdio.h>
#include <stdlib.h>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
            return 1;                                                        \
        }                                                                    \
    } while (0)

typedef struct {
    int v;
} box_t;

static void free_box(void *p) {
    free(p);
}

static int count_cb(const char *key, void *value, void *user) {
    AC_UNUSED(key);
    AC_UNUSED(value);
    (*(size_t *)user)++;
    return 0;
}

int main(void) {
    ac_hashmap_t *m = ac_hashmap_create(free_box);
    CHECK(m != NULL);
    CHECK(ac_hashmap_size(m) == 0);

    for (int i = 0; i < 1000; i++) {
        char key[32];
        snprintf(key, sizeof(key), "key-%d", i);
        box_t *b = (box_t *)malloc(sizeof(*b));
        CHECK(b != NULL);
        b->v = i;
        CHECK(ac_hashmap_set(m, key, b) == 0);
    }
    CHECK(ac_hashmap_size(m) == 1000);

    for (int i = 0; i < 1000; i++) {
        char key[32];
        snprintf(key, sizeof(key), "key-%d", i);
        CHECK(ac_hashmap_contains(m, key));
        box_t *b = (box_t *)ac_hashmap_get(m, key);
        CHECK(b != NULL && b->v == i);
    }
    CHECK(!ac_hashmap_contains(m, "missing"));
    CHECK(ac_hashmap_get(m, "missing") == NULL);

    for (int i = 0; i < 1000; i += 2) {
        char key[32];
        snprintf(key, sizeof(key), "key-%d", i);
        CHECK(ac_hashmap_remove(m, key) == 0);
    }
    CHECK(ac_hashmap_size(m) == 500);
    CHECK(!ac_hashmap_contains(m, "key-0"));
    CHECK(ac_hashmap_contains(m, "key-1"));
    CHECK(ac_hashmap_remove(m, "key-0") == -1);

    for (int i = 0; i < 1000; i += 2) {
        char key[32];
        snprintf(key, sizeof(key), "key-%d", i);
        box_t *b = (box_t *)malloc(sizeof(*b));
        CHECK(b != NULL);
        b->v = i;
        CHECK(ac_hashmap_set(m, key, b) == 0);
    }
    CHECK(ac_hashmap_size(m) == 1000);
    for (int i = 0; i < 1000; i++) {
        char key[32];
        snprintf(key, sizeof(key), "key-%d", i);
        box_t *b = (box_t *)ac_hashmap_get(m, key);
        CHECK(b != NULL && b->v == i);
    }

    {
        size_t seen = 0;
        ac_hashmap_foreach(m, count_cb, &seen);
        CHECK(seen == 1000);
    }

    ac_hashmap_clear(m);
    CHECK(ac_hashmap_size(m) == 0);
    ac_hashmap_destroy(m);

    printf("test_hashmap: OK\n");
    return 0;
}
