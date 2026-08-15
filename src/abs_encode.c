#include "abscom/abs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char b64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

var base64_encode(var str_obj) {
    if (!str_obj || str_obj->type != ABS_STR) return None;
    const unsigned char *src = (const unsigned char *)str_obj->val.s;
    size_t len = strlen(str_obj->val.s);
    size_t out_len = 4 * ((len + 2) / 3);
    char *out = (char *)malloc(out_len + 1);
    if (!out) return None;
    size_t oi = 0;
    for (size_t i = 0; i < len; i += 3) {
        unsigned v = (unsigned)src[i] << 16;
        if (i + 1 < len) v |= (unsigned)src[i + 1] << 8;
        if (i + 2 < len) v |= (unsigned)src[i + 2];
        out[oi++] = b64_table[(v >> 18) & 0x3F];
        out[oi++] = b64_table[(v >> 12) & 0x3F];
        out[oi++] = (i + 1 < len) ? b64_table[(v >> 6) & 0x3F] : '=';
        out[oi++] = (i + 2 < len) ? b64_table[v & 0x3F] : '=';
    }
    out[oi] = '\0';
    var res = abs_new_str(out);
    free(out);
    return res;
}

var uuid4(void) {
    char buf[37];
    sprintf(buf, "%04x%04x-%04x-%04x-%04x-%04x%04x%04x",
            (unsigned)rand() & 0xFFFF, (unsigned)rand() & 0xFFFF,
            (unsigned)rand() & 0xFFFF,
            ((unsigned)rand() & 0x0FFF) | 0x4000, /* version 4 */
            ((unsigned)rand() & 0x3FFF) | 0x8000, /* variant */
            (unsigned)rand() & 0xFFFF, (unsigned)rand() & 0xFFFF,
            (unsigned)rand() & 0xFFFF);
    return abs_new_str(buf);
}
