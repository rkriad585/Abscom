#include "abscom/abs.h"

static var gen_range_next(var gen) {
    if (!gen || gen->type != ABS_GENERATOR) return NULL;
    long s = gen->val.gen.state;
    long step = gen->val.gen.step;
    if (step > 0) {
        if (s >= gen->val.gen.limit) return NULL;
    } else {
        if (s <= gen->val.gen.limit) return NULL;
    }
    var res = abs_new_int(s);
    gen->val.gen.state = s + step;
    return res;
}

var range_gen(long start, long stop, long step) {
    if (step == 0) return abs_new_error("range_gen step cannot be zero");
    var o = abs_new_obj(ABS_GENERATOR);
    if (!o) return None;
    o->val.gen.func = gen_range_next;
    o->val.gen.state = start;
    o->val.gen.limit = stop;
    o->val.gen.step = step;
    return o;
}

var next(var gen) {
    if (!gen || gen->type != ABS_GENERATOR)
        return abs_new_error("next() expects a generator");
    var res = gen->val.gen.func ? gen->val.gen.func(gen) : NULL;
    if (res == NULL) return None;
    return res;
}
