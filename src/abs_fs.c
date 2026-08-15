#include "abscom/abs_fs.h"

#include <stdio.h>
#include <stdlib.h>

int abs_fs_exists(const char *path) {
    if (!path) return 0;
    FILE *f = fopen(path, "rb");
    if (f) {
        fclose(f);
        return 1;
    }
    return 0;
}

int abs_fs_read_file(const char *path, char **out_data, size_t *out_size) {
    if (!path || !out_data) return -1;
    if (out_size) *out_size = 0;
    *out_data = NULL;
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return -1;
    }
    long sz = ftell(f);
    if (sz < 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return -1;
    }
    size_t n = (size_t)sz;
    char *buf = (char *)malloc(n + 1);
    if (!buf) {
        fclose(f);
        return -1;
    }
    if (n > 0 && fread(buf, 1, n, f) != n) {
        free(buf);
        fclose(f);
        return -1;
    }
    buf[n] = '\0';
    fclose(f);
    *out_data = buf;
    if (out_size) *out_size = n;
    return 0;
}

int abs_fs_write_file(const char *path, const void *data, size_t size) {
    if (!path || (!data && size > 0)) return -1;
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    if (size > 0 && fwrite(data, 1, size, f) != size) {
        fclose(f);
        return -1;
    }
    if (fclose(f) != 0) return -1;
    return 0;
}

int abs_fs_remove(const char *path) {
    return (path && remove(path) == 0) ? 0 : -1;
}

int abs_fs_rename(const char *from, const char *to) {
    return (from && to && rename(from, to) == 0) ? 0 : -1;
}
