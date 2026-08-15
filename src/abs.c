#include "abscom/abs.h"
#include "abscom/abs_string.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#define POOL_BLOCK_SIZE 1024

typedef struct MemBlock {
    AbsObj objects[POOL_BLOCK_SIZE];
    struct MemBlock *next;
} MemBlock;

static MemBlock *pool_head = NULL;
static size_t pool_index = 0;
static var gc_dynamic_head = NULL;

#ifdef _WIN32
static CRITICAL_SECTION gc_lock;
#define GC_LOCK()   EnterCriticalSection(&gc_lock)
#define GC_UNLOCK() LeaveCriticalSection(&gc_lock)
#else
static pthread_mutex_t gc_lock = PTHREAD_MUTEX_INITIALIZER;
#define GC_LOCK()   pthread_mutex_lock(&gc_lock)
#define GC_UNLOCK() pthread_mutex_unlock(&gc_lock)
#endif

static var pool_alloc(void) {
    GC_LOCK();
    if (!pool_head || pool_index >= POOL_BLOCK_SIZE) {
        MemBlock *new_block = (MemBlock *)malloc(sizeof(MemBlock));
        if (!new_block) {
            GC_UNLOCK();
            return NULL;
        }
        new_block->next = pool_head;
        pool_head = new_block;
        pool_index = 0;
    }
    var obj = &pool_head->objects[pool_index++];
    memset(obj, 0, sizeof(AbsObj));
    GC_UNLOCK();
    return obj;
}

static char *abs_dup_str(const char *s) {
    if (!s) s = "";
    size_t n = strlen(s) + 1;
    char *copy = (char *)malloc(n);
    if (copy) memcpy(copy, s, n);
    return copy;
}

static void track_dynamic(var obj) {
    GC_LOCK();
    obj->next = gc_dynamic_head;
    gc_dynamic_head = obj;
    GC_UNLOCK();
}

static int is_num(var o) {
    return o->type == ABS_INT || o->type == ABS_FLOAT;
}

static double num_val(var o) {
    return o->type == ABS_FLOAT ? o->val.f : (double)o->val.i;
}

static void free_internals(var obj) {
    if (!obj) return;
    switch (obj->type) {
        case ABS_STR:   free(obj->val.s); break;
        case ABS_ERROR: free(obj->val.error_msg); break;
        case ABS_LIST:
        case ABS_SET:   free(obj->val.list.items); break;
        case ABS_CLASS: free(obj->val.cls.name); break;
        case ABS_FILE:
            if (obj->val.file_ptr) fclose(obj->val.file_ptr);
            break;
        case ABS_DICT:
            if (obj->val.dict.buckets) {
                for (size_t i = 0; i < obj->val.dict.capacity; i++) {
                    DictNode *node = obj->val.dict.buckets[i];
                    while (node) {
                        DictNode *tmp = node;
                        node = node->next;
                        free(tmp->key);
                        free(tmp);
                    }
                }
                free(obj->val.dict.buckets);
            }
            break;
        case ABS_MATRIX:
            free(obj->val.matrix.data);
            break;
        case ABS_THREAD:
#ifdef _WIN32
            if (obj->val.thread.handle)
                CloseHandle((HANDLE)obj->val.thread.handle);
#else
            free(obj->val.thread.handle);
#endif
            break;
        default:
            break;
    }
}

void abs_init(void) {
#ifdef _WIN32
    InitializeCriticalSection(&gc_lock);
#endif
    srand((unsigned int)time(NULL));
    if (pool_head) {
        MemBlock *b = pool_head;
        while (b) {
            MemBlock *t = b;
            b = b->next;
            free(t);
        }
        pool_head = NULL;
        pool_index = 0;
    }
    gc_dynamic_head = NULL;
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        /* sockets unavailable; http_get will still report errors */
    }
#endif
}

void abs_cleanup(void) {
    var current = gc_dynamic_head;
    while (current != NULL) {
        var temp = current;
        current = current->next;
        free_internals(temp);
    }
    gc_dynamic_head = NULL;
    MemBlock *b = pool_head;
    while (b) {
        MemBlock *t = b;
        b = b->next;
        free(t);
    }
    pool_head = NULL;
    pool_index = 0;
#ifdef _WIN32
    WSACleanup();
#endif
#ifdef _WIN32
    DeleteCriticalSection(&gc_lock);
#endif
}

void del(var obj) {
    if (!obj || obj->type == ABS_NONE) return;
    free_internals(obj);
    obj->type = ABS_NONE;
}

var abs_new_int(long v) {
    var obj = pool_alloc();
    if (!obj) return NULL;
    obj->type = ABS_INT;
    obj->val.i = v;
    return obj;
}

var abs_new_float(double v) {
    var obj = pool_alloc();
    if (!obj) return NULL;
    obj->type = ABS_FLOAT;
    obj->val.f = v;
    return obj;
}

var abs_new_str(const char *v) {
    var obj = pool_alloc();
    if (!obj) return NULL;
    obj->type = ABS_STR;
    obj->val.s = abs_dup_str(v);
    if (!obj->val.s) {
        obj->type = ABS_NONE;
        return NULL;
    }
    track_dynamic(obj);
    return obj;
}

var abs_new_bool(bool v) {
    var obj = pool_alloc();
    if (!obj) return NULL;
    obj->type = ABS_BOOL;
    obj->val.b = v;
    return obj;
}

var abs_new_none(void) {
    var obj = pool_alloc();
    if (!obj) return NULL;
    obj->type = ABS_NONE;
    return obj;
}

var abs_new_list(void) {
    var obj = pool_alloc();
    if (!obj) return NULL;
    obj->type = ABS_LIST;
    obj->val.list.size = 0;
    obj->val.list.capacity = 4;
    obj->val.list.items = (var *)malloc(4 * sizeof(var));
    if (!obj->val.list.items) {
        obj->type = ABS_NONE;
        return NULL;
    }
    track_dynamic(obj);
    return obj;
}

var abs_new_set(void) {
    var obj = pool_alloc();
    if (!obj) return NULL;
    obj->type = ABS_SET;
    obj->val.list.size = 0;
    obj->val.list.capacity = 4;
    obj->val.list.items = (var *)malloc(4 * sizeof(var));
    if (!obj->val.list.items) {
        obj->type = ABS_NONE;
        return NULL;
    }
    track_dynamic(obj);
    return obj;
}

var abs_new_dict(void) {
    var obj = pool_alloc();
    if (!obj) return NULL;
    obj->type = ABS_DICT;
    obj->val.dict.capacity = 16;
    obj->val.dict.count = 0;
    obj->val.dict.buckets = (DictNode **)calloc(16, sizeof(DictNode *));
    if (!obj->val.dict.buckets) {
        obj->type = ABS_NONE;
        return NULL;
    }
    track_dynamic(obj);
    return obj;
}

