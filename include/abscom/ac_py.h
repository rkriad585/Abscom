#ifndef ABSCOM_AC_PY_H
#define ABSCOM_AC_PY_H

#include "ac_common.h"

#include <stdio.h>
#include <stdbool.h>

AC_BEGIN_C_DECLS

typedef enum {
    ABS_INT,
    ABS_FLOAT,
    ABS_STR,
    ABS_BOOL,
    ABS_LIST,
    ABS_DICT,
    ABS_FILE,
    ABS_ERROR,
    ABS_NONE,
    ABS_SET,
    ABS_CLASS,
    ABS_INSTANCE
} AbsType;

typedef struct AbsObj AbsObj;

typedef struct DictNode {
    char *key;
    AbsObj *value;
    struct DictNode *next;
} DictNode;

typedef struct AbsObj {
    AbsType type;
    union {
        long   i;
        double f;
        char  *s;
        bool   b;
        struct {
            AbsObj **items;
            size_t size;
            size_t capacity;
        } list;
        struct {
            DictNode **buckets;
            size_t capacity;
            size_t count;
        } dict;
        struct {
            char *name;
            AbsObj *methods_dict;
        } cls;
        struct {
            AbsObj *cls_ptr;
            AbsObj *attr_dict;
        } inst;
        FILE *file_ptr;
        char *error_msg;
    } val;
    struct AbsObj *next;
} AbsObj;

typedef AbsObj *var;
typedef var (*AbsFunc)(var);
typedef var (*AbsMapFunc)(var);
typedef bool (*AbsFilterFunc)(var);

AC_API void abs_init(void);
AC_API void abs_cleanup(void);
AC_API void del(var obj);

AC_API var abs_new_int(long v);
AC_API var abs_new_float(double v);
AC_API var abs_new_str(const char *v);
AC_API var abs_new_bool(bool v);
AC_API var abs_new_none(void);
AC_API var abs_new_list(void);
AC_API var abs_new_dict(void);
AC_API var abs_new_set(void);
AC_API var abs_new_error(const char *msg);

#define v(X) _Generic((X),                                             \
    int: abs_new_int,                                                  \
    long: abs_new_int,                                                 \
    double: abs_new_float,                                             \
    float: abs_new_float,                                              \
    char *: abs_new_str,                                               \
    const char *: abs_new_str,                                         \
    bool: abs_new_bool                                                 \
)(X)

#define None    abs_new_none()
#define True    ((bool)1)
#define False   ((bool)0)
#define List()  abs_new_list()
#define Dict()  abs_new_dict()
#define Set()   abs_new_set()

AC_API void abs_print_impl(const char *end, ...);
#define print(...)      abs_print_impl("\n", __VA_ARGS__, (var)0)
#define print_end(e, ...) abs_print_impl(e, __VA_ARGS__, (var)0)

AC_API var input(const char *prompt);

AC_API var add(var a, var b);
AC_API var sub(var a, var b);
AC_API var mul(var a, var b);
AC_API var eq(var a, var b);

AC_API var to_str(var obj);
AC_API var to_int(var obj);
AC_API var to_float(var obj);

AC_API void dset(var dict, const char *key, var val);
AC_API var dget(var dict, const char *key);

AC_API void append(var list, var item);
AC_API var get(var list, long index);
AC_API var len(var obj);
AC_API var range(int start, int stop);
AC_API var range_step(int start, int stop, int step);
AC_API var slice(var obj, int start, int stop);

AC_API var upper(var obj);
AC_API var lower(var obj);
AC_API var split(var str_obj, const char *delimiter);
AC_API var join(var delimiter, var list_obj);

AC_API var map_func(var list, AbsFunc f);
AC_API var filter_func(var list, AbsFunc f);

AC_API var fmt_impl(const char *format, ...);
#define fmt(f, ...) fmt_impl(f, __VA_ARGS__, (var)0)

AC_API var fopen_safe(const char *filename, const char *mode);
AC_API var read_file(var file_obj);
AC_API void write_file(var file_obj, var content);
AC_API void close_file(var file_obj);

AC_API var json_parse(const char *json_str);

AC_API bool is_err(var obj);

AC_API void random_seed(void);
AC_API void seed(var obj);
AC_API var randint(int min, int max);
AC_API var random_float(void);
AC_API var uniform(double a, double b);
AC_API var choice(var seq);
AC_API var choices(var seq, int k);
AC_API var sample(var seq, int k);
AC_API void shuffle(var list);

AC_API var type(var obj);

AC_API int compare_objs(var a, var b);

AC_API var min_val(var list);
AC_API var max_val(var list);
AC_API var sum_val(var list);

AC_API bool is_true(var obj);
AC_API var not_(var obj);
AC_API var any(var list);
AC_API var all(var list);

AC_API var abs_val(var obj);
AC_API var pow_val(var base, var exp);
AC_API var round_val(var obj, int digits);

AC_API var sorted(var list, bool reverse);
AC_API var reversed_seq(var list);
AC_API var zip_lists(var list1, var list2);

AC_API var strip(var str_obj);
AC_API var startswith(var str, var prefix);
AC_API var endswith(var str, var suffix);
AC_API var count(var container, var item);

AC_API void sleep_sec(double seconds);
AC_API var time_now(void);
AC_API var exec_cmd(const char *cmd);

AC_API bool is_int(var o);
AC_API bool is_float(var o);
AC_API bool is_str(var o);
AC_API bool is_list(var o);
AC_API bool is_dict(var o);
AC_API bool is_set(var o);
AC_API bool is_none(var o);

AC_API var Class(const char *name);
AC_API var New(var cls);
AC_API void set_attr(var obj, const char *key, var val);
AC_API var get_attr(var obj, const char *key);

AC_API var json_dump(var obj);
AC_API var http_get(const char *url);

AC_API void set_add(var set_obj, var item);
AC_API bool set_contains(var set_obj, var item);
AC_API var set_union(var a, var b);
AC_API var set_diff(var a, var b);

AC_API var list_comp(var list, AbsMapFunc mapf, AbsFilterFunc filterf);

AC_API long get_len_fast(var obj);

#define foreach(VAR, LIST)                                                     \
    for (long _i_##VAR = 0, _len_##VAR = get_len_fast(LIST);                   \
         _i_##VAR < _len_##VAR && (VAR = get(LIST, _i_##VAR));                 \
         _i_##VAR++)

#define str(o) to_str(o)

AC_END_C_DECLS

#endif
