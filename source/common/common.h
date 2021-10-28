/*****************************************************************************
 * Copyright (C) 2021 Christian Doppler Laboratory ATHENA
 *
 * Authors: Vignesh V Menon <vignesh.menon@aau.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 *****************************************************************************/

#ifndef VCA_COMMON_H
#define VCA_COMMON_H

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <ctime>

#include <stdint.h>
#include <memory.h>
#include <assert.h>

#include "vca.h"

#if ENABLE_PPA && ENABLE_VTUNE
#error "PPA and VTUNE cannot both be enabled. Disable one of them."
#endif
#if ENABLE_PPA
#include "profile/PPA/ppa.h"
#define ProfileScopeEvent(x) PPAScopeEvent(x)
#define THREAD_NAME(n,i)
#define PROFILE_INIT()       PPA_INIT()
#define PROFILE_PAUSE()
#define PROFILE_RESUME()
#elif ENABLE_VTUNE
#include "profile/vtune/vtune.h"
#define ProfileScopeEvent(x) VTuneScopeEvent _vtuneTask(x)
#define THREAD_NAME(n,i)     vtuneSetThreadName(n, i)
#define PROFILE_INIT()       vtuneInit()
#define PROFILE_PAUSE()      __itt_pause()
#define PROFILE_RESUME()     __itt_resume()
#else
#define ProfileScopeEvent(x)
#define THREAD_NAME(n,i)
#define PROFILE_INIT()
#define PROFILE_PAUSE()
#define PROFILE_RESUME()
#endif

#if defined(__GNUC__)
#define ALIGN_VAR_4(T, var)  T var __attribute__((aligned(4)))
#define ALIGN_VAR_8(T, var)  T var __attribute__((aligned(8)))
#define ALIGN_VAR_16(T, var) T var __attribute__((aligned(16)))
#define ALIGN_VAR_32(T, var) T var __attribute__((aligned(32)))
#define ALIGN_VAR_64(T, var) T var __attribute__((aligned(64)))
#if defined(__MINGW32__)
#define fseeko fseeko64
#define ftello ftello64
#endif
#elif defined(_MSC_VER)

#define ALIGN_VAR_32(T, var) __declspec(align(32)) T var
#define fseeko _fseeki64
#define ftello _ftelli64
#endif // if defined(__GNUC__)
#if HAVE_INT_TYPES_H
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#define VCA_LL "%" PRIu64
#else
#define VCA_LL "%lld"
#endif

#if _DEBUG && defined(_MSC_VER)
#define DEBUG_BREAK() __debugbreak()
#elif __APPLE_CC__
#define DEBUG_BREAK() __builtin_trap()
#else
#define DEBUG_BREAK() abort()
#endif

/* If compiled with CHECKED_BUILD perform run-time checks and log any that
 * fail, both to stderr and to a file */
#if CHECKED_BUILD || _DEBUG
namespace VCA_NS { extern int g_checkFailures; }
#define VCA_CHECK(expr, ...) if (!(expr)) { \
    vca_log(NULL, VCA_LOG_ERROR, __VA_ARGS__); \
    FILE *fp = fopen("vca_check_failures.txt", "a"); \
    if (fp) { fprintf(fp, "%s:%d\n", __FILE__, __LINE__); fprintf(fp, __VA_ARGS__); fclose(fp); } \
    g_checkFailures++; DEBUG_BREAK(); \
}
#if _MSC_VER
#pragma warning(disable: 4127) // some checks have constant conditions
#endif
#else
#define VCA_CHECK(expr, ...)
#endif

#if HIGH_BIT_DEPTH
typedef uint16_t pixel;
#else
typedef uint8_t  pixel;
#endif // if HIGH_BIT_DEPTH

#ifndef NULL
#define NULL 0
#endif

