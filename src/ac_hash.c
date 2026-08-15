#include "abscom/ac_hash.h"

#include <string.h>

uint32_t ac_hash_fnv1a32(const void *data, size_t len) {
    const unsigned char *p = (const unsigned char *)data;
    uint32_t h = 2166136261u;
    for (size_t i = 0; i < len; i++) {
        h ^= p[i];
        h *= 16777619u;
    }
    return h;
}

uint64_t ac_hash_fnv1a64(const void *data, size_t len) {
    const unsigned char *p = (const unsigned char *)data;
    uint64_t h = 14695981039346656037ULL;
    for (size_t i = 0; i < len; i++) {
        h ^= p[i];
        h *= 1099511628211ULL;
    }
    return h;
}

uint64_t ac_hash_fnv1a64_str(const char *str) {
    return str ? ac_hash_fnv1a64(str, strlen(str)) : 0;
}

uint32_t ac_hash_djb2(const char *str) {
    uint32_t h = 5381;
    if (!str) return h;
    while (*str) h = ((h << 5) + h) + (unsigned char)*str++;
    return h;
}
