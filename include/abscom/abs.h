#ifndef ABSCOM_ABS_H
#define ABSCOM_ABS_H

#include "abs_common.h"
#include "abs_dynarray.h"
#include "abs_string.h"
#include "abs_hash.h"
#include "abs_hashmap.h"
#include "abs_time.h"
#include "abs_fs.h"

#include <stdio.h>
#include <stdbool.h>

ABS_BEGIN_C_DECLS

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

ABS_API void abs_init(void);
ABS_API void abs_cleanup(void);
ABS_API void del(var obj);

ABS_API var abs_new_int(long v);
ABS_API var abs_new_float(double v);
ABS_API var abs_new_str(const char *v);
ABS_API var abs_new_bool(bool v);
ABS_API var abs_new_none(void);
ABS_API var abs_new_list(void);
ABS_API var abs_new_dict(void);
ABS_API var abs_new_set(void);
ABS_API var abs_new_error(const char *msg);

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

ABS_API void abs_print_impl(const char *end, ...);
#define print(...)      abs_print_impl("\n", __VA_ARGS__, (var)0)
#define print_end(e, ...) abs_print_impl(e, __VA_ARGS__, (var)0)

ABS_API var input(const char *prompt);

ABS_API var add(var a, var b);
ABS_API var sub(var a, var b);
ABS_API var mul(var a, var b);
ABS_API var eq(var a, var b);

ABS_API var to_str(var obj);
ABS_API var to_int(var obj);
ABS_API var to_float(var obj);

ABS_API void dset(var dict, const char *key, var val);
ABS_API var dget(var dict, const char *key);

ABS_API void append(var list, var item);
ABS_API var get(var list, long index);
ABS_API var len(var obj);
ABS_API var range(int start, int stop);
ABS_API var range_step(int start, int stop, int step);
ABS_API var slice(var obj, int start, int stop);

ABS_API var upper(var obj);
ABS_API var lower(var obj);
ABS_API var split(var str_obj, const char *delimiter);
ABS_API var join(var delimiter, var list_obj);

ABS_API var map_func(var list, AbsFunc f);
ABS_API var filter_func(var list, AbsFunc f);

ABS_API var fmt_impl(const char *format, ...);
#define fmt(f, ...) fmt_impl(f, __VA_ARGS__, (var)0)

ABS_API var fopen_safe(const char *filename, const char *mode);
ABS_API var read_file(var file_obj);
ABS_API void write_file(var file_obj, var content);
ABS_API void close_file(var file_obj);

ABS_API var json_parse(const char *json_str);

ABS_API bool is_err(var obj);

ABS_API void random_seed(void);
ABS_API void seed(var obj);
ABS_API var randint(int min, int max);
ABS_API var random_float(void);
ABS_API var uniform(double a, double b);
ABS_API var choice(var seq);
ABS_API var choices(var seq, int k);
ABS_API var sample(var seq, int k);
ABS_API void shuffle(var list);

ABS_API var type(var obj);

ABS_API int compare_objs(var a, var b);

ABS_API var min_val(var list);
ABS_API var max_val(var list);
ABS_API var sum_val(var list);

ABS_API bool is_true(var obj);
ABS_API var not_(var obj);
ABS_API var any(var list);
ABS_API var all(var list);

ABS_API var abs_val(var obj);
ABS_API var pow_val(var base, var exp);
ABS_API var round_val(var obj, int digits);

ABS_API var sorted(var list, bool reverse);
ABS_API var reversed_seq(var list);
ABS_API var zip_lists(var list1, var list2);

ABS_API var strip(var str_obj);
ABS_API var startswith(var str, var prefix);
ABS_API var endswith(var str, var suffix);
ABS_API var count(var container, var item);

ABS_API void sleep_sec(double seconds);
ABS_API var time_now(void);
ABS_API var exec_cmd(const char *cmd);

ABS_API bool is_int(var o);
ABS_API bool is_float(var o);
ABS_API bool is_str(var o);
ABS_API bool is_list(var o);
ABS_API bool is_dict(var o);
ABS_API bool is_set(var o);
ABS_API bool is_none(var o);

ABS_API var Class(const char *name);
ABS_API var New(var cls);
ABS_API void set_attr(var obj, const char *key, var val);
ABS_API var get_attr(var obj, const char *key);

ABS_API var json_dump(var obj);
ABS_API var http_get(const char *url);

ABS_API void set_add(var set_obj, var item);
ABS_API bool set_contains(var set_obj, var item);
ABS_API var set_union(var a, var b);
ABS_API var set_diff(var a, var b);

ABS_API var list_comp(var list, AbsMapFunc mapf, AbsFilterFunc filterf);

ABS_API long get_len_fast(var obj);

#define foreach(VAR, LIST)                                                     \
    for (long _i_##VAR = 0, _len_##VAR = get_len_fast(LIST);                   \
         _i_##VAR < _len_##VAR && (VAR = get(LIST, _i_##VAR));                 \
         _i_##VAR++)

#define str(o) to_str(o)

ABS_END_C_DECLS

#endif
