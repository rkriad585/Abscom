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
#include <stdint.h>
#include <time.h>
#include <setjmp.h>

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
    ABS_INSTANCE,
    ABS_MATRIX,
    ABS_THREAD,
    ABS_TIME,       /* datetime object */
    ABS_GENERATOR,  /* stateful iterator */
    ABS_SERVER,     /* web server object */
    ABS_LIB,        /* dynamically loaded library */
    ABS_FUNC,       /* wrapped C function / decorator / memoized callable */
    ABS_ITERATOR    /* itertools chain/cycle state machine */
} AbsType;

typedef struct AbsObj AbsObj;
typedef AbsObj *var;
typedef var (*AbsThreadFunc)(var);
typedef var (*AbsGenFunc)(var);
typedef var (*AbsFunc)(var); /* standard signature: var -> var */

/* Opaque handle to a shared library: HMODULE on Windows, void* on POSIX. */
typedef void *LibHandle;

/* Socket handle used by the WebSocket API: SOCKET on Windows, int on POSIX. */
typedef intptr_t abs_socket;

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
        struct {
            int rows;
            int cols;
            double *data;
        } matrix;
        struct {
            void *handle; /* Windows HANDLE or malloc'd pthread_t * on POSIX */
            AbsThreadFunc func;
            AbsObj *arg;
            AbsObj *result;
        } thread;
        struct {
            struct tm tm_val;
        } time_data;
        struct {
            AbsGenFunc func;
            long state;
            long limit;
            long step;
        } gen;
        struct {
            int port;
            intptr_t socket_fd; /* SOCKET on Windows, int fd on POSIX */
            AbsObj *routes;     /* ABS_DICT: path -> ABS_FUNC */
        } server;
        struct {
            LibHandle handle;
            char *path;
        } lib;
        struct {
            AbsFunc func_ptr;
            AbsObj *cache;    /* ABS_DICT used by memoize(); NULL otherwise */
            AbsObj *metadata; /* original function kept by decorate() */
        } func;
        struct {
            AbsObj *source_a;
            AbsObj *source_b; /* NULL for cycle() and repeat() */
            long index;
            int mode;         /* 0=chain, 1=cycle, 2=repeat */
            long limit;       /* repeat() emission count; unused otherwise */
        } iter;
        FILE *file_ptr;
        char *error_msg;
    } val;
    struct AbsObj *next;
} AbsObj;

typedef AbsObj *var;
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
ABS_API var abs_new_file(FILE *f);

/* Low-level object helpers used by the runtime and the scientific modules. */
ABS_API var abs_new_obj(AbsType type);
ABS_API void abs_gc_track(var obj);
ABS_API double abs_num_val(var obj);

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

/* --- Scientific layer: matrices, statistics, math, CSV, paths, threads --- */

ABS_API var abs_matrix_new(int rows, int cols);
ABS_API var abs_matrix_eye(int n);
ABS_API int abs_matrix_rows(var m);
ABS_API int abs_matrix_cols(var m);
ABS_API void abs_matrix_set(var m, int r, int c, double val);
ABS_API double abs_matrix_get(var m, int r, int c);
ABS_API var abs_matrix_mul(var A, var B);
ABS_API var abs_matrix_transpose(var m);
ABS_API var abs_matrix_det(var m);
ABS_API void abs_matrix_print(var m);

ABS_API var abs_stats_mean(var list);
ABS_API var abs_stats_median(var list);
ABS_API var abs_stats_mode(var list);
ABS_API var abs_stats_variance(var list);
ABS_API var abs_stats_stdev(var list);

ABS_API var sin_val(var x);
ABS_API var cos_val(var x);
ABS_API var tan_val(var x);
ABS_API var log_val(var x);
ABS_API var log10_val(var x);
ABS_API var sqrt_val(var x);
ABS_API var deg2rad(var x);

ABS_API var factorial(var n);
ABS_API var nCr(var n, var r);
ABS_API var nPr(var n, var r);

ABS_API var path_join(var p1, var p2);
ABS_API var path_exists(var path);
ABS_API var getcwd_val(void);

ABS_API var csv_read(const char *filename);
ABS_API void csv_write(const char *filename, var list_of_lists);

ABS_API var thread_start(AbsThreadFunc func, var arg);
ABS_API var thread_join(var thread_obj);

/* --- Language features: exceptions, regex, datetime, generators, encoding --- */

/* Exception handling: try / catch / end_try + throw. The jmp_buf stack and the
 * last thrown error live here so nested try blocks work. */
#define ABS_ENV_STACK_SIZE 10

ABS_API extern jmp_buf abs_env_stack[ABS_ENV_STACK_SIZE];
ABS_API extern int abs_env_idx;
ABS_API extern var abs_last_error;

/* Generic resource cleanup used by the with() context-manager macro. */
ABS_API void close_resource(var obj);

#ifndef __cplusplus
/* C++ reserves try/catch/throw, so the control-flow API is C-only. */
ABS_API jmp_buf *abs_push_jmp(void);
ABS_API void abs_pop_jmp(void);
ABS_API void throw(const char *msg);

#define try if (setjmp(*abs_push_jmp()) == 0)
#define catch(VAR) else if ((VAR = abs_last_error, 1))
#define end_try abs_pop_jmp()

