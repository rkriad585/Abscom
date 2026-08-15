/* Dynamic plugins: load a shared library (.so/.dylib/.dll) at runtime and call
 * exported functions with the standard AbsFunc signature (var (*)(var)).
 *
 * Windows uses LoadLibrary/GetProcAddress; POSIX uses dlopen/dlsym (link -ldl
 * on Linux, or rely on libSystem on macOS where dlopen is always available).
 */

#include "abscom/abs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

static char *dup_str(const char *s) {
    if (!s) s = "";
    size_t n = strlen(s) + 1;
    char *copy = (char *)malloc(n);
    if (copy) memcpy(copy, s, n);
    return copy;
}

var load_library(const char *path) {
    if (!path) return abs_new_error("No library path");
    var lib = abs_new_obj(ABS_LIB);
    if (!lib) return abs_new_error("Out of memory");
    lib->val.lib.handle = NULL;
    lib->val.lib.path = dup_str(path);
#ifdef _WIN32
    lib->val.lib.handle = (LibHandle)LoadLibrary(path);
#else
    lib->val.lib.handle = dlopen(path, RTLD_LAZY);
#endif
    if (!lib->val.lib.handle) {
        const char *detail = "";
#ifdef _WIN32
        (void)detail;
#else
        const char *err = dlerror();
        if (err) detail = err;
#endif
        char msg[512];
        snprintf(msg, sizeof(msg), "Could not load library: %s", detail);
        return abs_new_error(msg);
    }
    abs_gc_track(lib);
    return lib;
}

var call_lib_func(var lib, const char *func_name, var arg) {
    if (!lib || lib->type != ABS_LIB || !func_name) return None;
    if (!lib->val.lib.handle) return abs_new_error("Library not loaded");
    AbsFunc f_ptr;
#ifdef _WIN32
    f_ptr = (AbsFunc)GetProcAddress((HMODULE)lib->val.lib.handle, func_name);
#else
    f_ptr = (AbsFunc)dlsym(lib->val.lib.handle, func_name);
#endif
    if (!f_ptr) return abs_new_error("Function not found in library");
    return f_ptr(arg);
}