var abs_new_error(const char *msg) {
    var obj = pool_alloc();
    if (!obj) return NULL;
    obj->type = ABS_ERROR;
    obj->val.error_msg = abs_dup_str(msg);
    if (!obj->val.error_msg) {
        obj->type = ABS_NONE;
        return NULL;
    }
    track_dynamic(obj);
    return obj;
}

var abs_new_obj(AbsType type) {
    var obj = pool_alloc();
    if (!obj) return NULL;
    obj->type = type;
    return obj;
}

void abs_gc_track(var obj) {
    if (!obj) return;
    track_dynamic(obj);
}

double abs_num_val(var obj) {
    if (!obj) return 0.0;
    if (obj->type == ABS_FLOAT) return obj->val.f;
    if (obj->type == ABS_INT) return (double)obj->val.i;
    return 0.0;
}

static unsigned long dict_hash(const char *s) {
    unsigned long h = 5381;
    int c;
    while ((c = (unsigned char)*s++) != 0) h = ((h << 5) + h) + c;
    return h;
}

void dset(var dict, const char *key, var val) {
    if (!dict || dict->type != ABS_DICT || !key) return;
    unsigned long h = dict_hash(key) % dict->val.dict.capacity;
    DictNode *node = dict->val.dict.buckets[h];
    while (node) {
        if (strcmp(node->key, key) == 0) {
            node->value = val;
            return;
        }
        node = node->next;
    }
    DictNode *new_node = (DictNode *)malloc(sizeof(DictNode));
    if (!new_node) return;
    new_node->key = abs_dup_str(key);
    new_node->value = val;
    new_node->next = dict->val.dict.buckets[h];
    dict->val.dict.buckets[h] = new_node;
    dict->val.dict.count++;
}

var dget(var dict, const char *key) {
    if (!dict || dict->type != ABS_DICT) return abs_new_error("Not a dictionary");
    if (!key) return None;
    unsigned long h = dict_hash(key) % dict->val.dict.capacity;
    DictNode *node = dict->val.dict.buckets[h];
    while (node) {
        if (strcmp(node->key, key) == 0) return node->value;
        node = node->next;
    }
    return None;
}

static void print_single(var obj) {
    if (!obj) {
        printf("NULL");
        return;
    }
    switch (obj->type) {
        case ABS_INT:   printf("%ld", obj->val.i); break;
        case ABS_FLOAT: printf("%.2f", obj->val.f); break;
        case ABS_STR:   printf("%s", obj->val.s); break;
        case ABS_BOOL:  printf("%s", obj->val.b ? "True" : "False"); break;
        case ABS_NONE:  printf("None"); break;
        case ABS_LIST:
            printf("[");
            for (size_t i = 0; i < obj->val.list.size; i++) {
                if (i > 0) printf(", ");
                print_single(obj->val.list.items[i]);
            }
            printf("]");
            break;
        case ABS_DICT: {
            printf("{");
            int first_item = 1;
            for (size_t i = 0; i < obj->val.dict.capacity; i++) {
                DictNode *node = obj->val.dict.buckets[i];
                while (node) {
                    if (!first_item) printf(", ");
                    printf("%s: ", node->key);
                    print_single(node->value);
                    first_item = 0;
                    node = node->next;
                }
            }
            printf("}");
            break;
        }
        case ABS_SET:
            printf("{");
            for (size_t i = 0; i < obj->val.list.size; i++) {
                if (i > 0) printf(", ");
                print_single(obj->val.list.items[i]);
            }
            printf("}");
            break;
        case ABS_CLASS:
            printf("<class '%s'>", obj->val.cls.name);
            break;
        case ABS_INSTANCE:
            if (obj->val.inst.cls_ptr)
                printf("<%s object>", obj->val.inst.cls_ptr->val.cls.name);
            else
                printf("<instance object>");
            break;
        case ABS_FILE:  printf("<file>"); break;
        case ABS_ERROR: printf("Error: %s", obj->val.error_msg); break;
        case ABS_MATRIX: {
            int rows = obj->val.matrix.rows;
            int cols = obj->val.matrix.cols;
            printf("Matrix(%dx%d): [", rows, cols);
            for (int i = 0; i < rows; i++) {
                if (i > 0) printf(", ");
                printf("[");
                for (int j = 0; j < cols; j++) {
                    if (j > 0) printf(", ");
                    printf("%.2f", obj->val.matrix.data[i * cols + j]);
                }
                printf("]");
            }
            printf("]");
            break;
        }
        case ABS_THREAD: printf("<thread>"); break;
    }
}

void abs_print_impl(const char *end, ...) {
    va_list args;
    va_start(args, end);
    var obj;
    int first = 1;
    while ((obj = va_arg(args, var)) != NULL) {
        if (!first) printf(" ");
        print_single(obj);
        first = 0;
    }
    printf("%s", end);
    va_end(args);
}

var input(const char *prompt) {
    if (prompt) printf("%s", prompt);
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0';
        return abs_new_str(buffer);
    }
    return abs_new_str("");
}

var add(var a, var b) {
    if (!a || !b) return None;
    if (is_num(a) && is_num(b)) {
        if (a->type == ABS_INT && b->type == ABS_INT)
            return abs_new_int(a->val.i + b->val.i);
        return abs_new_float(num_val(a) + num_val(b));
    }
    if (a->type == ABS_STR && b->type == ABS_STR) {
        size_t n1 = strlen(a->val.s);
        size_t n2 = strlen(b->val.s);
        char *tmp = (char *)malloc(n1 + n2 + 1);
        if (!tmp) return None;
        memcpy(tmp, a->val.s, n1);
        memcpy(tmp + n1, b->val.s, n2 + 1);
        var res = abs_new_str(tmp);
        free(tmp);
        return res;
    }
    if (a->type == ABS_LIST && b->type == ABS_LIST) {
        var l = abs_new_list();
        if (!l) return None;
        for (size_t i = 0; i < a->val.list.size; i++)
            append(l, a->val.list.items[i]);
        for (size_t i = 0; i < b->val.list.size; i++)
            append(l, b->val.list.items[i]);
        return l;
    }
    return None;
}

var sub(var a, var b) {
    if (!a || !b || !is_num(a) || !is_num(b)) return None;
    if (a->type == ABS_INT && b->type == ABS_INT)
        return abs_new_int(a->val.i - b->val.i);
    return abs_new_float(num_val(a) - num_val(b));
}

static var str_repeat(const char *s, long count) {
    if (count <= 0) return abs_new_str("");
    size_t n = strlen(s);
    if (n > 0 && (size_t)count > SIZE_MAX / n) return None;
    size_t total = n * (size_t)count;
    char *tmp = (char *)malloc(total + 1);
    if (!tmp) return None;
    for (long i = 0; i < count; i++) memcpy(tmp + i * n, s, n);
    tmp[total] = '\0';
    var res = abs_new_str(tmp);
    free(tmp);
    return res;
}

