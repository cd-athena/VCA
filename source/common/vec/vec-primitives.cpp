/*****************************************************************************
 * Copyright (C) 2021 Christian Doppler Laboratory ATHENA
 *
 * Authors: Steve Borho <steve@borho.org>
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

#include "primitives.h"
#include "vca.h"

/* The #if logic here must match the file lists in CMakeLists.txt */
#if VCA_ARCH_X86
#if defined(__INTEL_COMPILER)
#define HAVE_SSE3
#define HAVE_SSSE3
#define HAVE_SSE4
#define HAVE_AVX2
#elif defined(__GNUC__)
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if __clang__ || GCC_VERSION >= 40300 /* gcc_version >= gcc-4.3.0 */
#define HAVE_SSE3
#define HAVE_SSSE3
#define HAVE_SSE4
#endif
#if __clang__ || GCC_VERSION >= 40700 /* gcc_version >= gcc-4.7.0 */
#define HAVE_AVX2
#endif
#elif defined(_MSC_VER)
#define HAVE_SSE3
#define HAVE_SSSE3
#define HAVE_SSE4
#if _MSC_VER >= 1700 // VC11
#define HAVE_AVX2
#endif
#endif // compiler checks
#endif // if VCA_ARCH_X86

namespace VCA_NS {

void setupIntrinsicDCT_sse3(AnalyzerPrimitives&);
void setupIntrinsicDCT_ssse3(AnalyzerPrimitives&);
void setupIntrinsicDCT_sse41(AnalyzerPrimitives&);

/* Use primitives for the best available vector architecture */
void setupInstrinsicPrimitives(AnalyzerPrimitives &p, int cpuMask)
{
#ifdef HAVE_SSE3
    if (cpuMask & VCA_CPU_SSE3)
    {
        setupIntrinsicDCT_sse3(p);
    }
#endif
#ifdef HAVE_SSSE3
    if (cpuMask & VCA_CPU_SSSE3)
    {
        setupIntrinsicDCT_ssse3(p);
    }
#endif
    (void)p;
    (void)cpuMask;
}
}
