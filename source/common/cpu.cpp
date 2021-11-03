/*****************************************************************************
 * Copyright (C) 2021 Christian Doppler Laboratory ATHENA
 *
 * Authors: Loren Merritt <lorenm@u.washington.edu>
 *          Laurent Aimar <fenrir@via.ecp.fr>
 *          Fiona Glaser <fiona@x264.com>
 *          Steve Borho <steve@borho.org>
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

#include "cpu.h"

#if MACOS || SYS_FREEBSD
#include <sys/types.h>
#include <sys/sysctl.h>
#endif
#if SYS_OPENBSD
#include <sys/param.h>
#include <sys/sysctl.h>
#include <machine/cpu.h>
#endif

namespace VCA_NS {

const cpu_name_t cpu_names[] =
{
#if VCA_ARCH_X86
#define MMX2 VCA_CPU_MMX | VCA_CPU_MMX2
    { "MMX2",        MMX2 },
    { "MMXEXT",      MMX2 },
    { "SSE",         MMX2 | VCA_CPU_SSE },
#define SSE2 MMX2 | VCA_CPU_SSE | VCA_CPU_SSE2
    { "SSE2Slow",    SSE2 | VCA_CPU_SSE2_IS_SLOW },
    { "SSE2",        SSE2 },
    { "SSE2Fast",    SSE2 | VCA_CPU_SSE2_IS_FAST },
    { "LZCNT", VCA_CPU_LZCNT },
    { "SSE3",        SSE2 | VCA_CPU_SSE3 },
    { "SSSE3",       SSE2 | VCA_CPU_SSE3 | VCA_CPU_SSSE3 },
    { "SSE4.1",      SSE2 | VCA_CPU_SSE3 | VCA_CPU_SSSE3 | VCA_CPU_SSE4 },
    { "SSE4",        SSE2 | VCA_CPU_SSE3 | VCA_CPU_SSSE3 | VCA_CPU_SSE4 },
    { "SSE4.2",      SSE2 | VCA_CPU_SSE3 | VCA_CPU_SSSE3 | VCA_CPU_SSE4 | VCA_CPU_SSE42 },
#define AVX SSE2 | VCA_CPU_SSE3 | VCA_CPU_SSSE3 | VCA_CPU_SSE4 | VCA_CPU_SSE42 | VCA_CPU_AVX
    { "AVX",         AVX },
    { "XOP",         AVX | VCA_CPU_XOP },
    { "FMA4",        AVX | VCA_CPU_FMA4 },
    { "FMA3",        AVX | VCA_CPU_FMA3 },
    { "BMI1",        AVX | VCA_CPU_LZCNT | VCA_CPU_BMI1 },
    { "BMI2",        AVX | VCA_CPU_LZCNT | VCA_CPU_BMI1 | VCA_CPU_BMI2 },
#define AVX2 AVX | VCA_CPU_FMA3 | VCA_CPU_LZCNT | VCA_CPU_BMI1 | VCA_CPU_BMI2 | VCA_CPU_AVX2
    { "AVX2", AVX2},
    { "AVX512", AVX2 | VCA_CPU_AVX512 },
#undef AVX2
#undef AVX
#undef SSE2
#undef MMX2
    { "Cache32",         VCA_CPU_CACHELINE_32 },
    { "Cache64",         VCA_CPU_CACHELINE_64 },
    { "SlowAtom",        VCA_CPU_SLOW_ATOM },
    { "SlowPshufb",      VCA_CPU_SLOW_PSHUFB },
    { "SlowPalignr",     VCA_CPU_SLOW_PALIGNR },
    { "SlowShuffle",     VCA_CPU_SLOW_SHUFFLE },
    { "UnalignedStack",  VCA_CPU_STACK_MOD4 },
#endif // if VCA_ARCH_X86
    { "", 0 },
};

#if VCA_ARCH_X86

extern "C" {
    /* cpu-a.asm */
    int vca_cpu_cpuid_test(void);
    void vca_cpu_cpuid(uint32_t op, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx);
    uint64_t vca_cpu_xgetbv(int xcr);
}