var mul(var a, var b) {
    if (!a || !b) return None;
    if (a->type == ABS_STR && b->type == ABS_INT) return str_repeat(a->val.s, b->val.i);
    if (a->type == ABS_INT && b->type == ABS_STR) return str_repeat(b->val.s, a->val.i);
    if (is_num(a) && is_num(b)) {
        if (a->type == ABS_INT && b->type == ABS_INT)
            return abs_new_int(a->val.i * b->val.i);
        return abs_new_float(num_val(a) * num_val(b));
    }
    return None;
}

var eq(var a, var b) {
    if (!a || !b) return abs_new_bool(false);
    if (is_num(a) && is_num(b)) return abs_new_bool(num_val(a) == num_val(b));
    if (a->type == ABS_BOOL && b->type == ABS_BOOL)
        return abs_new_bool(a->val.b == b->val.b);
    if (a->type == ABS_STR && b->type == ABS_STR)
        return abs_new_bool(strcmp(a->val.s, b->val.s) == 0);
    if (a->type == ABS_NONE && b->type == ABS_NONE)
        return abs_new_bool(true);
    if (a->type == ABS_LIST && b->type == ABS_LIST) {
        if (a->val.list.size != b->val.list.size) return abs_new_bool(false);
        for (size_t i = 0; i < a->val.list.size; i++) {
            var r = eq(a->val.list.items[i], b->val.list.items[i]);
            if (!(r && r->type == ABS_BOOL && r->val.b)) return abs_new_bool(false);
        }
        return abs_new_bool(true);
    }
    return abs_new_bool(false);
}

static void str_build(abs_string_t *s, var obj) {
    char buf[64];
    switch (obj->type) {
        case ABS_INT:
            snprintf(buf, sizeof(buf), "%ld", obj->val.i);
            abs_string_append_cstr(s, buf);
            break;
        case ABS_FLOAT:
            snprintf(buf, sizeof(buf), "%.2f", obj->val.f);
            abs_string_append_cstr(s, buf);
            break;
        case ABS_STR:
            abs_string_append_cstr(s, obj->val.s);
            break;
        case ABS_BOOL:
            abs_string_append_cstr(s, obj->val.b ? "True" : "False");
            break;
        case ABS_NONE:
            abs_string_append_cstr(s, "None");
            break;
        case ABS_LIST:
            abs_string_append_cstr(s, "[");
            for (size_t i = 0; i < obj->val.list.size; i++) {
                if (i > 0) abs_string_append_cstr(s, ", ");
                str_build(s, obj->val.list.items[i]);
            }
            abs_string_append_cstr(s, "]");
            break;
        case ABS_DICT: {
            abs_string_append_cstr(s, "{");
            int first_item = 1;
            for (size_t i = 0; i < obj->val.dict.capacity; i++) {
                DictNode *node = obj->val.dict.buckets[i];
                while (node) {
                    if (!first_item) abs_string_append_cstr(s, ", ");
                    abs_string_append_cstr(s, node->key);
                    abs_string_append_cstr(s, ": ");
                    str_build(s, node->value);
                    first_item = 0;
                    node = node->next;
                }
            }
            abs_string_append_cstr(s, "}");
            break;
        }
        case ABS_SET: {
            abs_string_append_cstr(s, "{");
            for (size_t i = 0; i < obj->val.list.size; i++) {
                if (i > 0) abs_string_append_cstr(s, ", ");
                str_build(s, obj->val.list.items[i]);
            }
            abs_string_append_cstr(s, "}");
            break;
        }
        case ABS_CLASS: {
            char cls_buf[128];
            snprintf(cls_buf, sizeof(cls_buf), "<class '%s'>", obj->val.cls.name);
            abs_string_append_cstr(s, cls_buf);
            break;
        }
        case ABS_INSTANCE: {
            char inst_buf[128];
            if (obj->val.inst.cls_ptr)
                snprintf(inst_buf, sizeof(inst_buf), "<%s object>",
                         obj->val.inst.cls_ptr->val.cls.name);
            else
                snprintf(inst_buf, sizeof(inst_buf), "<instance object>");
            abs_string_append_cstr(s, inst_buf);
            break;
        }
        case ABS_FILE:
            abs_string_append_cstr(s, "<file>");
            break;
        case ABS_ERROR:
            abs_string_append_cstr(s, obj->val.error_msg);
            break;
        case ABS_MATRIX: {
            char num_buf[32];
            abs_string_append_cstr(s, "Matrix(");
            snprintf(num_buf, sizeof(num_buf), "%dx%d): [",
                     obj->val.matrix.rows, obj->val.matrix.cols);
            abs_string_append_cstr(s, num_buf);
            for (int i = 0; i < obj->val.matrix.rows; i++) {
                if (i > 0) abs_string_append_cstr(s, ", ");
                abs_string_append_cstr(s, "[");
                for (int j = 0; j < obj->val.matrix.cols; j++) {
                    if (j > 0) abs_string_append_cstr(s, ", ");
                    snprintf(num_buf, sizeof(num_buf), "%.2f",
                             obj->val.matrix.data[i * obj->val.matrix.cols + j]);
                    abs_string_append_cstr(s, num_buf);
                }
                abs_string_append_cstr(s, "]");
            }
            abs_string_append_cstr(s, "]");
            break;
        }
        case ABS_THREAD:
            abs_string_append_cstr(s, "<thread>");
            break;
    }
}

var to_str(var obj) {
    if (!obj) return abs_new_str("None");
    if (obj->type == ABS_STR) return obj;
    abs_string_t s;
    abs_string_init(&s);
    str_build(&s, obj);
    var res = abs_new_str(abs_string_c_str(&s));
    abs_string_destroy(&s);
    return res;
}

var to_int(var obj) {
    if (!obj) return None;
    char *end;
    switch (obj->type) {
        case ABS_INT:   return abs_new_int(obj->val.i);
        case ABS_FLOAT: return abs_new_int((long)obj->val.f);
        case ABS_BOOL:  return abs_new_int(obj->val.b ? 1 : 0);
        case ABS_STR: {
            long v = strtol(obj->val.s, &end, 10);
            if (end == obj->val.s) return None;
            return abs_new_int(v);
        }
        default: return None;
    }
}

var to_float(var obj) {
    if (!obj) return None;
    char *end;
    switch (obj->type) {
        case ABS_INT:   return abs_new_float((double)obj->val.i);
        case ABS_FLOAT: return abs_new_float(obj->val.f);
        case ABS_BOOL:  return abs_new_float(obj->val.b ? 1.0 : 0.0);
        case ABS_STR: {
            double v = strtod(obj->val.s, &end);
            if (end == obj->val.s) return None;
            return abs_new_float(v);
        }
        default: return None;
    }
}

