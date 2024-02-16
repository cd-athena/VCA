/*****************************************************************************
 * Copyright (C) 2024 Christian Doppler Laboratory ATHENA
 *
 * Authors: Loren Merritt <lorenm@u.washington.edu>
 *          Steve Borho <steve@borho.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#pragma once

#include <vcaLib.h>

#define VCA_CPU_SSE2 (1 << 0)
#define VCA_CPU_SSSE3 (1 << 1)
#define VCA_CPU_SSE4 (1 << 2)
#define VCA_CPU_AVX2 (1 << 3)

// from primitives.cpp
#if ENABLE_NASM
extern "C" void vca_cpu_emms(void);
#endif

#if _MSC_VER
#include <mmintrin.h>
#define vca_emms() _mm_empty()
#elif __GNUC__
// Cannot use _mm_empty() directly without compiling all the source with
// a fixed CPU arch, which we would like to avoid at the moment
#define vca_emms() vca_cpu_emms()
#else
#define vca_emms() vca_cpu_emms()
#endif

namespace vca {

CpuSimd cpuDetectMaxSimd();

bool isSimdSupported(CpuSimd simd);

struct cpu_name_t
{
    char name[16];
    uint32_t flags;
};

} // namespace vca
