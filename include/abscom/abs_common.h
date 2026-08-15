#ifndef ABSCOM_ABS_COMMON_H
#define ABSCOM_ABS_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(ABS_BUILDING_LIBRARY)
#    define ABS_API __declspec(dllexport)
#  elif defined(ABS_USE_LIBRARY)
#    define ABS_API __declspec(dllimport)
#  else
#    define ABS_API
#  endif
#elif defined(__GNUC__) || defined(__clang__)
#  define ABS_API __attribute__((visibility("default")))
#else
#  define ABS_API
#endif

#ifdef __cplusplus
#  define ABS_BEGIN_C_DECLS extern "C" {
#  define ABS_END_C_DECLS }
#else
#  define ABS_BEGIN_C_DECLS
#  define ABS_END_C_DECLS
#endif

#define ABS_UNUSED(x) ((void)(x))

#endif
