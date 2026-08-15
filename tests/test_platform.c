#include "abscom/abs_fs.h"
#include "abscom/abs_time.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);  \
            return 1;                                                        \
        }                                                                    \
    } while (0)

int main(void) {
    double w1 = abs_time_wall();
    double w2 = abs_time_wall();
    CHECK(w2 >= w1);
    CHECK(w1 > 1500000000.0);

    double t0 = abs_time_now();
    volatile double acc = 0.0;
    for (int i = 0; i < 1000000; i++) acc += (double)i;
    double elapsed = abs_time_now() - t0;
    ABS_UNUSED(acc);
    CHECK(elapsed >= 0.0);
    CHECK(elapsed < 60.0);
    CHECK(abs_time_now_ns() > 0);
    CHECK(abs_time_wall_ms() > 0);

    const char *path = "abs_fs_test.txt";
    const char *data = "hello abscom file\nsecond line";
    size_t n = strlen(data);
    CHECK(abs_fs_write_file(path, data, n) == 0);
    CHECK(abs_fs_exists(path));

    {
        char *out = NULL;
        size_t out_n = 0;
        CHECK(abs_fs_read_file(path, &out, &out_n) == 0);
        CHECK(out != NULL);
        CHECK(out_n == n);
        CHECK(memcmp(out, data, n) == 0);
        CHECK(out[n] == '\0');
        free(out);
    }

    const char *path2 = "abs_fs_test2.txt";
    CHECK(abs_fs_rename(path, path2) == 0);
    CHECK(!abs_fs_exists(path));
    CHECK(abs_fs_exists(path2));
    CHECK(abs_fs_remove(path2) == 0);
    CHECK(!abs_fs_exists(path2));
    CHECK(abs_fs_remove(path2) == -1);

    printf("test_platform: OK\n");
    return 0;
}
