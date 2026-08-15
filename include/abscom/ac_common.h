#ifndef ABSCOM_AC_COMMON_H
#define ABSCOM_AC_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(AC_BUILDING_LIBRARY)
#    define AC_API __declspec(dllexport)
#  elif defined(AC_USE_LIBRARY)
#    define AC_API __declspec(dllimport)
#  else
#    define AC_API
#  endif
#elif defined(__GNUC__) || defined(__clang__)
#  define AC_API __attribute__((visibility("default")))
#else
#  define AC_API
#endif

#ifdef __cplusplus
#  define AC_BEGIN_C_DECLS extern "C" {
#  define AC_END_C_DECLS }
#else
#  define AC_BEGIN_C_DECLS
#  define AC_END_C_DECLS
#endif

#define AC_UNUSED(x) ((void)(x))

#endif