/* Runs INIT, executes the body exactly once, then calls close_resource(). */
#define with(VAR, INIT)                                                        \
    for (var VAR = (INIT), _once_##VAR = (var)1; _once_##VAR;                  \
         _once_##VAR = NULL, close_resource(VAR))
#endif

/* Regex: a small matcher supporting . (any), * (closure), ^ and $ anchors. */
ABS_API bool re_match(var pattern, var text);
ABS_API var re_findall(var pattern, var text);
ABS_API var re_sub(var pattern, var repl, var text);

/* Date & time. */
ABS_API var datetime_now(void);
ABS_API var strftime_val(const char *fmt, var time_obj);
ABS_API var timedelta(int days, int seconds);

/* Generators. */
ABS_API var range_gen(long start, long stop, long step);
ABS_API var next(var gen);

/* Encoding and environment. */
ABS_API var base64_encode(var str_obj);
ABS_API var uuid4(void);
ABS_API var os_getenv(const char *key);
ABS_API void os_setenv(const char *key, const char *val);

/* --- Framework layer: web server, events, plugins, functions, introspection --- */

/* 1. Micro web server (synchronous HTTP/1.1 over raw sockets). */
ABS_API var Server(int port);
ABS_API void route(var app, const char *path, AbsFunc handler);
ABS_API void server_run(var app);
/* Dispatch one request line ("GET /path HTTP/1.1"); returns the body var or
 * NULL when no route matches. Used by server_run() and by the tests. */
ABS_API var server_handle(var app, const char *request_line);

/* 2. Event emitter: a dict of event name -> list of ABS_FUNC handlers. */
ABS_API var EventBus(void);
ABS_API void on(var bus, const char *event_name, AbsFunc handler);
ABS_API void emit(var bus, const char *event_name, var data);

/* 3. Dynamic plugins (LoadLibrary/dlopen). */
ABS_API var load_library(const char *path);
ABS_API var call_lib_func(var lib, const char *func_name, var arg);

/* 4. Function objects, decorators and memoization. */
ABS_API var make_func(AbsFunc f);
ABS_API var call_func(var func_obj, var arg);
ABS_API var call_memoized(var func_obj, var arg);
ABS_API var decorate(var func_obj, AbsFunc wrapper_logic);
ABS_API var func_meta(var func_obj);
ABS_API var memoize(AbsFunc f);

/* 5. Introspection. */
ABS_API var dir(var obj);
ABS_API var id(var obj);
ABS_API var repr(var obj);

/* 6. Itertools: chain / cycle iterators. */
ABS_API var chain(var list_a, var list_b);
ABS_API var cycle(var list);
ABS_API var iter_next(var iter);

/* --- Algorithm suite: sorting, benchmarking, binary search --- */

/* Visualizer callback: called with the list and the two swapped indices. */
typedef void (*AbsSortVis)(var list, int idx_a, int idx_b);

/* Copy a list. Primitive elements (ints, floats, strings, bools) are copied;
 * nested containers are shared. */
ABS_API var list_copy(var list);
ABS_API bool is_sorted(var list);

/* O(n^2) - educational. */
ABS_API void sort_bubble(var list);
ABS_API void sort_selection(var list);
ABS_API void sort_insertion(var list);

/* O(n log n) - efficient. */
ABS_API void sort_shell(var list);
ABS_API void sort_heap(var list);
ABS_API void sort_merge(var list);
ABS_API void sort_quick(var list);

/* Linear / integer-only. */
ABS_API void sort_counting(var list);
ABS_API void sort_radix(var list);
ABS_API void sort_bucket(var list);

/* Novelty / standard library. */
ABS_API void sort_bogo(var list);
ABS_API void sort_c_qsort(var list);

/* Bubble sort that invokes vis_func on every swap. */
ABS_API void sort_bubble_visual(var list, AbsSortVis vis_func);

/* Time sort_func on a copy of list, returning seconds. */
ABS_API double timeit(void (*sort_func)(var), var list);

/* Binary search on a sorted list; returns the index or -1. */
ABS_API long binary_search(var sorted_list, var target);

/* --- Realtime & crypto layer: WebSockets, hashing, shuffling, repeat --- */

/* SHA-256 (hex) and keyed HMAC-SHA-256 (hex), returned as ABS_STR values. */
ABS_API var sha256(const char *input);
ABS_API var hmac_sha256(const char *key, const char *msg);

/* Server-side WebSocket handshake on an accepted client socket: parses the
 * Sec-WebSocket-Key out of the HTTP request, sends back the 101 response, and
 * returns whether the handshake succeeded. */
ABS_API bool ws_accept(abs_socket client_fd, const char *request_str);
/* RFC 6455: base64(SHA1(key + magic GUID)) — the Sec-WebSocket-Accept value. */
ABS_API var ws_compute_accept(const char *key);
/* Encode msg into a server->client text frame at out; returns total bytes. */
ABS_API size_t ws_encode_frame(char *out, size_t cap, const char *msg);
/* Decode one text frame (masked client frame or unmasked server frame) into
 * out; returns the payload length, or -1 when the frame is malformed. */
ABS_API long ws_decode_frame(const char *buf, size_t buf_len, char *out,
                             size_t out_cap);
/* Send msg to fd as a text frame, and receive one frame as an ABS_STR. */
ABS_API void ws_send(abs_socket fd, const char *msg);
ABS_API var ws_recv(abs_socket fd);

/* Shuffling: fisher_yates() is shuffle() under its classic name; riffle_shuffle
 * splits the deck in half and randomly interleaves the two piles. */
ABS_API void fisher_yates(var list);
ABS_API void riffle_shuffle(var list);

/* Itertools: repeat(val, n) yields val n times, then None. */
ABS_API var repeat(var val, int n);

ABS_END_C_DECLS

#endif