void append(var list, var item) {
    if (!list || list->type != ABS_LIST) return;
    if (list->val.list.size >= list->val.list.capacity) {
        size_t new_cap = list->val.list.capacity ? list->val.list.capacity * 2 : 4;
        if (new_cap < list->val.list.capacity) return;
        var *items = (var *)realloc(list->val.list.items, new_cap * sizeof(var));
        if (!items) return;
        list->val.list.items = items;
        list->val.list.capacity = new_cap;
    }
    list->val.list.items[list->val.list.size++] = item;
}

var get(var obj, long index) {
    if (!obj || (obj->type != ABS_LIST && obj->type != ABS_SET))
        return abs_new_error("Not a list");
    if (index < 0) index += (long)obj->val.list.size;
    if (index < 0 || (size_t)index >= obj->val.list.size)
        return abs_new_error("Index out of bounds");
    return obj->val.list.items[index];
}

var len(var obj) {
    if (!obj) return abs_new_int(0);
    switch (obj->type) {
        case ABS_STR:  return abs_new_int((long)strlen(obj->val.s));
        case ABS_LIST:
        case ABS_SET:  return abs_new_int((long)obj->val.list.size);
        case ABS_DICT: return abs_new_int((long)obj->val.dict.count);
        default:       return abs_new_int(0);
    }
}

long get_len_fast(var obj) {
    if (!obj) return 0;
    if (obj->type == ABS_LIST || obj->type == ABS_SET) return (long)obj->val.list.size;
    return 0;
}

var range(int start, int stop) {
    var l = abs_new_list();
    if (!l) return None;
    for (long i = start; i < stop; i++) append(l, abs_new_int(i));
    return l;
}

var slice(var obj, int start, int stop) {
    if (!obj || obj->type != ABS_LIST) return abs_new_error("Can only slice lists");
    long n = (long)obj->val.list.size;
    if (start < 0) start += (int)n;
    if (stop < 0) stop += (int)n;
    if (start < 0) start = 0;
    if (stop > n) stop = (int)n;
    var l = abs_new_list();
    if (!l) return None;
    for (int i = start; i < stop; i++) append(l, obj->val.list.items[i]);
    return l;
}

var upper(var obj) {
    if (!obj || obj->type != ABS_STR) return None;
    char *tmp = abs_dup_str(obj->val.s);
    if (!tmp) return None;
    for (size_t i = 0; tmp[i]; i++) tmp[i] = (char)toupper((unsigned char)tmp[i]);
    var res = abs_new_str(tmp);
    free(tmp);
    return res;
}

var lower(var obj) {
    if (!obj || obj->type != ABS_STR) return None;
    char *tmp = abs_dup_str(obj->val.s);
    if (!tmp) return None;
    for (size_t i = 0; tmp[i]; i++) tmp[i] = (char)tolower((unsigned char)tmp[i]);
    var res = abs_new_str(tmp);
    free(tmp);
    return res;
}

var split(var str_obj, const char *delimiter) {
    if (!str_obj || str_obj->type != ABS_STR) return abs_new_error("Not a string");
    if (!delimiter) delimiter = "";
    var l = abs_new_list();
    if (!l) return None;
    size_t dlen = strlen(delimiter);
    if (dlen == 0) {
        for (const char *p = str_obj->val.s; *p; p++) {
            char ch[2] = { *p, '\0' };
            append(l, abs_new_str(ch));
        }
        if (!*str_obj->val.s) append(l, abs_new_str(""));
        return l;
    }
    const char *start = str_obj->val.s;
    const char *p = start;
    while ((p = strstr(start, delimiter)) != NULL) {
        size_t n = (size_t)(p - start);
        char *tmp = (char *)malloc(n + 1);
        if (!tmp) return l;
        memcpy(tmp, start, n);
        tmp[n] = '\0';
        append(l, abs_new_str(tmp));
        free(tmp);
        start = p + dlen;
    }
    append(l, abs_new_str(start));
    return l;
}

var join(var delimiter, var list_obj) {
    if (!list_obj || list_obj->type != ABS_LIST) return abs_new_error("Not a list");
    const char *delim = (delimiter && delimiter->type == ABS_STR) ? delimiter->val.s : "";
    size_t dlen = strlen(delim);
    size_t total = 1;
    for (size_t i = 0; i < list_obj->val.list.size; i++) {
        var item = list_obj->val.list.items[i];
        if (item && item->type == ABS_STR) total += strlen(item->val.s);
        if (i < list_obj->val.list.size - 1) total += dlen;
    }
    char *res = (char *)calloc(total, 1);
    if (!res) return None;
    for (size_t i = 0; i < list_obj->val.list.size; i++) {
        var item = list_obj->val.list.items[i];
        if (item && item->type == ABS_STR) strcat(res, item->val.s);
        if (i < list_obj->val.list.size - 1) strcat(res, delim);
    }
    var ret = abs_new_str(res);
    free(res);
    return ret;
}

var fmt_impl(const char *format, ...) {
    va_list args;
    va_start(args, format);
    abs_string_t s;
    abs_string_init(&s);
    const char *p = format ? format : "";
    while (*p) {
        if (p[0] == '{' && p[1] == '}') {
            var arg = va_arg(args, var);
            if (arg) {
                var text = to_str(arg);
                if (text && text->val.s) abs_string_append_cstr(&s, text->val.s);
            }
            p += 2;
        } else {
            abs_string_append_char(&s, *p);
            p++;
        }
    }
    va_end(args);
    var res = abs_new_str(abs_string_c_str(&s));
    abs_string_destroy(&s);
    return res;
}

var map_func(var list, AbsFunc f) {
    if (!list || list->type != ABS_LIST) return abs_new_error("Not a list");
    var res = abs_new_list();
    if (!res) return None;
    for (size_t i = 0; i < list->val.list.size; i++)
        append(res, f(list->val.list.items[i]));
    return res;
}

bool is_true(var obj) {
    if (!obj || obj->type == ABS_NONE) return false;
    switch (obj->type) {
        case ABS_BOOL:  return obj->val.b;
        case ABS_INT:   return obj->val.i != 0;
        case ABS_FLOAT: return obj->val.f != 0.0;
        case ABS_STR:   return obj->val.s && obj->val.s[0];
        case ABS_LIST:
        case ABS_SET:   return obj->val.list.size != 0;
        case ABS_DICT:  return obj->val.dict.count != 0;
        case ABS_ERROR: return false;
        default:        return true;
    }
}

var filter_func(var list, AbsFunc f) {
    if (!list || list->type != ABS_LIST) return abs_new_error("Not a list");
    var res = abs_new_list();
    if (!res) return None;
    for (size_t i = 0; i < list->val.list.size; i++) {
        var item = list->val.list.items[i];
        if (is_true(f(item))) append(res, item);
    }
    return res;
}

