#ifndef ABSCOM_AC_FS_H
#define ABSCOM_AC_FS_H

#include "ac_common.h"

AC_BEGIN_C_DECLS

AC_API int ac_fs_exists(const char *path);
AC_API int ac_fs_read_file(const char *path, char **out_data, size_t *out_size);
AC_API int ac_fs_write_file(const char *path, const void *data, size_t size);
AC_API int ac_fs_remove(const char *path);
AC_API int ac_fs_rename(const char *from, const char *to);

AC_END_C_DECLS

#endif
