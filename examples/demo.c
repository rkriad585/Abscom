#include "abscom/abs.h"

#include <stdio.h>
#include <stdlib.h>

static int count_visit(const char *key, void *value, void *user) {
    ABS_UNUSED(key);
    ABS_UNUSED(value);
    (*(size_t *)user)++;
    return 0;
}

int main(void) {
    abs_string_t s;
    if (abs_string_init(&s) != 0) return 1;
    abs_string_append_cstr(&s, "Hello, ");
    abs_string_append_fmt(&s, "world %d!", 42);
    printf("string: %s (len=%zu)\n", abs_string_c_str(&s), abs_string_len(&s));
    abs_string_destroy(&s);

    abs_dynarray_t nums;
    if (abs_dynarray_init(&nums, sizeof(int)) != 0) return 1;
    for (int i = 0; i < 5; i++) abs_dynarray_push(&nums, &i);
    printf("dynarray (%zu):", abs_dynarray_len(&nums));
    for (size_t i = 0; i < abs_dynarray_len(&nums); i++)
        printf(" %d", *(int *)abs_dynarray_at(&nums, i));
    printf("\n");
    abs_dynarray_destroy(&nums);

    abs_hashmap_t *m = abs_hashmap_create(NULL);
    abs_hashmap_set(m, "alpha", (void *)(intptr_t)1);
    abs_hashmap_set(m, "beta", (void *)(intptr_t)2);
    size_t n = 0;
    abs_hashmap_foreach(m, count_visit, &n);
    printf("hashmap: size=%zu visited=%zu alpha=%ld beta=%ld\n",
           abs_hashmap_size(m), n,
           (long)(intptr_t)abs_hashmap_get(m, "alpha"),
           (long)(intptr_t)abs_hashmap_get(m, "beta"));
    abs_hashmap_destroy(m);

    printf("hash: fnv1a64(\"hello\")=%llu djb2(\"hello\")=%u\n",
           (unsigned long long)abs_hash_fnv1a64_str("hello"),
           abs_hash_djb2("hello"));

    double t0 = abs_time_now();
    printf("time: wall=%.3f monotonic_elapsed=%.6f\n",
           abs_time_wall(), abs_time_now() - t0);

    const char *path = "demo_tmp.txt";
    if (abs_fs_write_file(path, "abscom", 6) == 0) {
        char *data = NULL;
        size_t sz = 0;
        if (abs_fs_read_file(path, &data, &sz) == 0) {
            printf("fs: \"%s\" (%zu bytes)\n", data, sz);
            free(data);
        }
        abs_fs_remove(path);
    }

    printf("demo finished\n");
    return 0;
}