var fopen_safe(const char *filename, const char *mode) {
    if (!filename) return abs_new_error("No filename given");
    FILE *f = fopen(filename, mode ? mode : "r");
    if (!f) return abs_new_error("Could not open file");
    var obj = pool_alloc();
    if (!obj) {
        fclose(f);
        return NULL;
    }
    obj->type = ABS_FILE;
    obj->val.file_ptr = f;
    track_dynamic(obj);
    return obj;
}

var read_file(var file_obj) {
    if (!file_obj || file_obj->type != ABS_FILE || !file_obj->val.file_ptr)
        return abs_new_error("Not a file");
    FILE *f = file_obj->val.file_ptr;
    if (fseek(f, 0, SEEK_END) != 0) return abs_new_error("File seek failed");
    long fsize = ftell(f);
    if (fsize < 0) return abs_new_error("File read failed");
    rewind(f);
    char *buf = (char *)malloc((size_t)fsize + 1);
    if (!buf) return None;
    size_t rd = fread(buf, 1, (size_t)fsize, f);
    buf[rd] = '\0';
    var res = abs_new_str(buf);
    free(buf);
    return res;
}

void write_file(var file_obj, var content) {
    if (!file_obj || file_obj->type != ABS_FILE || !file_obj->val.file_ptr) return;
    if (!content) return;
    var text = content->type == ABS_STR ? content : to_str(content);
    if (text && text->val.s) fputs(text->val.s, file_obj->val.file_ptr);
}

void close_file(var file_obj) {
    if (!file_obj || file_obj->type != ABS_FILE) return;
    if (file_obj->val.file_ptr) {
        fclose(file_obj->val.file_ptr);
        file_obj->val.file_ptr = NULL;
    }
}

static const char *json_cur;

static void json_skip_ws(void) {
    while (*json_cur && isspace((unsigned char)*json_cur)) json_cur++;
}

static var parse_json_value(void);

static var parse_json_string(void) {
    if (*json_cur == '"') json_cur++;
    else return abs_new_error("Expected string");
    abs_string_t s;
    abs_string_init(&s);
    while (*json_cur && *json_cur != '"') {
        char c = *json_cur;
        if (c == '\\') {
            json_cur++;
            if (!*json_cur) break;
            switch (*json_cur) {
                case 'n': abs_string_append_char(&s, '\n'); break;
                case 't': abs_string_append_char(&s, '\t'); break;
                case 'r': abs_string_append_char(&s, '\r'); break;
                case 'b': abs_string_append_char(&s, '\b'); break;
                case 'f': abs_string_append_char(&s, '\f'); break;
                case '"': abs_string_append_char(&s, '"'); break;
                case '\\': abs_string_append_char(&s, '\\'); break;
                case '/': abs_string_append_char(&s, '/'); break;
                default:
                    abs_string_append_char(&s, '\\');
                    abs_string_append_char(&s, *json_cur);
                    break;
            }
        } else {
            abs_string_append_char(&s, c);
        }
        json_cur++;
    }
    if (*json_cur == '"') json_cur++;
    var res = abs_new_str(abs_string_c_str(&s));
    abs_string_destroy(&s);
    return res;
}

static var parse_json_number(void) {
    char buf[64];
    size_t i = 0;
    while (i < sizeof(buf) - 1 &&
           (*json_cur == '-' || *json_cur == '+' ||
            isdigit((unsigned char)*json_cur) || *json_cur == '.' ||
            *json_cur == 'e' || *json_cur == 'E')) {
        buf[i++] = *json_cur++;
    }
    buf[i] = '\0';
    if (i == 0) return abs_new_error("Invalid number");
    if (strchr(buf, '.') || strchr(buf, 'e') || strchr(buf, 'E'))
        return abs_new_float(atof(buf));
    return abs_new_int(atol(buf));
}

static var parse_json_keyword(void) {
    if (strncmp(json_cur, "true", 4) == 0) { json_cur += 4; return abs_new_bool(true); }
    if (strncmp(json_cur, "false", 5) == 0) { json_cur += 5; return abs_new_bool(false); }
    if (strncmp(json_cur, "null", 4) == 0) { json_cur += 4; return abs_new_none(); }
    return abs_new_error("Invalid JSON value");
}

static var parse_json_list(void) {
    json_cur++;
    var l = abs_new_list();
    if (!l) return None;
    json_skip_ws();
    if (*json_cur == ']') { json_cur++; return l; }
    for (;;) {
        var item = parse_json_value();
        if (item->type == ABS_ERROR) return item;
        append(l, item);
        json_skip_ws();
        if (*json_cur == ']') { json_cur++; return l; }
        if (*json_cur == ',') { json_cur++; json_skip_ws(); }
        else return abs_new_error("Expected ',' or ']' in list");
    }
}

static var parse_json_dict(void) {
    json_cur++;
    var d = abs_new_dict();
    if (!d) return None;
    json_skip_ws();
    if (*json_cur == '}') { json_cur++; return d; }
    for (;;) {
        json_skip_ws();
        var key = parse_json_string();
        if (key->type == ABS_ERROR) return key;
        json_skip_ws();
        if (*json_cur != ':') return abs_new_error("Expected ':' in object");
        json_cur++;
        var val = parse_json_value();
        if (val->type == ABS_ERROR) return val;
        dset(d, key->val.s, val);
        json_skip_ws();
        if (*json_cur == '}') { json_cur++; return d; }
        if (*json_cur == ',') { json_cur++; json_skip_ws(); }
        else return abs_new_error("Expected ',' or '}' in object");
    }
}

static var parse_json_value(void) {
    json_skip_ws();
    if (!*json_cur) return abs_new_error("Unexpected end of JSON");
    if (*json_cur == '"') return parse_json_string();
    if (*json_cur == '[') return parse_json_list();
    if (*json_cur == '{') return parse_json_dict();
    if (*json_cur == '-' || isdigit((unsigned char)*json_cur)) return parse_json_number();
    return parse_json_keyword();
}

var json_parse(const char *json_str) {
    if (!json_str) return abs_new_error("No JSON string");
    json_cur = json_str;
    var res = parse_json_value();
    json_skip_ws();
    if (*json_cur && res->type != ABS_ERROR)
        return abs_new_error("Trailing characters after JSON value");
    return res;
}

void random_seed(void) {
    srand((unsigned int)time(NULL));
}

void seed(var obj) {
    if (obj && obj->type == ABS_INT) {
        srand((unsigned int)obj->val.i);
    } else if (obj && obj->type == ABS_STR) {
        srand((unsigned int)dict_hash(obj->val.s));
    } else {
        srand((unsigned int)time(NULL));
    }
}

var randint(int min, int max) {
    if (max < min) {
        int tmp = min;
        min = max;
        max = tmp;
    }
    long range = (long)max - (long)min + 1;
    return abs_new_int((long)(rand() % (int)range) + min);
}

