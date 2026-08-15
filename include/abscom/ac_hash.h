#ifndef ABSCOM_AC_HASH_H
#define ABSCOM_AC_HASH_H

#include "ac_common.h"

AC_BEGIN_C_DECLS

AC_API uint32_t ac_hash_fnv1a32(const void *data, size_t len);
AC_API uint64_t ac_hash_fnv1a64(const void *data, size_t len);
AC_API uint64_t ac_hash_fnv1a64_str(const char *str);
AC_API uint32_t ac_hash_djb2(const char *str);

AC_END_C_DECLS

#endif
