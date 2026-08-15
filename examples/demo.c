#include "abscom/ac.h"

#include <stdio.h>
#include <stdlib.h>

static int count_visit(const char *key, void *value, void *user) {
    AC_UNUSED(key);
    AC_UNUSED(value);
    (*(size_t *)user)++;
    return 0;
}

int main(void) {
    ac_string_t s;
    if (ac_string_init(&s) != 0) return 1;
    ac_string_append_cstr(&s, "Hello, ");
    ac_string_append_fmt(&s, "world %d!", 42);
    printf("string: %s (len=%zu)\n", ac_string_c_str(&s), ac_string_len(&s));
    ac_string_destroy(&s);

    ac_dynarray_t nums;
    if (ac_dynarray_init(&nums, sizeof(int)) != 0) return 1;
    for (int i = 0; i < 5; i++) ac_dynarray_push(&nums, &i);
    printf("dynarray (%zu):", ac_dynarray_len(&nums));
    for (size_t i = 0; i < ac_dynarray_len(&nums); i++)
        printf(" %d", *(int *)ac_dynarray_at(&nums, i));
    printf("\n");
    ac_dynarray_destroy(&nums);

    ac_hashmap_t *m = ac_hashmap_create(NULL);
    ac_hashmap_set(m, "alpha", (void *)(intptr_t)1);
    ac_hashmap_set(m, "beta", (void *)(intptr_t)2);
    size_t n = 0;
    ac_hashmap_foreach(m, count_visit, &n);
    printf("hashmap: size=%zu visited=%zu alpha=%ld beta=%ld\n",
           ac_hashmap_size(m), n,
           (long)(intptr_t)ac_hashmap_get(m, "alpha"),
           (long)(intptr_t)ac_hashmap_get(m, "beta"));
    ac_hashmap_destroy(m);

    printf("hash: fnv1a64(\"hello\")=%llu djb2(\"hello\")=%u\n",
           (unsigned long long)ac_hash_fnv1a64_str("hello"),
           ac_hash_djb2("hello"));

    double t0 = ac_time_now();
    printf("time: wall=%.3f monotonic_elapsed=%.6f\n",
           ac_time_wall(), ac_time_now() - t0);

    const char *path = "demo_tmp.txt";
    if (ac_fs_write_file(path, "abscom", 6) == 0) {
        char *data = NULL;
        size_t sz = 0;
        if (ac_fs_read_file(path, &data, &sz) == 0) {
            printf("fs: \"%s\" (%zu bytes)\n", data, sz);
            free(data);
        }
        ac_fs_remove(path);
    }

    printf("demo finished\n");
    return 0;
}