var random_float(void) {
    return abs_new_float((double)rand() / ((double)RAND_MAX + 1.0));
}

var uniform(double a, double b) {
    double t = (double)rand() / (double)RAND_MAX;
    return abs_new_float(a + (b - a) * t);
}

var choice(var seq) {
    if (!seq) return abs_new_error("Object is not a sequence");
    if (seq->type == ABS_LIST) {
        if (seq->val.list.size == 0) return abs_new_error("Cannot choose from empty list");
        return seq->val.list.items[rand() % seq->val.list.size];
    }
    if (seq->type == ABS_STR) {
        size_t len = strlen(seq->val.s);
        if (len == 0) return abs_new_error("Cannot choose from empty string");
        char buf[2] = { seq->val.s[rand() % len], '\0' };
        return abs_new_str(buf);
    }
    return abs_new_error("Object is not a sequence");
}

var choices(var seq, int k) {
    if (!seq || (seq->type != ABS_LIST && seq->type != ABS_STR))
        return abs_new_error("Object is not a sequence");
    size_t n = seq->type == ABS_LIST ? seq->val.list.size : strlen(seq->val.s);
    if (n == 0) return abs_new_error("Cannot choose from empty sequence");
    var res = abs_new_list();
    if (!res) return None;
    for (int i = 0; i < k; i++) append(res, choice(seq));
    return res;
}

void shuffle(var list) {
    if (!list || list->type != ABS_LIST) return;
    size_t n = list->val.list.size;
    if (n < 2) return;
    for (size_t i = n - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);
        var temp = list->val.list.items[i];
        list->val.list.items[i] = list->val.list.items[j];
        list->val.list.items[j] = temp;
    }
}

var sample(var seq, int k) {
    if (!seq || seq->type != ABS_LIST)
        return abs_new_error("Sample currently only supports lists");
    size_t n = seq->val.list.size;
    if (k < 0 || (size_t)k > n)
        return abs_new_error("Sample larger than population or negative");
    int *indices = (int *)malloc(n * sizeof(int));
    if (!indices) return None;
    for (size_t i = 0; i < n; i++) indices[i] = (int)i;
    for (int i = 0; i < k; i++) {
        int j = i + (int)(rand() % (n - (size_t)i));
        int tmp = indices[i];
        indices[i] = indices[j];
        indices[j] = tmp;
    }
    var res = abs_new_list();
    if (!res) {
        free(indices);
        return None;
    }
    for (int i = 0; i < k; i++) append(res, seq->val.list.items[indices[i]]);
    free(indices);
    return res;
}

var type(var obj) {
    if (!obj) return abs_new_str("<class 'NoneType'>");
    switch (obj->type) {
        case ABS_INT:   return abs_new_str("<class 'int'>");
        case ABS_FLOAT: return abs_new_str("<class 'float'>");
        case ABS_STR:   return abs_new_str("<class 'str'>");
        case ABS_BOOL:  return abs_new_str("<class 'bool'>");
        case ABS_LIST:  return abs_new_str("<class 'list'>");
        case ABS_DICT:  return abs_new_str("<class 'dict'>");
        case ABS_SET:   return abs_new_str("<class 'set'>");
        case ABS_FILE:  return abs_new_str("<class 'file'>");
        case ABS_ERROR: return abs_new_str("<class 'error'>");
        case ABS_CLASS: return abs_new_str("<class 'class'>");
        case ABS_INSTANCE: return abs_new_str("<class 'instance'>");
        case ABS_MATRIX:  return abs_new_str("<class 'matrix'>");
        case ABS_THREAD:  return abs_new_str("<class 'thread'>");
        default:        return abs_new_str("<class 'NoneType'>");
    }
}

bool is_err(var obj) {
    return obj && obj->type == ABS_ERROR;
}

int compare_objs(var a, var b) {
    if (!a || !b) return 0;
    if (a->type == ABS_INT && b->type == ABS_INT)
        return (a->val.i < b->val.i) ? -1 : (a->val.i > b->val.i);
    if (is_num(a) && is_num(b)) {
        double v1 = num_val(a);
        double v2 = num_val(b);
        return (v1 < v2) ? -1 : (v1 > v2);
    }
    if (a->type == ABS_STR && b->type == ABS_STR) {
        int c = strcmp(a->val.s, b->val.s);
        return (c < 0) ? -1 : (c > 0);
    }
    return 0;
}

var min_val(var list) {
    if (!is_list(list) || list->val.list.size == 0) return None;
    var best = list->val.list.items[0];
    for (size_t i = 1; i < list->val.list.size; i++)
        if (compare_objs(list->val.list.items[i], best) < 0) best = list->val.list.items[i];
    return best;
}

var max_val(var list) {
    if (!is_list(list) || list->val.list.size == 0) return None;
    var best = list->val.list.items[0];
    for (size_t i = 1; i < list->val.list.size; i++)
        if (compare_objs(list->val.list.items[i], best) > 0) best = list->val.list.items[i];
    return best;
}

var sum_val(var list) {
    if (!is_list(list)) return abs_new_int(0);
    double total_f = 0;
    long total_i = 0;
    bool use_float = false;
    for (size_t i = 0; i < list->val.list.size; i++) {
        var item = list->val.list.items[i];
        if (item->type == ABS_FLOAT) {
            use_float = true;
            total_f += item->val.f;
        } else if (item->type == ABS_INT) {
            total_i += item->val.i;
            total_f += (double)item->val.i;
        }
    }
    if (use_float) return abs_new_float(total_f);
    return abs_new_int(total_i);
}

var not_(var obj) {
    return abs_new_bool(!is_true(obj));
}

var any(var list) {
    if (!is_list(list)) return abs_new_bool(false);
    for (size_t i = 0; i < list->val.list.size; i++)
        if (is_true(list->val.list.items[i])) return abs_new_bool(true);
    return abs_new_bool(false);
}

var all(var list) {
    if (!is_list(list)) return abs_new_bool(true);
    for (size_t i = 0; i < list->val.list.size; i++)
        if (!is_true(list->val.list.items[i])) return abs_new_bool(false);
    return abs_new_bool(true);
}

var abs_val(var obj) {
    if (is_int(obj)) return abs_new_int(labs(obj->val.i));
    if (is_float(obj)) return abs_new_float(fabs(obj->val.f));
    return abs_new_int(0);
}

var pow_val(var base, var exp) {
    if (!base || !exp) return None;
    double b = is_num(base) ? num_val(base) : 0.0;
    double e = is_num(exp) ? num_val(exp) : 0.0;
    return abs_new_float(pow(b, e));
}

var round_val(var obj, int digits) {
    if (!is_float(obj)) return obj;
    double scale = pow(10, digits);
    double res = round(obj->val.f * scale) / scale;
    if (digits == 0) return abs_new_int((long)res);
    return abs_new_float(res);
}

