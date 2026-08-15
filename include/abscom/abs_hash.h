#ifndef ABSCOM_ABS_HASH_H
#define ABSCOM_ABS_HASH_H

#include "abs_common.h"

ABS_BEGIN_C_DECLS

ABS_API uint32_t abs_hash_fnv1a32(const void *data, size_t len);
ABS_API uint64_t abs_hash_fnv1a64(const void *data, size_t len);
ABS_API uint64_t abs_hash_fnv1a64_str(const char *str);
ABS_API uint32_t abs_hash_djb2(const char *str);

ABS_END_C_DECLS

#endif
