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
/* Decorator wrapper: receives the original target function plus the call
 * argument, so it can run pre/post logic around the target (Python-style
 * @decorator wrapping). Call the target with call_func(target, args). */
typedef var (*AbsWrapperFunc)(var target, var args);

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
            AbsFunc func_ptr;        /* body for plain wrapped functions */
            AbsWrapperFunc wrap_ptr; /* target-aware decorator body (decorate_func) */
            char *name;              /* optional display name (def) */
            AbsObj *cache;           /* ABS_DICT used by memoize(); NULL otherwise */
            AbsObj *metadata;        /* original function kept by decorate() */
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

/* Mathematical constants. */
#define ABS_PI      3.14159265358979323846
#define ABS_E       2.71828182845904523536
#define ABS_SQRT2   1.41421356237309504880
#define ABS_PHI     1.61803398874989484820
#define ABS_EPSILON 1e-9

ABS_API var abs_matrix_new(int rows, int cols);
ABS_API var abs_matrix_eye(int n);
/* Uniformly random matrix with entries in [-1, 1] (for weight initialization). */
ABS_API var abs_matrix_random(int rows, int cols);
ABS_API int abs_matrix_rows(var m);
ABS_API int abs_matrix_cols(var m);
ABS_API void abs_matrix_set(var m, int r, int c, double val);
ABS_API double abs_matrix_get(var m, int r, int c);
ABS_API var abs_matrix_mul(var A, var B);
ABS_API var abs_matrix_add(var A, var B);
ABS_API var abs_matrix_sub(var A, var B);
/* Hadamard (element-wise) product; ABS_ERROR on dimension mismatch. */
ABS_API var abs_matrix_mul_element(var A, var B);
ABS_API var abs_matrix_scale(var m, double s);
ABS_API var abs_matrix_add_scalar(var m, double s);
/* Apply func to every element in place (e.g. abs_act_relu). */
ABS_API void abs_matrix_apply(var m, double (*func)(double));
/* Deep copy of a matrix. */
ABS_API var abs_matrix_copy(var m);
/* Broadcasting: add the 1 x cols row vector v to every row of m in place
 * (used to fold bias terms into a batched activation). */
ABS_API void abs_matrix_add_row_vector(var m, var v);
ABS_API var abs_matrix_sum(var m);
ABS_API var abs_matrix_mean(var m);
ABS_API var abs_matrix_min(var m);
ABS_API var abs_matrix_max(var m);
/* Flat index of the largest element; -1 for non-matrices. */
ABS_API long abs_matrix_argmax(var m);
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

/* --- AI/ML layer: activations, loss, and numerical gradient --- */

/* Element-wise activation functions. They take and return plain doubles so
 * they plug straight into abs_matrix_apply(). */
ABS_API double abs_act_sigmoid(double x);
ABS_API double abs_act_relu(double x);
ABS_API double abs_act_tanh(double x);
/* Derivatives of the activations above, given the *activated* output y
 * (used during backpropagation). */
ABS_API double abs_diff_sigmoid(double y);
ABS_API double abs_diff_relu(double y);
ABS_API double abs_diff_tanh(double y);
/* Copy m, then apply func to every element of the copy. */
ABS_API var abs_matrix_apply_deriv(var m, double (*func)(double));
/* Row-wise softmax applied to a matrix in place. */
ABS_API void abs_matrix_softmax(var m);
/* Mean squared error between two matrices; returns a float var. */
ABS_API var abs_loss_mse(var y_true, var y_pred);
/* Classification accuracy: fraction of rows where the argmax column of
 * y_true matches y_pred; returns a float var. */
ABS_API var abs_accuracy(var y_true, var y_pred);
/* Central-difference numerical gradient: (f(x+h) - f(x-h)) / 2h. */
ABS_API var abs_grad(AbsFunc f, var x);

/* --- General mathematics: scalar helpers, number theory, geometry, calculus --- */