#if defined(_MSC_VER)
#pragma warning(disable: 4309) // truncation of constant value
#endif

uint32_t cpu_detect()
{

    uint32_t cpu = 0; 
    uint32_t eax, ebx, ecx, edx;
    uint32_t vendor[4] = { 0 };
    uint32_t max_extended_cap, max_basic_cap;
    uint64_t xcr0 = 0;

#if !X86_64
    if (!vca_cpu_cpuid_test())
        return 0;
#endif

    vca_cpu_cpuid(0, &max_basic_cap, vendor + 0, vendor + 2, vendor + 1);
    if (max_basic_cap == 0)
        return 0;

    vca_cpu_cpuid(1, &eax, &ebx, &ecx, &edx);
    if (edx & 0x00800000)
        cpu |= VCA_CPU_MMX;
    else
        return cpu;
    if (edx & 0x02000000)
        cpu |= VCA_CPU_MMX2 | VCA_CPU_SSE;
    if (edx & 0x04000000)
        cpu |= VCA_CPU_SSE2;
    if (ecx & 0x00000001)
        cpu |= VCA_CPU_SSE3;
    if (ecx & 0x00000200)
        cpu |= VCA_CPU_SSSE3 | VCA_CPU_SSE2_IS_FAST;
    if (ecx & 0x00080000)
        cpu |= VCA_CPU_SSE4;
    if (ecx & 0x00100000)
        cpu |= VCA_CPU_SSE42;

    if (ecx & 0x08000000) /* XGETBV supported and XSAVE enabled by OS */
    {
        /* Check for OS support */
        xcr0 = vca_cpu_xgetbv(0);
        if ((xcr0 & 0x6) == 0x6) /* XMM/YMM state */
        {
            if (ecx & 0x10000000)
            cpu |= VCA_CPU_AVX;
            if (ecx & 0x00001000)
                cpu |= VCA_CPU_FMA3;
        }
    }

    if (max_basic_cap >= 7)
    {
        vca_cpu_cpuid(7, &eax, &ebx, &ecx, &edx);
        /* AVX2 requires OS support, but BMI1/2 don't. */
        if (ebx & 0x00000008)
            cpu |= VCA_CPU_BMI1;
        if (ebx & 0x00000100)
            cpu |= VCA_CPU_BMI2;

        if ((xcr0 & 0x6) == 0x6) /* XMM/YMM state */
        {
            if (ebx & 0x00000020)
                cpu |= VCA_CPU_AVX2;

            if ((xcr0 & 0xE0) == 0xE0) /* OPMASK/ZMM state */
            {
                if ((ebx & 0xD0030000) == 0xD0030000)
                {
                    cpu |= VCA_CPU_AVX512;
                }
            }
        }
    }

    vca_cpu_cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    max_extended_cap = eax;

    if (max_extended_cap >= 0x80000001)
    {
        vca_cpu_cpuid(0x80000001, &eax, &ebx, &ecx, &edx);

        if (ecx & 0x00000020)
            cpu |= VCA_CPU_LZCNT; /* Supported by Intel chips starting with Haswell */
        if (ecx & 0x00000040) /* SSE4a, AMD only */
        {
            int family = ((eax >> 8) & 0xf) + ((eax >> 20) & 0xff);
            cpu |= VCA_CPU_SSE2_IS_FAST;      /* Phenom and later CPUs have fast SSE units */
            if (family == 0x14)
            {
                cpu &= ~VCA_CPU_SSE2_IS_FAST; /* SSSE3 doesn't imply fast SSE anymore... */
                cpu |= VCA_CPU_SSE2_IS_SLOW;  /* Bobcat has 64-bit SIMD units */
                cpu |= VCA_CPU_SLOW_PALIGNR;  /* palignr is insanely slow on Bobcat */
            }
            if (family == 0x16)
            {
                cpu |= VCA_CPU_SLOW_PSHUFB;   /* Jaguar's pshufb isn't that slow, but it's slow enough
                                               * compared to alternate instruction sequences that this
                                               * is equal or faster on almost all such functions. */
            }
        }

        if (cpu & VCA_CPU_AVX)
        {
            if (ecx & 0x00000800) /* XOP */
                cpu |= VCA_CPU_XOP;
            if (ecx & 0x00010000) /* FMA4 */
                cpu |= VCA_CPU_FMA4;
        }

        if (!strcmp((char*)vendor, "AuthenticAMD"))
        {
            if (edx & 0x00400000)
                cpu |= VCA_CPU_MMX2;
            if ((cpu & VCA_CPU_SSE2) && !(cpu & VCA_CPU_SSE2_IS_FAST))
                cpu |= VCA_CPU_SSE2_IS_SLOW; /* AMD CPUs come in two types: terrible at SSE and great at it */
        }
    }

    if (!strcmp((char*)vendor, "GenuineIntel"))
    {
        vca_cpu_cpuid(1, &eax, &ebx, &ecx, &edx);
        int family = ((eax >> 8) & 0xf) + ((eax >> 20) & 0xff);
        int model  = ((eax >> 4) & 0xf) + ((eax >> 12) & 0xf0);
        if (family == 6)
        {
            /* Detect Atom CPU */
            if (model == 28)
            {
                cpu |= VCA_CPU_SLOW_ATOM;
                cpu |= VCA_CPU_SLOW_PSHUFB;
            }

            /* Conroe has a slow shuffle unit. Check the model number to make sure not
             * to include crippled low-end Penryns and Nehalems that don't have SSE4. */
            else if ((cpu & VCA_CPU_SSSE3) && !(cpu & VCA_CPU_SSE4) && model < 23)
                cpu |= VCA_CPU_SLOW_SHUFFLE;
        }
    }

    if ((!strcmp((char*)vendor, "GenuineIntel") || !strcmp((char*)vendor, "CyrixInstead")) && !(cpu & VCA_CPU_SSE42))
    {
        /* cacheline size is specified in 3 places, any of which may be missing */
        vca_cpu_cpuid(1, &eax, &ebx, &ecx, &edx);
        int cache = (ebx & 0xff00) >> 5; // cflush size
        if (!cache && max_extended_cap >= 0x80000006)
        {
            vca_cpu_cpuid(0x80000006, &eax, &ebx, &ecx, &edx);
            cache = ecx & 0xff; // cacheline size
        }
        if (!cache && max_basic_cap >= 2)
        {
            // Cache and TLB Information
            static const char cache32_ids[] = { '\x0a','\x0c','\x41','\x42','\x43','\x44','\x45','\x82','\x83','\x84','\x85','\0' };
            static const char cache64_ids[] = { '\x22','\x23','\x25','\x29','\x2c','\x46','\x47','\x49','\x60','\x66','\x67',
                                                '\x68','\x78','\x79','\x7a','\x7b','\x7c','\x7c','\x7f','\x86','\x87','\0' };
            uint32_t buf[4];
            int max, i = 0;
            do
            {
                vca_cpu_cpuid(2, buf + 0, buf + 1, buf + 2, buf + 3);
                max = buf[0] & 0xff;
                buf[0] &= ~0xff;
                for (int j = 0; j < 4; j++)
                {
                    if (!(buf[j] >> 31))
                        while (buf[j])
                        {
                            if (strchr(cache32_ids, buf[j] & 0xff))
                                cache = 32;
                            if (strchr(cache64_ids, buf[j] & 0xff))
                                cache = 64;
                            buf[j] >>= 8;
                        }
                }
            }
            while (++i < max);
        }

        if (cache == 32)
            cpu |= VCA_CPU_CACHELINE_32;
        else if (cache == 64)
            cpu |= VCA_CPU_CACHELINE_64;
        else
            vca_log(NULL, VCA_LOG_WARNING, "unable to determine cacheline size\n");
    }

#if BROKEN_STACK_ALIGNMENT
    cpu |= X265_CPU_STACK_MOD4;
#endif

    return cpu;
}

#else
uint32_t cpu_detect()
{
    return 0;
}
#endif // if VCA_ARCH_X86
}

