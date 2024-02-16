/*****************************************************************************
 * Copyright (C) 2024 Christian Doppler Laboratory ATHENA
 *
 * Authors: Loren Merritt <lorenm@u.washington.edu>
 *          Laurent Aimar <fenrir@via.ecp.fr>
 *          Fiona Glaser <fiona@x264.com>
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

#include "cpu.h"

#include <analyzer/common/common.h>

#if MACOS || SYS_FREEBSD
#include <sys/sysctl.h>
#include <sys/types.h>

#endif
#if SYS_OPENBSD
#include <machine/cpu.h>
#include <sys/param.h>
#include <sys/sysctl.h>

#endif

namespace vca {

#if VCA_ARCH_X86

extern "C" {
#ifdef __INTEL_COMPILER

/* Agner's patch to Intel's CPU dispatcher from pages 131-132 of
 * http://agner.org/optimize/optimizing_cpp.pdf (2011-01-30)
 * adapted to vca's cpu schema. */

// Global variable indicating cpu
int __intel_cpu_indicator = 0;
// CPU dispatcher function
void vca_intel_cpu_indicator_init(void)
{
    uint32_t cpu = vca::cpu_detect();
    if (cpu & VCA_CPU_AVX)
        __intel_cpu_indicator = 0x20000;
    else if (cpu & VCA_CPU_SSE42)
        __intel_cpu_indicator = 0x8000;
    else if (cpu & VCA_CPU_SSE4)
        __intel_cpu_indicator = 0x2000;
    else if (cpu & VCA_CPU_SSSE3)
        __intel_cpu_indicator = 0x1000;
    else if (cpu & VCA_CPU_SSE3)
        __intel_cpu_indicator = 0x800;
    else if (cpu & VCA_CPU_SSE2 && !(cpu & VCA_CPU_SSE2_IS_SLOW))
        __intel_cpu_indicator = 0x200;
    else if (cpu & VCA_CPU_SSE)
        __intel_cpu_indicator = 0x80;
    else if (cpu & VCA_CPU_MMX2)
        __intel_cpu_indicator = 8;
    else
        __intel_cpu_indicator = 1;
}

/* __intel_cpu_indicator_init appears to have a non-standard calling convention that
 * assumes certain registers aren't preserved, so we'll route it through a function
 * that backs up all the registers. */
void __intel_cpu_indicator_init(void)
{
    vca_safe_intel_cpu_indicator_init();
}

#else // ifdef __INTEL_COMPILER
void vca_intel_cpu_indicator_init(void) {}

#endif // ifdef __INTEL_COMPILER
}

#if ENABLE_NASM
extern "C" {
/* cpu-a.asm */
int vca_cpu_cpuid_test(void);
void vca_cpu_cpuid(uint32_t op, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx);
uint64_t vca_cpu_xgetbv(int xcr);
}
#endif

#if defined(_MSC_VER)
#pragma warning(disable : 4309) // truncation of constant value
#endif

bool isSimdSupported(CpuSimd simd)
{
    const auto simdLevelIndex             = CpuSimdMapper.indexOf(simd);
    const auto maxSupportedSimdLevelIndex = CpuSimdMapper.indexOf(cpuDetectMaxSimd());
    return maxSupportedSimdLevelIndex >= simdLevelIndex;
}

CpuSimd cpuDetectMaxSimd()
{
    auto cpu = CpuSimd::SSSE3;
#if ENABLE_NASM
    uint32_t eax, ebx, ecx, edx;
    uint32_t vendor[4] = {0};
    uint32_t max_basic_cap;
    uint64_t xcr0 = 0;

#if !X86_64
    if (!vca_cpu_cpuid_test())
        return 0;
#endif

    vca_cpu_cpuid(0, &max_basic_cap, vendor + 0, vendor + 2, vendor + 1);
    if (max_basic_cap == 0)
        return cpu;

    vca_cpu_cpuid(1, &eax, &ebx, &ecx, &edx);
    if (!(edx & 0x00800000))
        // Not even mmx supported
        return cpu;
    if (edx & 0x04000000)
        cpu = CpuSimd::SSE2;
    if (ecx & 0x00000200)
        cpu = CpuSimd::SSSE3;
    if (ecx & 0x00080000)
        cpu = CpuSimd::SSE4;

    if (max_basic_cap >= 7)
    {
        xcr0 = vca_cpu_xgetbv(0);
        vca_cpu_cpuid(7, &eax, &ebx, &ecx, &edx);

        if ((xcr0 & 0x6) == 0x6) /* XMM/YMM state */
        {
            if (ebx & 0x00000020)
                cpu = CpuSimd::AVX2;
        }
    }
#endif
    return cpu;
}

#else
CpuSimd cpuDetectMaxSimd()
{
    return CpuSimd::None;
}
#endif // if VCA_ARCH_X86
} // namespace vca
