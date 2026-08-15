#include "abscom/abs.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define ABS_GETCWD(buf, sz) _getcwd((buf), (int)(sz))
#define ABS_PATH_SEPARATOR "\\"
#else
#include <unistd.h>
#define ABS_GETCWD(buf, sz) getcwd((buf), (sz))
#define ABS_PATH_SEPARATOR "/"
#endif

var path_join(var p1, var p2) {
    if (!p1 || !p2 || p1->type != ABS_STR || p2->type != ABS_STR)
        return abs_new_error("path_join expects two strings");
    char buf[4096];
    snprintf(buf, sizeof(buf), "%s%s%s", p1->val.s, ABS_PATH_SEPARATOR, p2->val.s);
    return abs_new_str(buf);
}

var path_exists(var path) {
    if (!path || path->type != ABS_STR) return abs_new_bool(false);
    FILE *f = fopen(path->val.s, "rb");
    if (f) {
        fclose(f);
        return abs_new_bool(true);
    }
    return abs_new_bool(false);
}

var getcwd_val(void) {
    char buf[4096];
    if (ABS_GETCWD(buf, sizeof(buf))) return abs_new_str(buf);
    return abs_new_str("");
}
