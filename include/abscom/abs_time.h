#ifndef ABSCOM_ABS_TIME_H
#define ABSCOM_ABS_TIME_H

#include "abs_common.h"

ABS_BEGIN_C_DECLS

ABS_API double abs_time_now(void);
ABS_API double abs_time_wall(void);
ABS_API uint64_t abs_time_now_ns(void);
ABS_API uint64_t abs_time_wall_ms(void);

ABS_END_C_DECLS

#endif
