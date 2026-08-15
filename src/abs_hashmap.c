#include "abscom/abs_hashmap.h"

#include "abscom/abs_hash.h"

#include <stdlib.h>
#include <string.h>

typedef enum {
    ABS_HM_EMPTY,
    ABS_HM_TOMBSTONE,
    ABS_HM_OCCUPIED
} abs_hm_state;

struct abs_hashmap {
    char             **keys;
    void             **values;
    unsigned char      *states;
    size_t              cap;
    size_t              count;
    size_t              tombstones;
    abs_hashmap_free_fn  free_value;
};

static const size_t abs_hm_min_cap = 16;

static char *abs_hm_strdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *copy = (char *)malloc(n);
    if (copy) memcpy(copy, s, n);
    return copy;
}

static int abs_hm_resize(abs_hashmap_t *m, size_t new_cap) {
    char **new_keys = (char **)calloc(new_cap, sizeof(*new_keys));
    void **new_vals = (void **)calloc(new_cap, sizeof(*new_vals));
    unsigned char *new_states = (unsigned char *)calloc(new_cap, sizeof(*new_states));
    if (!new_keys || !new_vals || !new_states) {
        free(new_keys);
        free(new_vals);
        free(new_states);
        return -1;
    }
    for (size_t i = 0; i < m->cap; i++) {
        if (m->states[i] != ABS_HM_OCCUPIED) continue;
        size_t idx = (size_t)abs_hash_fnv1a64_str(m->keys[i]) & (new_cap - 1);
        while (new_states[idx] == ABS_HM_OCCUPIED) idx = (idx + 1) & (new_cap - 1);
        new_keys[idx] = m->keys[i];
        new_vals[idx] = m->values[i];
        new_states[idx] = ABS_HM_OCCUPIED;
    }
    free(m->keys);
    free(m->values);
    free(m->states);
    m->keys = new_keys;
    m->values = new_vals;
    m->states = new_states;
    m->cap = new_cap;
    m->tombstones = 0;
    return 0;
}

static size_t abs_hm_find_key(const abs_hashmap_t *m, const char *key) {
    if (!m || !key || m->cap == 0) return SIZE_MAX;
    size_t idx = (size_t)abs_hash_fnv1a64_str(key) & (m->cap - 1);
    while (m->states[idx] != ABS_HM_EMPTY) {
        if (m->states[idx] == ABS_HM_OCCUPIED && strcmp(m->keys[idx], key) == 0)
            return idx;
        idx = (idx + 1) & (m->cap - 1);
    }
    return SIZE_MAX;
}

abs_hashmap_t *abs_hashmap_create(abs_hashmap_free_fn free_value) {
    abs_hashmap_t *m = (abs_hashmap_t *)calloc(1, sizeof(*m));
    if (!m) return NULL;
    m->free_value = free_value;
    return m;
}

void abs_hashmap_destroy(abs_hashmap_t *m) {
    if (!m) return;
    abs_hashmap_clear(m);
    free(m->keys);
    free(m->values);
    free(m->states);
    free(m);
}

void abs_hashmap_clear(abs_hashmap_t *m) {
    if (!m) return;
    for (size_t i = 0; i < m->cap; i++) {
        if (m->states[i] == ABS_HM_OCCUPIED) {
            if (m->free_value) m->free_value(m->values[i]);
            free(m->keys[i]);
        }
        m->keys[i] = NULL;
        m->values[i] = NULL;
        m->states[i] = ABS_HM_EMPTY;
    }
    m->count = 0;
    m->tombstones = 0;
}

int abs_hashmap_set(abs_hashmap_t *m, const char *key, void *value) {
    if (!m || !key) return -1;
    if (m->cap == 0) {
        if (abs_hm_resize(m, abs_hm_min_cap) != 0) return -1;
    }
    if ((m->count + m->tombstones + 1) * 10 > m->cap * 7) {
        size_t new_cap = m->cap * 2;
        if (new_cap < m->cap) return -1;
        if (abs_hm_resize(m, new_cap) != 0) return -1;
    }
    size_t idx = (size_t)abs_hash_fnv1a64_str(key) & (m->cap - 1);
    size_t tomb = SIZE_MAX;
    while (m->states[idx] != ABS_HM_EMPTY) {
        if (m->states[idx] == ABS_HM_TOMBSTONE) {
            if (tomb == SIZE_MAX) tomb = idx;
        } else if (strcmp(m->keys[idx], key) == 0) {
            if (m->free_value && m->values[idx]) m->free_value(m->values[idx]);
            m->values[idx] = value;
            return 0;
        }
        idx = (idx + 1) & (m->cap - 1);
    }
    if (tomb != SIZE_MAX) idx = tomb;
    char *copy = abs_hm_strdup(key);
    if (!copy) return -1;
    if (m->states[idx] == ABS_HM_TOMBSTONE) m->tombstones--;
    m->count++;
    m->keys[idx] = copy;
    m->values[idx] = value;
    m->states[idx] = ABS_HM_OCCUPIED;
    return 0;
}

void *abs_hashmap_get(const abs_hashmap_t *m, const char *key) {
    size_t idx = abs_hm_find_key(m, key);
    return (idx == SIZE_MAX) ? NULL : m->values[idx];
}

int abs_hashmap_contains(const abs_hashmap_t *m, const char *key) {
    return abs_hm_find_key(m, key) != SIZE_MAX;
}

int abs_hashmap_remove(abs_hashmap_t *m, const char *key) {
    size_t idx = abs_hm_find_key(m, key);
    if (idx == SIZE_MAX) return -1;
    if (m->free_value) m->free_value(m->values[idx]);
    free(m->keys[idx]);
    m->keys[idx] = NULL;
    m->values[idx] = NULL;
    m->states[idx] = ABS_HM_TOMBSTONE;
    m->count--;
    m->tombstones++;
    return 0;
}

size_t abs_hashmap_size(const abs_hashmap_t *m) {
    return m ? m->count : 0;
}

void abs_hashmap_foreach(abs_hashmap_t *m, abs_hashmap_visit_fn fn, void *user) {
    if (!m || !fn) return;
    for (size_t i = 0; i < m->cap; i++) {
        if (m->states[i] == ABS_HM_OCCUPIED) {
            if (fn(m->keys[i], m->values[i], user) != 0) break;
        }
    }
}
