#include "abscom/abs.h"

#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#ifdef _WIN32
static DWORD WINAPI abs_thread_wrapper(LPVOID param) {
    var t = (var)param;
    t->val.thread.result = t->val.thread.func(t->val.thread.arg);
    return 0;
}
#else
static void *abs_thread_wrapper(void *param) {
    var t = (var)param;
    t->val.thread.result = t->val.thread.func(t->val.thread.arg);
    return NULL;
}
#endif

var thread_start(AbsThreadFunc func, var arg) {
    if (!func) return abs_new_error("thread_start requires a function");
    var t = abs_new_obj(ABS_THREAD);
    if (!t) return abs_new_error("Out of memory");
    t->val.thread.func = func;
    t->val.thread.arg = arg;
    t->val.thread.result = None;
    abs_gc_track(t);
#ifdef _WIN32
    DWORD id = 0;
    HANDLE h = CreateThread(NULL, 0, abs_thread_wrapper, t, 0, &id);
    if (!h) return abs_new_error("Failed to create thread");
    t->val.thread.handle = (void *)h;
#else
    pthread_t *pt = (pthread_t *)malloc(sizeof(pthread_t));
    if (!pt) return abs_new_error("Out of memory");
    if (pthread_create(pt, NULL, abs_thread_wrapper, t) != 0) {
        free(pt);
        return abs_new_error("Failed to create thread");
    }
    t->val.thread.handle = (void *)pt;
#endif
    return t;
}

var thread_join(var thread_obj) {
    if (!thread_obj || thread_obj->type != ABS_THREAD)
        return abs_new_error("thread_join expects a thread object");
#ifdef _WIN32
    HANDLE h = (HANDLE)thread_obj->val.thread.handle;
    if (h) WaitForSingleObject(h, INFINITE);
#else
    pthread_t *pt = (pthread_t *)thread_obj->val.thread.handle;
    if (pt) pthread_join(*pt, NULL);
#endif
    return thread_obj->val.thread.result;
}