/* Scalar utilities. */
ABS_API double abs_sq(double x);   /* x*x */
ABS_API double abs_cb(double x);   /* x*x*x */
ABS_API double abs_clamp(double x, double min, double max);
ABS_API double abs_lerp(double a, double b, double t);
/* Approximate float equality within ABS_EPSILON. */
ABS_API int abs_eq(double a, double b);

/* Number theory and discrete math. */
ABS_API long abs_gcd(long a, long b);
ABS_API long abs_lcm(long a, long b);
ABS_API long abs_factorial(int n);
ABS_API int abs_is_prime(long n);
ABS_API long abs_fibonacci(int n);

/* Combinatorics on plain C ints (the var versions are nCr/nPr). */
ABS_API long abs_nPr(int n, int r);
ABS_API long abs_nCr(int n, int r);

/* Geometry. */
ABS_API double abs_rad2deg(double rad);
ABS_API double abs_hypot(double a, double b);
ABS_API double abs_dist_euclidean(double x1, double y1, double x2, double y2);
ABS_API double abs_dist_manhattan(double x1, double y1, double x2, double y2);

/* Numerical analysis: Newton-Raphson root finder for f(x) = 0. Pass the
 * derivative, or NULL to fall back on a finite-difference approximation. */
ABS_API double abs_root_find(double (*f)(double), double (*f_prime)(double),
                             double guess);

/* Statistics over raw C double arrays (the var-list versions are abs_stats_*). */
ABS_API double abs_stat_mean(double *arr, int size);
ABS_API double abs_stat_median(double *arr, int size);
ABS_API double abs_stat_variance(double *arr, int size);
ABS_API double abs_stat_stddev(double *arr, int size);

/* --- Complex numbers (plain value type, not a var object) --- */

typedef struct {
    double real;
    double imag;
} AbsComplex;

ABS_API AbsComplex abs_c_add(AbsComplex a, AbsComplex b);
ABS_API AbsComplex abs_c_sub(AbsComplex a, AbsComplex b);
ABS_API AbsComplex abs_c_mul(AbsComplex a, AbsComplex b);
ABS_API double abs_c_mag(AbsComplex a);
ABS_API AbsComplex abs_c_conj(AbsComplex a);
ABS_API void abs_c_print(AbsComplex a);

/* --- Data science layer: NumPy-style shapes and generators, Pandas-style
 *     CSV, functional utils, and SciKit-Learn-style preprocessing --- */

/* Pythonic macros: print_mat(m) pretty-prints a matrix, foreach_mat(item, m)
 * loops over every element in row-major order. (len() and foreach() already
 * exist for the runtime's var objects, so the matrix variants are suffixed.) */
