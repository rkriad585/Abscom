#include "abscom/abs.h"

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

jmp_buf abs_env_stack[ABS_ENV_STACK_SIZE];
int abs_env_idx = 0;
var abs_last_error = NULL;

jmp_buf *abs_push_jmp(void) {
    if (abs_env_idx >= ABS_ENV_STACK_SIZE) {
        fprintf(stderr, "Abscom: too many nested try blocks (limit %d)\n",
                ABS_ENV_STACK_SIZE);
        exit(1);
    }
    return &abs_env_stack[abs_env_idx++];
}

void abs_pop_jmp(void) {
    if (abs_env_idx > 0) abs_env_idx--;
}

void throw(const char *msg) {
    abs_last_error = abs_new_error(msg ? msg : "exception");
    if (abs_env_idx == 0) {
        fprintf(stderr, "Abscom: uncaught exception: %s\n",
                msg ? msg : "exception");
        exit(1);
    }
    longjmp(abs_env_stack[abs_env_idx - 1], 1);
}

void close_resource(var obj) {
    if (!obj) return;
    switch (obj->type) {
        case ABS_FILE:
            if (obj->val.file_ptr) {
                fclose(obj->val.file_ptr);
                obj->val.file_ptr = NULL; /* avoid double close */
            }
            break;
        default:
            break;
    }
}
