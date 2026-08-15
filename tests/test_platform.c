#include "abscom/ac_fs.h"
#include "abscom/ac_time.h"

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
    double w1 = ac_time_wall();
    double w2 = ac_time_wall();
    CHECK(w2 >= w1);
    CHECK(w1 > 1500000000.0);

    double t0 = ac_time_now();
    volatile double acc = 0.0;
    for (int i = 0; i < 1000000; i++) acc += (double)i;
    double elapsed = ac_time_now() - t0;
    AC_UNUSED(acc);
    CHECK(elapsed >= 0.0);
    CHECK(elapsed < 60.0);
    CHECK(ac_time_now_ns() > 0);
    CHECK(ac_time_wall_ms() > 0);

    const char *path = "ac_fs_test.txt";
    const char *data = "hello abscom file\nsecond line";
    size_t n = strlen(data);
    CHECK(ac_fs_write_file(path, data, n) == 0);
    CHECK(ac_fs_exists(path));

    {
        char *out = NULL;
        size_t out_n = 0;
        CHECK(ac_fs_read_file(path, &out, &out_n) == 0);
        CHECK(out != NULL);
        CHECK(out_n == n);
        CHECK(memcmp(out, data, n) == 0);
        CHECK(out[n] == '\0');
        free(out);
    }

    const char *path2 = "ac_fs_test2.txt";
    CHECK(ac_fs_rename(path, path2) == 0);
    CHECK(!ac_fs_exists(path));
    CHECK(ac_fs_exists(path2));
    CHECK(ac_fs_remove(path2) == 0);
    CHECK(!ac_fs_exists(path2));
    CHECK(ac_fs_remove(path2) == -1);

    printf("test_platform: OK\n");
    return 0;
}
