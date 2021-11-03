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

namespace VCA_NS {

extern const uint8_t lumaPartitionMapTable[] =
{
//  4          8          12          16          20  24          28  32          36  40  44  48          52  56  60  64
    LUMA_4x4,  LUMA_4x8,  255,        LUMA_4x16,  255, 255,        255, 255,        255, 255, 255, 255,        255, 255, 255, 255,        // 4
    LUMA_8x4,  LUMA_8x8,  255,        LUMA_8x16,  255, 255,        255, LUMA_8x32,  255, 255, 255, 255,        255, 255, 255, 255,        // 8
    255,        255,      255,        LUMA_12x16, 255, 255,        255, 255,        255, 255, 255, 255,        255, 255, 255, 255,        // 12
    LUMA_16x4, LUMA_16x8, LUMA_16x12, LUMA_16x16, 255, 255,        255, LUMA_16x32, 255, 255, 255, 255,        255, 255, 255, LUMA_16x64, // 16
    255,        255,      255,        255,        255, 255,        255, 255,        255, 255, 255, 255,        255, 255, 255, 255,        // 20
    255,        255,      255,        255,        255, 255,        255, LUMA_24x32, 255, 255, 255, 255,        255, 255, 255, 255,        // 24
    255,        255,      255,        255,        255, 255,        255, 255,        255, 255, 255, 255,        255, 255, 255, 255,        // 28
    255,        LUMA_32x8, 255,       LUMA_32x16, 255, LUMA_32x24, 255, LUMA_32x32, 255, 255, 255, 255,        255, 255, 255, LUMA_32x64, // 32
    255,        255,      255,        255,        255, 255,        255, 255,        255, 255, 255, 255,        255, 255, 255, 255,        // 36
    255,        255,      255,        255,        255, 255,        255, 255,        255, 255, 255, 255,        255, 255, 255, 255,        // 40
    255,        255,      255,        255,        255, 255,        255, 255,        255, 255, 255, 255,        255, 255, 255, 255,        // 44
    255,        255,      255,        255,        255, 255,        255, 255,        255, 255, 255, 255,        255, 255, 255, LUMA_48x64, // 48
    255,        255,      255,        255,        255, 255,        255, 255,        255, 255, 255, 255,        255, 255, 255, 255,        // 52
    255,        255,      255,        255,        255, 255,        255, 255,        255, 255, 255, 255,        255, 255, 255, 255,        // 56
    255,        255,      255,        255,        255, 255,        255, 255,        255, 255, 255, 255,        255, 255, 255, 255,        // 60
    255,        255,      255,        LUMA_64x16, 255, 255,        255, LUMA_64x32, 255, 255, 255, LUMA_64x48, 255, 255, 255, LUMA_64x64  // 64
};

/* the "authoritative" set of encoder primitives */
EncoderPrimitives primitives;

void setupPixelPrimitives_c(EncoderPrimitives &p);
void setupDCTPrimitives_c(EncoderPrimitives &p);
void setupLowPassPrimitives_c(EncoderPrimitives& p);

void setupCPrimitives(EncoderPrimitives &p)
{
    setupPixelPrimitives_c(p);      // pixel.cpp
    setupDCTPrimitives_c(p);        // dct.cpp
    setupLowPassPrimitives_c(p);    // lowpassdct.cpp
}

void enableLowpassDCTPrimitives(EncoderPrimitives &p)
{
    // update copies of the standard dct transform
    p.cu[BLOCK_4x4].standard_dct = p.cu[BLOCK_4x4].dct;
    p.cu[BLOCK_8x8].standard_dct = p.cu[BLOCK_8x8].dct;
    p.cu[BLOCK_16x16].standard_dct = p.cu[BLOCK_16x16].dct;
    p.cu[BLOCK_32x32].standard_dct = p.cu[BLOCK_32x32].dct;

    // replace active dct by lowpass dct for high dct transforms
    p.cu[BLOCK_16x16].dct = p.cu[BLOCK_16x16].lowpass_dct;
    p.cu[BLOCK_32x32].dct = p.cu[BLOCK_32x32].lowpass_dct;
}

void vca_report_simd(vca_param* param)
{
    if (param->logLevel >= VCA_LOG_INFO)
    {
        int cpuid = param->cpuid;
        char buf[1000];
        char *p = buf + sprintf(buf, "using cpu capabilities:");
        char *none = p;
        for (int i = 0; VCA_NS::cpu_names[i].flags; i++)
        {
            if (!strcmp(VCA_NS::cpu_names[i].name, "SSE")
                && (cpuid & VCA_CPU_SSE2))
                continue;
            if (!strcmp(VCA_NS::cpu_names[i].name, "SSE2")
                && (cpuid & (VCA_CPU_SSE2_IS_FAST | VCA_CPU_SSE2_IS_SLOW)))
                continue;
            if (!strcmp(VCA_NS::cpu_names[i].name, "SSE3")
                && (cpuid & VCA_CPU_SSSE3 || !(cpuid & VCA_CPU_CACHELINE_64)))
                continue;
            if (!strcmp(VCA_NS::cpu_names[i].name, "SSE4.1")
                && (cpuid & VCA_CPU_SSE42))
                continue;
            if (!strcmp(VCA_NS::cpu_names[i].name, "BMI1")
                && (cpuid & VCA_CPU_BMI2))
                continue;
            if ((cpuid & VCA_NS::cpu_names[i].flags) == VCA_NS::cpu_names[i].flags
                && (!i || VCA_NS::cpu_names[i].flags != VCA_NS::cpu_names[i - 1].flags))
                p += sprintf(p, " %s", VCA_NS::cpu_names[i].name);
        }
        if (p == none)
            sprintf(p, " none!");
        vca_log(param, VCA_LOG_INFO, "%s\n", buf);
    }
}

void vca_setup_primitives(vca_param *param)
{
    if (!primitives.cu[BLOCK_4x4].copy_ss)
    {
        setupCPrimitives(primitives);

#if ENABLE_ASSEMBLY
#if VCA_ARCH_X86
        setupInstrinsicPrimitives(primitives, param->cpuid);
#endif
#endif
        if (param->bLowPassDct)
        {
            enableLowpassDCTPrimitives(primitives); 
        }
    }
    vca_report_simd(param);
}
}

#if ENABLE_ASSEMBLY && VCA_ARCH_X86
/* these functions are implemented in assembly. When assembly is not being
 * compiled, they are unnecessary and can be NOPs */
#else
extern "C" {
int vca_cpu_cpuid_test(void) { return 0; }
void vca_cpu_emms(void) {}
void vca_cpu_cpuid(uint32_t, uint32_t *eax, uint32_t *, uint32_t *, uint32_t *) { *eax = 0; }
void vca_cpu_xgetbv(uint32_t, uint32_t *, uint32_t *) {}

}
#endif