var sorted(var list, bool reverse) {
    if (!is_list(list)) return None;
    var res = abs_new_list();
    if (!res) return None;
    for (size_t i = 0; i < list->val.list.size; i++)
        append(res, list->val.list.items[i]);
    size_t n = res->val.list.size;
    for (size_t i = 0; i + 1 < n; i++) {
        for (size_t j = 0; j + 1 < n - i; j++) {
            int cmp = compare_objs(res->val.list.items[j], res->val.list.items[j + 1]);
            bool swap = reverse ? (cmp < 0) : (cmp > 0);
            if (swap) {
                var temp = res->val.list.items[j];
                res->val.list.items[j] = res->val.list.items[j + 1];
                res->val.list.items[j + 1] = temp;
            }
        }
    }
    return res;
}

var reversed_seq(var list) {
    if (!is_list(list)) return None;
    var res = abs_new_list();
    if (!res) return None;
    long n = (long)list->val.list.size;
    for (long i = n - 1; i >= 0; i--) append(res, list->val.list.items[i]);
    return res;
}

var zip_lists(var list1, var list2) {
    if (!is_list(list1) || !is_list(list2)) return abs_new_list();
    var res = abs_new_list();
    if (!res) return None;
    size_t n = list1->val.list.size < list2->val.list.size ? list1->val.list.size : list2->val.list.size;
    for (size_t i = 0; i < n; i++) {
        var pair = abs_new_list();
        append(pair, list1->val.list.items[i]);
        append(pair, list2->val.list.items[i]);
        append(res, pair);
    }
    return res;
}

var strip(var str_obj) {
    if (!is_str(str_obj)) return None;
    const char *s = str_obj->val.s;
    while (*s && isspace((unsigned char)*s)) s++;
    if (!*s) return abs_new_str("");
    const char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    size_t len = (size_t)(end - s) + 1;
    char *new_s = (char *)malloc(len + 1);
    if (!new_s) return None;
    memcpy(new_s, s, len);
    new_s[len] = '\0';
    var res = abs_new_str(new_s);
    free(new_s);
    return res;
}

var startswith(var str, var prefix) {
    if (!is_str(str) || !is_str(prefix)) return abs_new_bool(false);
    return abs_new_bool(strncmp(str->val.s, prefix->val.s, strlen(prefix->val.s)) == 0);
}

var endswith(var str, var suffix) {
    if (!is_str(str) || !is_str(suffix)) return abs_new_bool(false);
    size_t slen = strlen(str->val.s);
    size_t plen = strlen(suffix->val.s);
    if (plen > slen) return abs_new_bool(false);
    return abs_new_bool(strcmp(str->val.s + slen - plen, suffix->val.s) == 0);
}

var count(var container, var item) {
    int c = 0;
    if (is_list(container)) {
        for (size_t i = 0; i < container->val.list.size; i++)
            if (compare_objs(container->val.list.items[i], item) == 0) c++;
    } else if (is_str(container) && is_str(item)) {
        size_t plen = strlen(item->val.s);
        if (plen > 0) {
            const char *tmp = container->val.s;
            while ((tmp = strstr(tmp, item->val.s))) {
                c++;
                tmp += plen;
            }
        }
    }
    return abs_new_int(c);
}

void sleep_sec(double seconds) {
    if (seconds <= 0) return;
#ifdef _WIN32
    Sleep((DWORD)(seconds * 1000));
#else
    usleep((useconds_t)(seconds * 1000000));
#endif
}

var time_now(void) {
    return abs_new_float((double)time(NULL));
}

var exec_cmd(const char *cmd) {
    return abs_new_int(cmd ? system(cmd) : -1);
}

bool is_int(var o)   { return o && o->type == ABS_INT; }
bool is_float(var o) { return o && o->type == ABS_FLOAT; }
bool is_str(var o)   { return o && o->type == ABS_STR; }
bool is_list(var o)  { return o && o->type == ABS_LIST; }
bool is_dict(var o)  { return o && o->type == ABS_DICT; }
bool is_set(var o)   { return o && o->type == ABS_SET; }
bool is_none(var o)  { return !o || o->type == ABS_NONE; }

var Class(const char *name) {
    var obj = pool_alloc();
    if (!obj) return NULL;
    obj->type = ABS_CLASS;
    obj->val.cls.name = abs_dup_str(name ? name : "");
    obj->val.cls.methods_dict = abs_new_dict();
    track_dynamic(obj);
    return obj;
}

var New(var cls) {
    if (!cls || cls->type != ABS_CLASS) return None;
    var obj = pool_alloc();
    if (!obj) return NULL;
    obj->type = ABS_INSTANCE;
    obj->val.inst.cls_ptr = cls;
    obj->val.inst.attr_dict = abs_new_dict();
    return obj;
}

void set_attr(var obj, const char *key, var val) {
    if (!obj || obj->type != ABS_INSTANCE || !key) return;
    dset(obj->val.inst.attr_dict, key, val);
}

var get_attr(var obj, const char *key) {
    if (!obj || obj->type != ABS_INSTANCE) return None;
    return dget(obj->val.inst.attr_dict, key ? key : "");
}

static void json_escape(abs_string_t *s, const char *text) {
    char esc[8];
    const unsigned char *p = (const unsigned char *)text;
    while (*p) {
        switch (*p) {
            case '"':  abs_string_append_cstr(s, "\\\""); break;
            case '\\': abs_string_append_cstr(s, "\\\\"); break;
            case '\n': abs_string_append_cstr(s, "\\n"); break;
            case '\r': abs_string_append_cstr(s, "\\r"); break;
            case '\t': abs_string_append_cstr(s, "\\t"); break;
            case '\b': abs_string_append_cstr(s, "\\b"); break;
            case '\f': abs_string_append_cstr(s, "\\f"); break;
            default:
                if (*p < 0x20) {
                    snprintf(esc, sizeof(esc), "\\u%04x", *p);
                    abs_string_append_cstr(s, esc);
                } else {
                    abs_string_append_char(s, (char)*p);
                }
                break;
        }
        p++;
    }
}

