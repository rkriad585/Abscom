#include "abscom/abs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

var os_getenv(const char *key) {
    if (!key) return None;
    char *val = getenv(key);
    return val ? abs_new_str(val) : None;
}

void os_setenv(const char *key, const char *val) {
    if (!key || !val) return;
#ifdef _WIN32
    /* putenv() keeps the pointer, so hand it a heap buffer. */
    size_t n = strlen(key) + strlen(val) + 2;
    char *buf = (char *)malloc(n);
    if (!buf) return;
    sprintf(buf, "%s=%s", key, val);
    putenv(buf);
#else
    setenv(key, val, 1);
#endif
}
