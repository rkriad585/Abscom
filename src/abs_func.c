/* Function objects (ABS_FUNC): wrap C functions into first-class var values,
 * add memoization, and support a simple decorator mechanism.
 *
 *   make_func(f)     -> ABS_FUNC object; call it via call_func(f, arg)
 *   memoize(f)       -> ABS_FUNC with a cache dict; call_memoized(f, arg)
 *   decorate(f, w)   -> ABS_FUNC whose body is w(arg); the original f is kept
 *                       in metadata and reachable through func_meta(f).
 */

#include "abscom/abs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

var make_func(AbsFunc f) {
    if (!f) return abs_new_error("make_func requires a function");
    var o = abs_new_obj(ABS_FUNC);
    if (!o) return abs_new_error("Out of memory");
    o->val.func.func_ptr = f;
    o->val.func.cache = NULL;
    o->val.func.metadata = NULL;
    abs_gc_track(o);
    return o;
}

var call_func(var func_obj, var arg) {
    if (!func_obj || func_obj->type != ABS_FUNC)
        return abs_new_error("Not a function");
    if (func_obj->val.func.cache) return call_memoized(func_obj, arg);
    return func_obj->val.func.func_ptr(arg);
}

var call_memoized(var func_obj, var arg) {
    if (!func_obj || func_obj->type != ABS_FUNC || !func_obj->val.func.cache)
        return abs_new_error("Not a memoized function");
    if (!arg) arg = None;
    char key[64];
    if (arg->type == ABS_INT) {
        snprintf(key, sizeof(key), "%ld", arg->val.i);
    } else if (arg->type == ABS_STR) {
        snprintf(key, sizeof(key), "%s", arg->val.s);
    } else {
        snprintf(key, sizeof(key), "ptr_%p", (void *)arg);
    }
    var cached = dget(func_obj->val.func.cache, key);
    if (cached->type != ABS_NONE) return cached;
    var result = func_obj->val.func.func_ptr(arg);
    dset(func_obj->val.func.cache, key, result);
    return result;
}

var memoize(AbsFunc f) {
    var o = make_func(f);
    if (!o || o->type == ABS_ERROR) return o;
    o->val.func.cache = abs_new_dict();
    return o;
}

var decorate(var func_obj, AbsFunc wrapper_logic) {
    if (!func_obj || func_obj->type != ABS_FUNC)
        return abs_new_error("Not a function");
    if (!wrapper_logic) return func_obj;
    var o = make_func(wrapper_logic);
    if (!o) return abs_new_error("Out of memory");
    o->val.func.metadata = func_obj;
    return o;
}

var func_meta(var func_obj) {
    if (!func_obj || func_obj->type != ABS_FUNC) return None;
    return func_obj->val.func.metadata ? func_obj->val.func.metadata : None;
}