static void json_dump_rec(abs_string_t *s, var obj) {
    char buf[64];
    if (!obj) {
        abs_string_append_cstr(s, "null");
        return;
    }
    switch (obj->type) {
        case ABS_INT:
            snprintf(buf, sizeof(buf), "%ld", obj->val.i);
            abs_string_append_cstr(s, buf);
            break;
        case ABS_FLOAT:
            snprintf(buf, sizeof(buf), "%.17g", obj->val.f);
            abs_string_append_cstr(s, buf);
            break;
        case ABS_STR:
            abs_string_append_char(s, '"');
            json_escape(s, obj->val.s);
            abs_string_append_char(s, '"');
            break;
        case ABS_BOOL:
            abs_string_append_cstr(s, obj->val.b ? "true" : "false");
            break;
        case ABS_NONE:
            abs_string_append_cstr(s, "null");
            break;
        case ABS_LIST:
        case ABS_SET: {
            abs_string_append_char(s, '[');
            for (size_t i = 0; i < obj->val.list.size; i++) {
                if (i > 0) abs_string_append_cstr(s, ", ");
                json_dump_rec(s, obj->val.list.items[i]);
            }
            abs_string_append_char(s, ']');
            break;
        }
        case ABS_DICT: {
            abs_string_append_char(s, '{');
            int count = 0;
            for (size_t i = 0; i < obj->val.dict.capacity; i++) {
                DictNode *node = obj->val.dict.buckets[i];
                while (node) {
                    if (count > 0) abs_string_append_cstr(s, ", ");
                    abs_string_append_char(s, '"');
                    json_escape(s, node->key);
                    abs_string_append_cstr(s, "\": ");
                    json_dump_rec(s, node->value);
                    count++;
                    node = node->next;
                }
            }
            abs_string_append_char(s, '}');
            break;
        }
        default:
            abs_string_append_cstr(s, "null");
            break;
    }
}

var json_dump(var obj) {
    abs_string_t s;
    abs_string_init(&s);
    json_dump_rec(&s, obj);
    var res = abs_new_str(abs_string_c_str(&s));
    abs_string_destroy(&s);
    return res;
}

var http_get(const char *url) {
    if (!url) return abs_new_error("No URL");
    char hostname[256] = "";
    char path[1024] = "/";
    const char *p = strstr(url, "://");
    if (p) p += 3;
    else p = url;
    const char *slash = strchr(p, '/');
    if (slash) {
        size_t hlen = (size_t)(slash - p);
        if (hlen >= sizeof(hostname)) hlen = sizeof(hostname) - 1;
        memcpy(hostname, p, hlen);
        hostname[hlen] = '\0';
        snprintf(path, sizeof(path), "%s", slash);
    } else {
        snprintf(hostname, sizeof(hostname), "%s", p);
    }
    if (!*hostname) return abs_new_error("Invalid URL");

#ifdef _WIN32
    int sock_fd = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) return abs_new_error("Socket creation failed");
#else
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) return abs_new_error("Socket creation failed");
#endif

    struct hostent *he = gethostbyname(hostname);
    if (!he) {
#ifdef _WIN32
        closesocket((SOCKET)sock_fd);
#else
        close(sock_fd);
#endif
        return abs_new_error("Host not found");
    }
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(80);
    memcpy(&server.sin_addr, he->h_addr_list[0], (size_t)he->h_length);
    if (connect(sock_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
#ifdef _WIN32
        closesocket((SOCKET)sock_fd);
#else
        close(sock_fd);
#endif
        return abs_new_error("Connect failed");
    }

    char req[2048];
    snprintf(req, sizeof(req),
             "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: AbscomC\r\n\r\n",
             path, hostname);
#ifdef _WIN32
    send((SOCKET)sock_fd, req, (int)strlen(req), 0);
#else
    send(sock_fd, req, strlen(req), 0);
#endif

    abs_string_t resp;
    abs_string_init(&resp);
    char buf[1024];
    int received;
#ifdef _WIN32
    while ((received = recv((SOCKET)sock_fd, buf, (int)sizeof(buf) - 1, 0)) > 0) {
#else
    while ((received = (int)recv(sock_fd, buf, sizeof(buf) - 1, 0)) > 0) {
#endif
        buf[received] = '\0';
        abs_string_append_cstr(&resp, buf);
    }
#ifdef _WIN32
    closesocket((SOCKET)sock_fd);
#else
    close(sock_fd);
#endif

    const char *body = strstr(abs_string_c_str(&resp), "\r\n\r\n");
    var res = body ? abs_new_str(body + 4) : abs_new_str(abs_string_c_str(&resp));
    abs_string_destroy(&resp);
    return res;
}

bool set_contains(var set_obj, var item) {
    if (!set_obj || !item) return false;
    for (size_t i = 0; i < set_obj->val.list.size; i++) {
        var it = set_obj->val.list.items[i];
        if (!it || it->type != item->type) continue;
        if (is_num(it) && is_num(item)) {
            if (num_val(it) == num_val(item)) return true;
        } else if (it->type == ABS_STR) {
            if (strcmp(it->val.s, item->val.s) == 0) return true;
        } else if (it->type == ABS_BOOL) {
            if (it->val.b == item->val.b) return true;
        } else if (it->type == ABS_NONE) {
            return true;
        } else if (it->type == ABS_LIST || it->type == ABS_SET) {
            var r = eq(it, item);
            if (r && r->type == ABS_BOOL && r->val.b) return true;
        }
    }
    return false;
}

void set_add(var set_obj, var item) {
    if (!set_obj || set_obj->type != ABS_SET || !item) return;
    if (set_contains(set_obj, item)) return;
    if (set_obj->val.list.size >= set_obj->val.list.capacity) {
        size_t new_cap = set_obj->val.list.capacity * 2;
        if (new_cap < set_obj->val.list.capacity) return;
        var *items = (var *)realloc(set_obj->val.list.items, new_cap * sizeof(var));
        if (!items) return;
        set_obj->val.list.items = items;
        set_obj->val.list.capacity = new_cap;
    }
    set_obj->val.list.items[set_obj->val.list.size++] = item;
}

var set_union(var a, var b) {
    var res = abs_new_set();
    if (!res) return None;
    if (a && a->type == ABS_SET)
        for (size_t i = 0; i < a->val.list.size; i++) set_add(res, a->val.list.items[i]);
    if (b && b->type == ABS_SET)
        for (size_t i = 0; i < b->val.list.size; i++) set_add(res, b->val.list.items[i]);
    return res;
}

var set_diff(var a, var b) {
    var res = abs_new_set();
    if (!res) return None;
    if (!a || a->type != ABS_SET) return res;
    for (size_t i = 0; i < a->val.list.size; i++) {
        if (!(b && b->type == ABS_SET && set_contains(b, a->val.list.items[i])))
            set_add(res, a->val.list.items[i]);
    }
    return res;
}

var list_comp(var list, AbsMapFunc mapf, AbsFilterFunc filterf) {
    if (!list || (list->type != ABS_LIST && list->type != ABS_SET))
        return abs_new_list();
    var res = abs_new_list();
    if (!res) return None;
    for (size_t i = 0; i < list->val.list.size; i++) {
        var item = list->val.list.items[i];
        if (filterf == NULL || filterf(item))
            append(res, mapf ? mapf(item) : item);
    }
    return res;
}

var range_step(int start, int stop, int step) {
    var l = abs_new_list();
    if (!l) return None;
    if (step == 0) return l;
    if (step > 0) {
        for (int i = start; i < stop; i += step) append(l, abs_new_int(i));
    } else {
        for (int i = start; i > stop; i += step) append(l, abs_new_int(i));
    }
    return l;
}
