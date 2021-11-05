/*****************************************************************************
 * Copyright (C) 2021 Christian Doppler Laboratory ATHENA
 *
 * Authors: Steve Borho <steve@borho.org>
 *          Praveen Kumar Tiwari <praveen@multicorewareinc.com>
 *          Min Chen <chenm003@163.com> <min.chen@multicorewareinc.com>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at license @ x265.com.
 *****************************************************************************/

#include "common.h"
#include "primitives.h"
#include "vca.h"
#include "cpu.h"

extern "C" {
    #include "dct8.h"
}

#define ALL_LUMA_TU_TYPED_S(prim, fncdef, fname, cpu) \
    p.cu[BLOCK_4x4].prim   = fncdef vca_ ## fname ## 4_ ## cpu; \
    p.cu[BLOCK_8x8].prim   = fncdef vca_ ## fname ## 8_ ## cpu; \
    p.cu[BLOCK_16x16].prim = fncdef vca_ ## fname ## 16_ ## cpu; \
    p.cu[BLOCK_32x32].prim = fncdef vca_ ## fname ## 32_ ## cpu

#define ALL_LUMA_TU_S(prim, fname, cpu)    ALL_LUMA_TU_TYPED_S(prim, , fname, cpu)

namespace VCA_NS {

#if HIGH_BIT_DEPTH
    void setupAssemblyPrimitives(AnalyzerPrimitives &p, int cpuMask)
    {
        if (cpuMask & VCA_CPU_SSE2)
        {
            p.cu[BLOCK_4x4].dct = vca_dct4_sse2;
            p.cu[BLOCK_8x8].dct = vca_dct8_sse2;
        }
        if (cpuMask & VCA_CPU_SSE4)
        {
            p.cu[BLOCK_8x8].dct = vca_dct8_sse4;
        }
#if X86_64
        if (cpuMask & VCA_CPU_AVX2)
        {
            ALL_LUMA_TU_S(dct, dct, avx2);
        }
#endif
    }
#else
    void setupAssemblyPrimitives(AnalyzerPrimitives &p, int cpuMask)
    {
        if (cpuMask & VCA_CPU_SSE2)
        {
            p.cu[BLOCK_4x4].dct = vca_dct4_sse2;
            p.cu[BLOCK_8x8].dct = vca_dct8_sse2;
        }
        if (cpuMask & VCA_CPU_SSE4)
        {
            p.cu[BLOCK_8x8].dct = vca_dct8_sse4;
        }
#if X86_64
        if (cpuMask & VCA_CPU_AVX2)
        {
            ALL_LUMA_TU_S(dct, dct, avx2);
        }
#endif
    }
#endif
}

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
