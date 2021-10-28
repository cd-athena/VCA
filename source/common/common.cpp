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

#include "common.h"
#include "threading.h"
#include "vca.h"

#if _WIN32
#include <sys/types.h>
#include <sys/timeb.h>
#include <io.h>
#include <fcntl.h>
#else
#include <sys/time.h>
#endif

namespace VCA_NS {

#if CHECKED_BUILD || _DEBUG
int g_checkFailures;
#endif

int64_t vca_mdate(void)
{
#if _WIN32
    struct timeb tb;
    ftime(&tb);
    return ((int64_t)tb.time * 1000 + (int64_t)tb.millitm) * 1000;
#else
    struct timeval tv_date;
    gettimeofday(&tv_date, NULL);
    return (int64_t)tv_date.tv_sec * 1000000 + (int64_t)tv_date.tv_usec;
#endif
}

#define VCA_ALIGNBYTES 64

#if _WIN32
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
#define _aligned_malloc __mingw_aligned_malloc
#define _aligned_free   __mingw_aligned_free
#include "malloc.h"
#endif

void *vca_malloc(size_t size)
{
    return _aligned_malloc(size, VCA_ALIGNBYTES);
}

void vca_free(void *ptr)
{
    if (ptr) _aligned_free(ptr);
}

#else // if _WIN32
void *vca_malloc(size_t size)
{
    void *ptr;

    if (posix_memalign((void**)&ptr, VCA_ALIGNBYTES, size) == 0)
        return ptr;
    else
        return NULL;
}

void vca_free(void *ptr)
{
    if (ptr) free(ptr);
}

#endif // if _WIN32

void general_log(const vca_param* param, const char* caller, int level, const char* fmt, ...)
{
    if (param && level > param->logLevel)
        return;
    const int bufferSize = 4096;
    char buffer[bufferSize];
    int p = 0;
    const char* log_level;
    switch (level)
    {
    case VCA_LOG_ERROR:
        log_level = "error";
        break;
    case VCA_LOG_WARNING:
        log_level = "warning";
        break;
    case VCA_LOG_INFO:
        log_level = "info";
        break;
    case VCA_LOG_DEBUG:
        log_level = "debug";
        break;
    case VCA_LOG_FULL:
        log_level = "full";
        break;
    default:
        log_level = "unknown";
        break;
    }

    if (caller)
        p += sprintf(buffer, "%-4s [%s]: ", caller, log_level);
    va_list arg;
    va_start(arg, fmt);
    vsnprintf(buffer + p, bufferSize - p, fmt, arg);
    va_end(arg);
    fputs(buffer, stderr);
}

#if _WIN32
/* For Unicode filenames in Windows we convert UTF-8 strings to UTF-16 and we use _w functions.
 * For other OS we do not make any changes. */
void general_log_file(const vca_param* param, const char* caller, int level, const char* fmt, ...)
{
    if (param && level > param->logLevel)
        return;
    const int bufferSize = 4096;
    char buffer[bufferSize];
    int p = 0;
    const char* log_level;
    switch (level)
    {
    case VCA_LOG_ERROR:
        log_level = "error";
        break;
    case VCA_LOG_WARNING:
        log_level = "warning";
        break;
    case VCA_LOG_INFO:
        log_level = "info";
        break;
    case VCA_LOG_DEBUG:
        log_level = "debug";
        break;
    case VCA_LOG_FULL:
        log_level = "full";
        break;
    default:
        log_level = "unknown";
        break;
    }

    if (caller)
        p += sprintf(buffer, "%-4s [%s]: ", caller, log_level);
    va_list arg;
    va_start(arg, fmt);
    vsnprintf(buffer + p, bufferSize - p, fmt, arg);
    va_end(arg);

    HANDLE console = GetStdHandle(STD_ERROR_HANDLE);
    DWORD mode;
    if (GetConsoleMode(console, &mode))
    {
        wchar_t buf_utf16[bufferSize];
        int length_utf16 = MultiByteToWideChar(CP_UTF8, 0, buffer, -1, buf_utf16, sizeof(buf_utf16)/sizeof(wchar_t)) - 1;
        if (length_utf16 > 0)
            WriteConsoleW(console, buf_utf16, length_utf16, &mode, NULL);
    }
    else
        fputs(buffer, stderr);
}

FILE* vca_fopen(const char* fileName, const char* mode)
{
    wchar_t buf_utf16[MAX_PATH * 2], mode_utf16[16];

    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, fileName, -1, buf_utf16, sizeof(buf_utf16)/sizeof(wchar_t)) &&
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, mode, -1, mode_utf16, sizeof(mode_utf16)/sizeof(wchar_t)))
    {
        return _wfopen(buf_utf16, mode_utf16);
    }
    return NULL;
}

#endif

uint32_t vca_picturePlaneSize(int csp, int width, int height, int plane)
{
    uint32_t size = (uint32_t)(width >> vca_cli_csps[csp].width[plane]) * (height >> vca_cli_csps[csp].height[plane]);

    return size;
}


}