#define print_mat(m) abs_matrix_print(m)
#define foreach_mat(ITEM, MAT)                                                 \
    for (long _i_##ITEM = 0, _n_##ITEM = (long)((MAT)->val.matrix.rows *       \
                                                (MAT)->val.matrix.cols);       \
         _i_##ITEM < _n_##ITEM; _i_##ITEM++)                                   \
        if ((ITEM = (MAT)->val.matrix.data[_i_##ITEM]) || 1)

/* Shape manipulation (NumPy style). All return new matrices. */
ABS_API var abs_matrix_reshape(var m, int rows, int cols);
ABS_API var abs_matrix_flatten(var m);
ABS_API var abs_matrix_slice(var m, int r0, int r1, int c0, int c1);
ABS_API var abs_matrix_vstack(var A, var B);
ABS_API var abs_matrix_hstack(var A, var B);

/* Data generation. abs_matrix_arange/abs_matrix_linspace return 1 x N row
 * vectors, mirroring numpy.arange / numpy.linspace. */
ABS_API var abs_matrix_ones(int rows, int cols);
ABS_API var abs_matrix_arange(double start, double stop, double step);
ABS_API var abs_matrix_linspace(double start, double stop, int steps);

/* Pandas-style numeric CSV I/O. */
ABS_API var abs_matrix_read_csv(const char *filename);
ABS_API void abs_matrix_write_csv(var m, const char *filename);

/* Functional utils. abs_matrix_map returns a new matrix; abs_matrix_filter
 * returns a 1 x N matrix holding the elements that passed. */
ABS_API var abs_matrix_map(var m, double (*func)(double));
ABS_API var abs_matrix_filter(var m, int (*predicate)(double));

/* SciKit-Learn-style preprocessing. abs_matrix_train_test_split returns a
 * var list of four matrices: [X_train, X_test, Y_train, Y_test]. */
ABS_API var abs_matrix_one_hot_encode(var labels, int num_classes);
ABS_API var abs_matrix_train_test_split(var X, var Y, double test_ratio);

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
ABS_API var def(AbsFunc f, const char *name); /* make_func + display name */
ABS_API var call_func(var func_obj, var arg);
ABS_API var call_memoized(var func_obj, var arg);
ABS_API var decorate(var func_obj, AbsFunc wrapper_logic);
/* Target-aware decorator: the wrapper receives (target, args) and calls back
 * into call_func(target, args) to run the original; the original stays
 * reachable via func_meta(). */
ABS_API var decorate_func(var target, AbsWrapperFunc wrapper);
ABS_API var func_meta(var func_obj);
ABS_API var func_name(var func_obj);
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

/* --- Ultimate layer: backends, autograd, computer vision, plotting, dataframes --- */

/* Computational backends. ABS_CPU_AVX only activates the SIMD matrix kernel
 * when the library is compiled with -mavx (it falls back to the scalar kernel
 * otherwise, and to ABS_CPU whenever the CPU lacks AVX). ABS_GPU_CUDA is a
 * stub: it reports the request once and falls back to the CPU. */
typedef enum {
    ABS_CPU = 0,
    ABS_CPU_AVX = 1,
    ABS_GPU_CUDA = 2
} AbsBackend;

ABS_API void abs_set_backend(AbsBackend backend);
ABS_API AbsBackend abs_get_backend(void);
ABS_API const char *abs_backend_name(AbsBackend backend);

/* --- Scalar autograd: a Micrograd-style computational graph of double
 *     scalars with reverse-mode differentiation. Nodes are plain heap
 *     objects (like AbsComplex, not GC-tracked); free the root with
 *     abs_scalar_free() to release the whole graph. --- */

typedef struct AbsScalar AbsScalar;

ABS_API AbsScalar *abs_scalar_new(double val);
ABS_API AbsScalar *abs_scalar_add(AbsScalar *a, AbsScalar *b);
ABS_API AbsScalar *abs_scalar_mul(AbsScalar *a, AbsScalar *b);
ABS_API AbsScalar *abs_scalar_relu(AbsScalar *a);
ABS_API AbsScalar *abs_scalar_sigmoid(AbsScalar *a);
/* Reverse-mode pass: dL/da for every node reachable from root, with the root
 * seeded to 1.0. Accumulates on top of existing gradients. */
ABS_API void abs_scalar_backward(AbsScalar *root);
ABS_API void abs_scalar_zero_grad(AbsScalar *root);
/* Free the root and every node reachable from it (safe on shared subtrees). */
ABS_API void abs_scalar_free(AbsScalar *root);
ABS_API double abs_scalar_val(AbsScalar *v);
ABS_API double abs_scalar_grad(AbsScalar *v);

/* --- Computer vision: NetPBM PPM images (P3 text and P6 binary) plus a
 *     zero-padded 2D convolution. AbsImg owns a width*height*3 RGB buffer
 *     and is released with abs_img_free(). --- */

typedef struct {
    int width;
    int height;
    unsigned char *rgb; /* width * height * 3 bytes, row-major RGB */
} AbsImg;

ABS_API AbsImg *abs_img_load_ppm(const char *filename);
ABS_API void abs_img_save_ppm(const AbsImg *img, const char *filename);
/* Apply an odd-sized kernel_size x kernel_size filter (in row-major order)
 * to every channel independently, zero-padded, with output clamped to 0-255.
 * Returns a new image on success, NULL on invalid input. */
ABS_API AbsImg *abs_img_conv2d(const AbsImg *img, int kernel_size,
                               const double *kernel);
ABS_API void abs_img_free(AbsImg *img);

/* --- Plotting: ASCII terminal chart and SVG export of a double series. --- */

/* Prints a height-row ASCII chart of y[0..n). */
ABS_API void abs_plot_ascii(const double *y, int n, int height);
/* Exports an SVG line chart of (x[i], y[i]) to filename; pass x == NULL to
 * use sample indices as the x axis. */
ABS_API void abs_plot_svg(const double *x, const double *y, int n,
                          const char *filename);

/* --- Mixed-type DataFrame: named columns of doubles or strings over a
 *     shared row count. Owned by the caller; release with abs_df_free(). --- */

typedef enum {
    ABS_COL_DOUBLE,
    ABS_COL_STRING
} AbsColType;

typedef struct {
    char *name;
    AbsColType type;
    int rows;
    int capacity;
    double *doubles;  /* ABS_COL_DOUBLE values */
    char **strings;   /* ABS_COL_STRING values (owned) */
} AbsCol;

typedef struct {
    AbsCol **cols;
    int col_count;
    int rows;
} AbsDF;

ABS_API AbsDF *abs_df_create(int rows);
/* Append a column of df->rows values; the data is copied in. */
ABS_API void abs_df_add_col_double(AbsDF *df, const char *name,
                                   const double *values);
ABS_API void abs_df_add_col_string(AbsDF *df, const char *name,
                                   const char *const *values);
ABS_API void abs_df_print(const AbsDF *df);
ABS_API void abs_df_free(AbsDF *df);

/* --- Spatial math layer: modern type aliases (Rust/NumPy style), 2D/3D/4D
 *     vectors, fixed-size matrices, and quaternions. Plain value types —
 *     nothing to allocate or free. --- */

/* Part 1: modern type aliases. */
typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;
typedef ptrdiff_t   isize;
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;
typedef size_t      usize;
typedef float       f32;
typedef double      f64;
typedef bool        b8;
typedef uint32_t    b32;
typedef unsigned char byte;

/* Part 2: anonymous-union vectors. Components overlap, so v.x == v.raw[0],
 * and vec3 also offers r/g/b (color) aliases, vec4 r/g/b/a (RGBA). */
typedef union vec2 {
    struct { f32 x, y; };
    struct { f32 u, v; };
    f32 raw[2];
} vec2;

typedef union vec2d {
    struct { f64 x, y; };
    f64 raw[2];
} vec2d;

typedef union ivec2 {
    struct { i32 x, y; };
    i32 raw[2];
} ivec2;

typedef union vec3 {
    struct { f32 x, y, z; };
    struct { f32 r, g, b; };
    f32 raw[3];
} vec3;

typedef union vec3d {
    struct { f64 x, y, z; };
    f64 raw[3];
} vec3d;

typedef union ivec3 {
    struct { i32 x, y, z; };
    i32 raw[3];
} ivec3;

typedef union vec4 {
    struct { f32 x, y, z, w; };
    struct { f32 r, g, b, a; };
    f32 raw[4];
} vec4;

typedef union vec4d {
    struct { f64 x, y, z, w; };
    f64 raw[4];
} vec4d;

typedef union ivec4 {
    struct { i32 x, y, z, w; };
    i32 raw[4];
} ivec4;

/* Quaternion (x, y, z) vector part plus scalar w. */
typedef union quat {
    struct { f32 x, y, z, w; };
    f32 raw[4];
} quat;

/* Part 3: fixed-size matrices. Column-major, so m[col][row] and a mat4's
 * columns can be read directly as vec4s. */
typedef union mat2 {
    f32 m[2][2];
    f32 raw[4];
} mat2;

typedef union mat3 {
    f32 m[3][3];
    f32 raw[9];
    vec3 cols[3];
} mat3;

typedef union mat4 {
    f32 m[4][4];
    f32 raw[16];
    vec4 cols[4];
} mat4;

/* Part 4: constructors (static inline, no export needed). */
static inline vec2 v2(f32 x, f32 y) {
    vec2 v; v.x = x; v.y = y; return v;
}
static inline vec3 v3(f32 x, f32 y, f32 z) {
    vec3 v; v.x = x; v.y = y; v.z = z; return v;
}
static inline vec4 v4(f32 x, f32 y, f32 z, f32 w) {
    vec4 v; v.x = x; v.y = y; v.z = z; v.w = w; return v;
}
static inline ivec2 iv2(i32 x, i32 y) {
    ivec2 v; v.x = x; v.y = y; return v;
}
static inline ivec3 iv3(i32 x, i32 y, i32 z) {
    ivec3 v; v.x = x; v.y = y; v.z = z; return v;
}
static inline ivec4 iv4(i32 x, i32 y, i32 z, i32 w) {
    ivec4 v; v.x = x; v.y = y; v.z = z; v.w = w; return v;
}
static inline quat q4(f32 x, f32 y, f32 z, f32 w) {
    quat q; q.x = x; q.y = y; q.z = z; q.w = w; return q;
}

/* vec2 operations. */
ABS_API vec2 abs_v2_add(vec2 a, vec2 b);
ABS_API vec2 abs_v2_sub(vec2 a, vec2 b);
ABS_API vec2 abs_v2_scale(vec2 a, f32 s);
ABS_API f32 abs_v2_dot(vec2 a, vec2 b);
/* 2D cross product: the scalar z of the 3D cross product. */
ABS_API f32 abs_v2_cross(vec2 a, vec2 b);
ABS_API f32 abs_v2_len(vec2 v);
ABS_API f32 abs_v2_dist(vec2 a, vec2 b);
ABS_API vec2 abs_v2_norm(vec2 v);
ABS_API vec2 abs_v2_lerp(vec2 a, vec2 b, f32 t);
/* Reflect v about a unit normal n: v - 2*dot(v, n)*n. */
ABS_API vec2 abs_v2_reflect(vec2 v, vec2 n);
ABS_API void abs_v2_print(vec2 v, const char *name);

/* vec3 operations. */
ABS_API vec3 abs_v3_add(vec3 a, vec3 b);
ABS_API vec3 abs_v3_sub(vec3 a, vec3 b);
ABS_API vec3 abs_v3_scale(vec3 a, f32 s);
ABS_API f32 abs_v3_dot(vec3 a, vec3 b);
ABS_API vec3 abs_v3_cross(vec3 a, vec3 b);
ABS_API f32 abs_v3_len(vec3 v);
ABS_API f32 abs_v3_dist(vec3 a, vec3 b);
ABS_API vec3 abs_v3_norm(vec3 v);
ABS_API vec3 abs_v3_lerp(vec3 a, vec3 b, f32 t);
ABS_API vec3 abs_v3_reflect(vec3 v, vec3 n);
ABS_API void abs_v3_print(vec3 v, const char *name);

/* vec4 operations. */
ABS_API vec4 abs_v4_add(vec4 a, vec4 b);
ABS_API vec4 abs_v4_sub(vec4 a, vec4 b);
ABS_API vec4 abs_v4_scale(vec4 a, f32 s);
ABS_API f32 abs_v4_dot(vec4 a, vec4 b);
ABS_API f32 abs_v4_len(vec4 v);
ABS_API f32 abs_v4_dist(vec4 a, vec4 b);
ABS_API vec4 abs_v4_norm(vec4 v);
ABS_API vec4 abs_v4_lerp(vec4 a, vec4 b, f32 t);
ABS_API void abs_v4_print(vec4 v, const char *name);

/* Quaternion operations. */
ABS_API quat abs_quat_ident(void);
/* Hamilton product: compose rotation b (applied first), then a. */
ABS_API quat abs_quat_mul(quat a, quat b);
ABS_API quat abs_quat_norm(quat q);
ABS_API quat abs_quat_from_axis_angle(f32 ax, f32 ay, f32 az, f32 angle);
ABS_API vec3 abs_quat_rotate_vec3(quat q, vec3 v);
ABS_API void abs_quat_print(quat q, const char *name);

/* mat4 operations (column-major). */
ABS_API mat4 abs_mat4_identity(void);
ABS_API mat4 abs_mat4_mul(mat4 a, mat4 b);
ABS_API vec4 abs_mat4_mul_vec4(mat4 m, vec4 v);
ABS_API mat4 abs_mat4_translate(f32 x, f32 y, f32 z);
ABS_API mat4 abs_mat4_scale(f32 x, f32 y, f32 z);
ABS_API mat4 abs_mat4_rotate_x(f32 angle);
ABS_API mat4 abs_mat4_rotate_y(f32 angle);
ABS_API mat4 abs_mat4_rotate_z(f32 angle);
ABS_API mat4 abs_mat4_perspective(f32 fov_y, f32 aspect, f32 near, f32 far);
ABS_API mat4 abs_mat4_look_at(vec3 eye, vec3 center, vec3 up);
ABS_API void abs_mat4_print(mat4 m, const char *name);

/* --- Macro utilities suite: numeric, algebraic, graphics, bitwise, memory
 *     alignment, and array macros with defensive parentheses to prevent
 *     operator-precedence bugs, plus guarded unprefixed aliases. --- */

/* Part 5: comprehensive macro utilities. */

/* Basic arithmetic and comparison. */
#define ABS_MIN(a, b)             (((a) < (b)) ? (a) : (b))
#define ABS_MAX(a, b)             (((a) > (b)) ? (a) : (b))
#define ABS_MIN3(a, b, c)         ABS_MIN(ABS_MIN((a), (b)), (c))
#define ABS_MAX3(a, b, c)         ABS_MAX(ABS_MAX((a), (b)), (c))
#define ABS_MIN4(a, b, c, d)      ABS_MIN(ABS_MIN((a), (b)), ABS_MIN((c), (d)))
#define ABS_MAX4(a, b, c, d)      ABS_MAX(ABS_MAX((a), (b)), ABS_MAX((c), (d)))

#define ABS_ABS(x)                (((x) < 0) ? -(x) : (x))
#define ABS_SIGN(x)               (((x) > 0) - ((x) < 0))
#define ABS_CLAMP(x, lo, hi)      (((x) < (lo)) ? (lo) : (((x) > (hi)) ? (hi) : (x)))
#define ABS_CLAMP01(x)            ABS_CLAMP((x), 0.0, 1.0)
#define ABS_IN_RANGE(x, min, max) (((x) >= (min)) && ((x) <= (max)))

/* Powers, differences, and approximate equality. */
#define ABS_SQR(x)                ((x) * (x))
#define ABS_CUBE(x)               ((x) * (x) * (x))
#define ABS_DIFF(a, b)            ABS_ABS((a) - (b))
#define ABS_APPROX_EQ(a, b, eps)  (ABS_DIFF((a), (b)) <= (eps))

/* Interpolation and shading math (graphics / audio / AI curves). */
#define ABS_LERP(a, b, t)         ((a) + ((t) * ((b) - (a))))
#define ABS_UNLERP(a, b, val)     (((val) - (a)) / ((b) - (a)))
#define ABS_REMAP(val, in_a, in_b, out_a, out_b) \
    (ABS_LERP((out_a), (out_b), ABS_UNLERP((in_a), (in_b), (val))))

#define ABS_STEP(edge, x)         (((x) < (edge)) ? 0.0 : 1.0)
static inline double abs_smoothstep(double e0, double e1, double x) {
    double t = ABS_CLAMP01((x - e0) / (e1 - e0));
    return t * t * (3.0 - 2.0 * t);
}
#define ABS_SMOOTHSTEP(e0, e1, x) abs_smoothstep((e0), (e1), (x))

/* Angle and trigonometric conversions. */
#define ABS_DEG2RAD_M(deg)        ((deg) * (ABS_PI / 180.0))
#define ABS_RAD2DEG_M(rad)        ((rad) * (180.0 / ABS_PI))

/* Array, memory, and struct utilities. */
#define ABS_ARRAY_LEN(arr)        (sizeof(arr) / sizeof((arr)[0]))
#define ABS_OFFSETOF(type, member) ((size_t)&(((type *)0)->member))
#define ABS_CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - ABS_OFFSETOF(type, member)))

/* Generic type-safe swap (GCC/Clang statement expression). */
#if defined(__GNUC__) || defined(__clang__)
#define ABS_SWAP(a, b)            \
    do {                          \
        __typeof__(a) _tmp = (a); \
        (a) = (b);                \
        (b) = _tmp;               \
    } while (0)
#else
#define ABS_SWAP_T(type, a, b)    \
    do {                          \
        type _tmp = (a);          \
        (a) = (b);                \
        (b) = _tmp;               \
    } while (0)
#endif

/* Bitwise and power-of-two alignment operations. */
#define ABS_BIT(n)                (1ULL << (n))
#define ABS_BIT_SET(x, n)         ((x) |= ABS_BIT(n))
#define ABS_BIT_CLEAR(x, n)       ((x) &= ~ABS_BIT(n))
#define ABS_BIT_TOGGLE(x, n)      ((x) ^= ABS_BIT(n))
#define ABS_BIT_CHECK(x, n)       (((x) & ABS_BIT(n)) != 0)

#define ABS_IS_POW2(x)            (((x) != 0) && (((x) & ((x) - 1)) == 0))
#define ABS_ALIGN_UP(x, align)    (((x) + ((align) - 1)) & ~((align) - 1))
#define ABS_ALIGN_DOWN(x, align)  ((x) & ~((align) - 1))

/* AI / neural network activation helpers. */
#define ABS_RELU_M(x)             ABS_MAX(0.0, (x))
#define ABS_LEAKY_RELU_M(x, alpha) (((x) > 0.0) ? (x) : ((alpha) * (x)))
#define ABS_HEAVISIDE_M(x)        (((x) >= 0.0) ? 1.0 : 0.0)

/* Part 6: short-name aliases (guard-checked to prevent collisions). */
#ifndef MIN
#define MIN(a, b)                 ABS_MIN(a, b)
#endif
#ifndef MAX
#define MAX(a, b)                 ABS_MAX(a, b)
#endif
#ifndef CLAMP
#define CLAMP(x, lo, hi)          ABS_CLAMP(x, lo, hi)
#endif
#ifndef CLAMP01
#define CLAMP01(x)                ABS_CLAMP01(x)
#endif
#ifndef LERP
#define LERP(a, b, t)             ABS_LERP(a, b, t)
#endif
#ifndef REMAP
#define REMAP(v, ia, ib, oa, ob)  ABS_REMAP(v, ia, ib, oa, ob)
#endif
#ifndef SIGN
#define SIGN(x)                   ABS_SIGN(x)
#endif
#ifndef SQR
#define SQR(x)                    ABS_SQR(x)
#endif
#ifndef ARRAY_LEN
#define ARRAY_LEN(arr)            ABS_ARRAY_LEN(arr)
#endif
#ifndef SWAP
#define SWAP(a, b)                ABS_SWAP(a, b)
#endif
#ifndef DEG2RAD
#define DEG2RAD(d)                ABS_DEG2RAD_M(d)
#endif
#ifndef RAD2DEG
#define RAD2DEG(r)                ABS_RAD2DEG_M(r)
#endif
#ifndef BIT
#define BIT(n)                    ABS_BIT(n)
#endif
#ifndef IS_POW2
#define IS_POW2(x)                ABS_IS_POW2(x)
#endif

ABS_END_C_DECLS

#endif