#define MAX_UINT                 0xFFFFFFFFU // max. value of unsigned 32-bit integer
#define MAX_INT                  2147483647  // max. value of signed 32-bit integer
#define MAX_INT64                0x7FFFFFFFFFFFFFFFLL  // max. value of signed 64-bit integer
#define MAX_DOUBLE               1.7e+308    // max. value of double-type value

#define VCA_NEW_SHOT             255
#define VCA_NOTSURE_NEW_SHOT     128
#define VCA_NOT_NEW_SHOT         0

#define MAX_LOG2_CU_SIZE         6                           // log2(maxCUSize)
#define MAX_CU_SIZE              (1 << MAX_LOG2_CU_SIZE)     // maximum allowable size of CU
#define CHROMA_H_SHIFT(x)        (x == VCA_CSP_I420 || x == VCA_CSP_I422)
#define CHROMA_V_SHIFT(x)        (x == VCA_CSP_I420)

template<typename T>
inline T vca_min(T a, T b) { return a < b ? a : b; }

template<typename T>
inline T vca_max(T a, T b) { return a > b ? a : b; }

template<typename T>
inline T vca_clip3(T minVal, T maxVal, T a) { return vca_min(vca_max(minVal, a), maxVal); }

template<typename T> /* clip to pixel range, 0..255 or 0..1023 */
inline pixel vca_clip(T x) { return (pixel)vca_min<T>(T((1 << VCA_DEPTH) - 1), vca_max<T>(T(0), x)); }

typedef int16_t  coeff_t;      // transform coefficient

#define VCA_MIN(a, b) ((a) < (b) ? (a) : (b))
#define VCA_MAX(a, b) ((a) > (b) ? (a) : (b))

#define VCA_MIN3(a, b, c) VCA_MIN((a), VCA_MIN((b), (c)))

#define VCA_MALLOC(type, count)    (type*)vca_malloc(sizeof(type) * (count))
#define VCA_FREE(ptr)              vca_free(ptr)
#define VCA_FREE_ZERO(ptr)         vca_free(ptr); (ptr) = NULL
#define CHECKED_MALLOC(var, type, count) \
    { \
        var = (type*)vca_malloc(sizeof(type) * (count)); \
        if (!var) \
        { \
            vca_log(NULL, VCA_LOG_ERROR, "malloc of size %d failed\n", sizeof(type) * (count)); \
            goto fail; \
        } \
    }
#define CHECKED_MALLOC_ZERO(var, type, count) \
    { \
        var = (type*)vca_malloc(sizeof(type) * (count)); \
        if (var) \
            memset((void*)var, 0, sizeof(type) * (count)); \
        else \
        { \
            vca_log(NULL, VCA_LOG_ERROR, "malloc of size %d failed\n", sizeof(type) * (count)); \
            goto fail; \
        } \
    }

namespace VCA_NS {

/* located in common.cpp */
int64_t  vca_mdate(void);
#define  vca_log(param, ...) general_log(param, "vca", __VA_ARGS__)
#define  vca_log_file(param, ...) general_log_file(param, "vca", __VA_ARGS__)
void     general_log(const vca_param* param, const char* caller, int level, const char* fmt, ...);
#if _WIN32
void     general_log_file(const vca_param* param, const char* caller, int level, const char* fmt, ...);
FILE*    vca_fopen(const char* fileName, const char* mode);
#else
#define  general_log_file(param, caller, level, fmt, ...) general_log(param, caller, level, fmt, __VA_ARGS__)
#define  vca_fopen(fileName, mode) fopen(fileName, mode)
#define  vca_unlink(fileName) unlink(fileName)
#define  vca_rename(oldName, newName) rename(oldName, newName)
#endif
uint32_t vca_picturePlaneSize(int csp, int width, int height, int plane);

void*    vca_malloc(size_t size);
void     vca_free(void *ptr);

/* located in primitives.cpp */
void     vca_setup_primitives(vca_param* param);
void     vca_report_simd(vca_param* param);
}

#include "constants.h"

#endif // ifndef VCA_COMMON_H
