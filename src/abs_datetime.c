#include "abscom/abs.h"

#include <time.h>

var datetime_now(void) {
    var o = abs_new_obj(ABS_TIME);
    if (!o) return None;
    time_t raw = time(NULL);
    o->val.time_data.tm_val = *localtime(&raw);
    return o;
}

var strftime_val(const char *fmt, var time_obj) {
    if (!fmt || !time_obj || time_obj->type != ABS_TIME) return None;
    char buf[256];
    strftime(buf, sizeof(buf), fmt, &time_obj->val.time_data.tm_val);
    return abs_new_str(buf);
}

var timedelta(int days, int seconds) {
    var o = abs_new_obj(ABS_TIME);
    if (!o) return None;
    time_t raw = time(NULL) + (time_t)days * 86400 + seconds;
    o->val.time_data.tm_val = *localtime(&raw);
    return o;
}
