#ifndef ABSCOM_ABS_FS_H
#define ABSCOM_ABS_FS_H

#include "abs_common.h"

ABS_BEGIN_C_DECLS

ABS_API int abs_fs_exists(const char *path);
ABS_API int abs_fs_read_file(const char *path, char **out_data, size_t *out_size);
ABS_API int abs_fs_write_file(const char *path, const void *data, size_t size);
ABS_API int abs_fs_remove(const char *path);
ABS_API int abs_fs_rename(const char *from, const char *to);

ABS_END_C_DECLS

#endif
