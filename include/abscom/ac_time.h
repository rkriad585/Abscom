#ifndef ABSCOM_AC_TIME_H
#define ABSCOM_AC_TIME_H

#include "ac_common.h"

AC_BEGIN_C_DECLS

AC_API double ac_time_now(void);
AC_API double ac_time_wall(void);
AC_API uint64_t ac_time_now_ns(void);
AC_API uint64_t ac_time_wall_ms(void);

AC_END_C_DECLS

#endif
