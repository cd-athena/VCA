/*****************************************************************************
 * Copyright (C) 2025 Werner Robitza
 *
 * Authors: Werner Robitza <werner.robitza@gmail.com>
 *
 * Based on x265 HEVC encoder DCT implementation:
 * - https://github.com/videolan/x265/blob/master/source/common/dct.cpp
 * - https://github.com/videolan/x265/blob/master/source/common/arm/dct-a.S
 * x265 is Copyright (C) 2013-2020 MulticoreWare, Inc (GPL v2+)
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

#include "dct-neon.h"

#if defined(VCA_ARCH_ARM) || defined(__aarch64__) || defined(_M_ARM64)

#include "neon-utils.h"
#include <analyzer/common/common.h>
#include <cstring>

namespace {

using namespace vca::neon;

// Transform coefficient matrices (same as in DCTTransformsNative.cpp)
alignas(16) const int16_t g_t8[8][8] = {
    {64, 64, 64, 64, 64, 64, 64, 64},
    {89, 75, 50, 18, -18, -50, -75, -89},
    {83, 36, -36, -83, -83, -36, 36, 83},
    {75, -18, -89, -50, 50, 89, 18, -75},
    {64, -64, -64, 64, 64, -64, -64, 64},
    {50, -89, 18, 75, -75, -18, 89, -50},
    {36, -83, 83, -36, -36, 83, -83, 36},
    {18, -50, 75, -89, 89, -75, 50, -18}
};

alignas(16) const int16_t g_t16[16][16] = {
    {64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64},
    {90, 87, 80, 70, 57, 43, 25, 9, -9, -25, -43, -57, -70, -80, -87, -90},
    {89, 75, 50, 18, -18, -50, -75, -89, -89, -75, -50, -18, 18, 50, 75, 89},
    {87, 57, 9, -43, -80, -90, -70, -25, 25, 70, 90, 80, 43, -9, -57, -87},
    {83, 36, -36, -83, -83, -36, 36, 83, 83, 36, -36, -83, -83, -36, 36, 83},
    {80, 9, -70, -87, -25, 57, 90, 43, -43, -90, -57, 25, 87, 70, -9, -80},
    {75, -18, -89, -50, 50, 89, 18, -75, -75, 18, 89, 50, -50, -89, -18, 75},
    {70, -43, -87, 9, 90, 25, -80, -57, 57, 80, -25, -90, -9, 87, 43, -70},
    {64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64},
    {57, -80, -25, 90, -9, -87, 43, 70, -70, -43, 87, 9, -90, 25, 80, -57},
    {50, -89, 18, 75, -75, -18, 89, -50, -50, 89, -18, -75, 75, 18, -89, 50},
    {43, -90, 57, 25, -87, 70, 9, -80, 80, -9, -70, 87, -25, -57, 90, -43},
    {36, -83, 83, -36, -36, 83, -83, 36, 36, -83, 83, -36, -36, 83, -83, 36},
    {25, -70, 90, -80, 43, 9, -57, 87, -87, 57, -9, -43, 80, -90, 70, -25},
    {18, -50, 75, -89, 89, -75, 50, -18, -18, 50, -75, 89, -89, 75, -50, 18},
    {9, -25, 43, -57, 70, -80, 87, -90, 90, -87, 80, -70, 57, -43, 25, -9}
};

alignas(16) const int16_t g_t32[32][32] = {
    {64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
     64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64},
    {90, 90, 88, 85, 82, 78, 73, 67, 61, 54, 46, 38, 31, 22, 13, 4,
     -4, -13, -22, -31, -38, -46, -54, -61, -67, -73, -78, -82, -85, -88, -90, -90},
    {90, 87, 80, 70, 57, 43, 25, 9, -9, -25, -43, -57, -70, -80, -87, -90,
     -90, -87, -80, -70, -57, -43, -25, -9, 9, 25, 43, 57, 70, 80, 87, 90},
    {90, 82, 67, 46, 22, -4, -31, -54, -73, -85, -90, -88, -78, -61, -38, -13,
     13, 38, 61, 78, 88, 90, 85, 73, 54, 31, 4, -22, -46, -67, -82, -90},
    {89, 75, 50, 18, -18, -50, -75, -89, -89, -75, -50, -18, 18, 50, 75, 89,
     89, 75, 50, 18, -18, -50, -75, -89, -89, -75, -50, -18, 18, 50, 75, 89},
    {88, 67, 31, -13, -54, -82, -90, -78, -46, -4, 38, 73, 90, 85, 61, 22,
     -22, -61, -85, -90, -73, -38, 4, 46, 78, 90, 82, 54, 13, -31, -67, -88},
    {87, 57, 9, -43, -80, -90, -70, -25, 25, 70, 90, 80, 43, -9, -57, -87,
     -87, -57, -9, 43, 80, 90, 70, 25, -25, -70, -90, -80, -43, 9, 57, 87},
    {85, 46, -13, -67, -90, -73, -22, 38, 82, 88, 54, -4, -61, -90, -78, -31,
     31, 78, 90, 61, 4, -54, -88, -82, -38, 22, 73, 90, 67, 13, -46, -85},
    {83, 36, -36, -83, -83, -36, 36, 83, 83, 36, -36, -83, -83, -36, 36, 83,
     83, 36, -36, -83, -83, -36, 36, 83, 83, 36, -36, -83, -83, -36, 36, 83},
    {82, 22, -54, -90, -61, 13, 78, 85, 31, -46, -90, -67, 4, 73, 88, 38,
     -38, -88, -73, -4, 67, 90, 46, -31, -85, -78, -13, 61, 90, 54, -22, -82},
    {80, 9, -70, -87, -25, 57, 90, 43, -43, -90, -57, 25, 87, 70, -9, -80,
     -80, -9, 70, 87, 25, -57, -90, -43, 43, 90, 57, -25, -87, -70, 9, 80},
    {78, -4, -82, -73, 13, 85, 67, -22, -88, -61, 31, 90, 54, -38, -90, -46,
     46, 90, 38, -54, -90, -31, 61, 88, 22, -67, -85, -13, 73, 82, 4, -78},
    {75, -18, -89, -50, 50, 89, 18, -75, -75, 18, 89, 50, -50, -89, -18, 75,
     75, -18, -89, -50, 50, 89, 18, -75, -75, 18, 89, 50, -50, -89, -18, 75},
    {73, -31, -90, -22, 78, 67, -38, -90, -13, 82, 61, -46, -88, -4, 85, 54,
     -54, -85, 4, 88, 46, -61, -82, 13, 90, 38, -67, -78, 22, 90, 31, -73},
    {70, -43, -87, 9, 90, 25, -80, -57, 57, 80, -25, -90, -9, 87, 43, -70,
     -70, 43, 87, -9, -90, -25, 80, 57, -57, -80, 25, 90, 9, -87, -43, 70},
    {67, -54, -78, 38, 85, -22, -90, 4, 90, 13, -88, -31, 82, 46, -73, -61,
     61, 73, -46, -82, 31, 88, -13, -90, -4, 90, 22, -85, -38, 78, 54, -67},
    {64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64,
     64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64},
    {61, -73, -46, 82, 31, -88, -13, 90, -4, -90, 22, 85, -38, -78, 54, 67,
     -67, -54, 78, 38, -85, -22, 90, 4, -90, 13, 88, -31, -82, 46, 73, -61},
    {57, -80, -25, 90, -9, -87, 43, 70, -70, -43, 87, 9, -90, 25, 80, -57,
     -57, 80, 25, -90, 9, 87, -43, -70, 70, 43, -87, -9, 90, -25, -80, 57},
    {54, -85, -4, 88, -46, -61, 82, 13, -90, 38, 67, -78, -22, 90, -31, -73,
     73, 31, -90, 22, 78, -67, -38, 90, -13, -82, 61, 46, -88, 4, 85, -54},
    {50, -89, 18, 75, -75, -18, 89, -50, -50, 89, -18, -75, 75, 18, -89, 50,
     50, -89, 18, 75, -75, -18, 89, -50, -50, 89, -18, -75, 75, 18, -89, 50},
    {46, -90, 38, 54, -90, 31, 61, -88, 22, 67, -85, 13, 73, -82, 4, 78,
     -78, -4, 82, -73, -13, 85, -67, -22, 88, -61, -31, 90, -54, -38, 90, -46},
    {43, -90, 57, 25, -87, 70, 9, -80, 80, -9, -70, 87, -25, -57, 90, -43,
     -43, 90, -57, -25, 87, -70, -9, 80, -80, 9, 70, -87, 25, 57, -90, 43},
    {38, -88, 73, -4, -67, 90, -46, -31, 85, -78, 13, 61, -90, 54, 22, -82,
     82, -22, -54, 90, -61, -13, 78, -85, 31, 46, -90, 67, 4, -73, 88, -38},
    {36, -83, 83, -36, -36, 83, -83, 36, 36, -83, 83, -36, -36, 83, -83, 36,
     36, -83, 83, -36, -36, 83, -83, 36, 36, -83, 83, -36, -36, 83, -83, 36},
    {31, -78, 90, -61, 4, 54, -88, 82, -38, -22, 73, -90, 67, -13, -46, 85,
     -85, 46, 13, -67, 90, -73, 22, 38, -82, 88, -54, -4, 61, -90, 78, -31},
    {25, -70, 90, -80, 43, 9, -57, 87, -87, 57, -9, -43, 80, -90, 70, -25,
     -25, 70, -90, 80, -43, -9, 57, -87, 87, -57, 9, 43, -80, 90, -70, 25},
    {22, -61, 85, -90, 73, -38, -4, 46, -78, 90, -82, 54, -13, -31, 67, -88,
     88, -67, 31, 13, -54, 82, -90, 78, -46, 4, 38, -73, 90, -85, 61, -22},
    {18, -50, 75, -89, 89, -75, 50, -18, -18, 50, -75, 89, -89, 75, -50, 18,
     18, -50, 75, -89, 89, -75, 50, -18, -18, 50, -75, 89, -89, 75, -50, 18},
    {13, -38, 61, -78, 88, -90, 85, -73, 54, -31, 4, 22, -46, 67, -82, 90,
     -90, 82, -67, 46, -22, -4, 31, -54, 73, -85, 90, -88, 78, -61, 38, -13},
    {9, -25, 43, -57, 70, -80, 87, -90, 90, -87, 80, -70, 57, -43, 25, -9,
     -9, 25, -43, 57, -70, 80, -87, 90, -90, 87, -80, 70, -57, 43, -25, 9},
    {4, -13, 22, -31, 38, -46, 54, -61, 67, -73, 78, -82, 85, -88, 90, -90,
     90, -90, 88, -85, 82, -78, 73, -67, 61, -54, 46, -38, 31, -22, 13, -4}
};

// Even coefficients for 8x8 transform (used for EE calculations)
alignas(16) const int32_t t8_even[4][4] = {
    {64, 64, 64, 64},
    {83, 36, 83, 36},
    {64, -64, 64, -64},
    {36, -83, 36, -83}
};

//------------------------------------------------------------------------------
// DCT8 Implementation
//------------------------------------------------------------------------------

template<int shift>
static inline void partialButterfly8_neon(const int16_t *src, int16_t *dst)
{
    const int line = 8;

    int16x4_t O[line];
    int32x4_t EE[line / 2];
    int32x4_t EO[line / 2];

    // Calculate O, EE, EO for all rows
    for (int i = 0; i < line; i += 2)
    {
        int16x4_t s0_lo = vld1_s16(src + i * line);
        int16x4_t s0_hi = vrev64_s16(vld1_s16(src + i * line + 4));

        int16x4_t s1_lo = vld1_s16(src + (i + 1) * line);
        int16x4_t s1_hi = vrev64_s16(vld1_s16(src + (i + 1) * line + 4));

        int32x4_t E0 = vaddl_s16(s0_lo, s0_hi);
        int32x4_t E1 = vaddl_s16(s1_lo, s1_hi);

        O[i + 0] = vsub_s16(s0_lo, s0_hi);
        O[i + 1] = vsub_s16(s1_lo, s1_hi);

        int32x4_t t0 = vreinterpretq_s32_s64(
            vzip1q_s64(vreinterpretq_s64_s32(E0), vreinterpretq_s64_s32(E1)));
        int32x4_t t1 = vrev64q_s32(vreinterpretq_s32_s64(
            vzip2q_s64(vreinterpretq_s64_s32(E0), vreinterpretq_s64_s32(E1))));

        EE[i / 2] = vaddq_s32(t0, t1);
        EO[i / 2] = vsubq_s32(t0, t1);
    }

    int16_t *d = dst;

    int32x4_t c0 = vld1q_s32(t8_even[0]);
    int32x4_t c2 = vld1q_s32(t8_even[1]);
    int32x4_t c4 = vld1q_s32(t8_even[2]);
    int32x4_t c6 = vld1q_s32(t8_even[3]);
    int16x4_t c1 = vld1_s16(g_t8[1]);
    int16x4_t c3 = vld1_s16(g_t8[3]);
    int16x4_t c5 = vld1_s16(g_t8[5]);
    int16x4_t c7 = vld1_s16(g_t8[7]);

    for (int j = 0; j < line; j += 4)
    {
        // Odd coefficients (1, 3, 5, 7)
        int32x4_t t01 = vpaddq_s32(vmull_s16(c1, O[j + 0]),
                                   vmull_s16(c1, O[j + 1]));
        int32x4_t t23 = vpaddq_s32(vmull_s16(c1, O[j + 2]),
                                   vmull_s16(c1, O[j + 3]));
        int16x4_t res1 = vrshrn_n_s32(vpaddq_s32(t01, t23), shift);
        vst1_s16(d + 1 * line, res1);

        t01 = vpaddq_s32(vmull_s16(c3, O[j + 0]), vmull_s16(c3, O[j + 1]));
        t23 = vpaddq_s32(vmull_s16(c3, O[j + 2]), vmull_s16(c3, O[j + 3]));
        int16x4_t res3 = vrshrn_n_s32(vpaddq_s32(t01, t23), shift);
        vst1_s16(d + 3 * line, res3);

        t01 = vpaddq_s32(vmull_s16(c5, O[j + 0]), vmull_s16(c5, O[j + 1]));
        t23 = vpaddq_s32(vmull_s16(c5, O[j + 2]), vmull_s16(c5, O[j + 3]));
        int16x4_t res5 = vrshrn_n_s32(vpaddq_s32(t01, t23), shift);
        vst1_s16(d + 5 * line, res5);

        t01 = vpaddq_s32(vmull_s16(c7, O[j + 0]), vmull_s16(c7, O[j + 1]));
        t23 = vpaddq_s32(vmull_s16(c7, O[j + 2]), vmull_s16(c7, O[j + 3]));
        int16x4_t res7 = vrshrn_n_s32(vpaddq_s32(t01, t23), shift);
        vst1_s16(d + 7 * line, res7);

        // Even coefficients (0, 2, 4, 6)
        int32x4_t t0 = vpaddq_s32(EE[j / 2 + 0], EE[j / 2 + 1]);
        int32x4_t t1 = vmulq_s32(c0, t0);
        int16x4_t res0 = vrshrn_n_s32(t1, shift);
        vst1_s16(d + 0 * line, res0);

        int32x4_t t2 = vmulq_s32(c2, EO[j / 2 + 0]);
        int32x4_t t3 = vmulq_s32(c2, EO[j / 2 + 1]);
        int16x4_t res2 = vrshrn_n_s32(vpaddq_s32(t2, t3), shift);
        vst1_s16(d + 2 * line, res2);

        int32x4_t t4 = vmulq_s32(c4, EE[j / 2 + 0]);
        int32x4_t t5 = vmulq_s32(c4, EE[j / 2 + 1]);
        int16x4_t res4 = vrshrn_n_s32(vpaddq_s32(t4, t5), shift);
        vst1_s16(d + 4 * line, res4);

        int32x4_t t6 = vmulq_s32(c6, EO[j / 2 + 0]);
        int32x4_t t7 = vmulq_s32(c6, EO[j / 2 + 1]);
        int16x4_t res6 = vrshrn_n_s32(vpaddq_s32(t6, t7), shift);
        vst1_s16(d + 6 * line, res6);

        d += 4;
    }
}

template<int bitDepth>
static void dct8_neon(const int16_t *src, int16_t *dst, intptr_t srcStride)
{
    const int shift_1st = 2 + bitDepth - 8;
    const int shift_2nd = 9;

    ALIGN_VAR_32(int16_t, coef[8 * 8]);
    ALIGN_VAR_32(int16_t, block[8 * 8]);

    // Copy input with stride to contiguous block
    for (int i = 0; i < 8; i++)
    {
        std::memcpy(&block[i * 8], &src[i * srcStride], 8 * sizeof(int16_t));
    }

    // Two-pass transform
    if (shift_1st == 2) {
        partialButterfly8_neon<2>(block, coef);
    } else if (shift_1st == 4) {
        partialButterfly8_neon<4>(block, coef);
    } else {
        partialButterfly8_neon<6>(block, coef);
    }
    partialButterfly8_neon<9>(coef, dst);
}

//------------------------------------------------------------------------------
// DCT16 Implementation
//------------------------------------------------------------------------------

template<int shift>
static inline void partialButterfly16_neon(const int16_t *src, int16_t *dst)
{
    const int line = 16;

    int16x8_t O[line];
    int32x4_t EO[line];
    int32x4_t EEE[line / 2];
    int32x4_t EEO[line / 2];

    for (int i = 0; i < line; i += 2)
    {
        int16x8_t s0_lo = vld1q_s16(src + i * line);
        int16x8_t s0_hi = rev16(vld1q_s16(src + i * line + 8));

        int16x8_t s1_lo = vld1q_s16(src + (i + 1) * line);
        int16x8_t s1_hi = rev16(vld1q_s16(src + (i + 1) * line + 8));

        int32x4_t E0[2];
        E0[0] = vaddl_s16(vget_low_s16(s0_lo), vget_low_s16(s0_hi));
        E0[1] = vaddl_s16(vget_high_s16(s0_lo), vget_high_s16(s0_hi));

        int32x4_t E1[2];
        E1[0] = vaddl_s16(vget_low_s16(s1_lo), vget_low_s16(s1_hi));
        E1[1] = vaddl_s16(vget_high_s16(s1_lo), vget_high_s16(s1_hi));

        O[i + 0] = vsubq_s16(s0_lo, s0_hi);
        O[i + 1] = vsubq_s16(s1_lo, s1_hi);

        int32x4_t EE0 = vaddq_s32(E0[0], rev32(E0[1]));
        int32x4_t EE1 = vaddq_s32(E1[0], rev32(E1[1]));
        EO[i + 0] = vsubq_s32(E0[0], rev32(E0[1]));
        EO[i + 1] = vsubq_s32(E1[0], rev32(E1[1]));

        int32x4_t t0 = vreinterpretq_s32_s64(
            vzip1q_s64(vreinterpretq_s64_s32(EE0), vreinterpretq_s64_s32(EE1)));
        int32x4_t t1 = vrev64q_s32(vreinterpretq_s32_s64(vzip2q_s64(
            vreinterpretq_s64_s32(EE0), vreinterpretq_s64_s32(EE1))));

        EEE[i / 2] = vaddq_s32(t0, t1);
        EEO[i / 2] = vsubq_s32(t0, t1);
    }

    for (int i = 0; i < line; i += 4)
    {
        // Odd coefficients
        for (int k = 1; k < 16; k += 2)
        {
            int16x8_t c0_c4 = vld1q_s16(&g_t16[k][0]);

            int32x4_t t0 = vmull_s16(vget_low_s16(c0_c4), vget_low_s16(O[i + 0]));
            int32x4_t t1 = vmull_s16(vget_low_s16(c0_c4), vget_low_s16(O[i + 1]));
            int32x4_t t2 = vmull_s16(vget_low_s16(c0_c4), vget_low_s16(O[i + 2]));
            int32x4_t t3 = vmull_s16(vget_low_s16(c0_c4), vget_low_s16(O[i + 3]));
            t0 = vmlal_s16(t0, vget_high_s16(c0_c4), vget_high_s16(O[i + 0]));
            t1 = vmlal_s16(t1, vget_high_s16(c0_c4), vget_high_s16(O[i + 1]));
            t2 = vmlal_s16(t2, vget_high_s16(c0_c4), vget_high_s16(O[i + 2]));
            t3 = vmlal_s16(t3, vget_high_s16(c0_c4), vget_high_s16(O[i + 3]));

            int32x4_t t = vpaddq_s32(vpaddq_s32(t0, t1), vpaddq_s32(t2, t3));
            int16x4_t res = vrshrn_n_s32(t, shift);
            vst1_s16(dst + k * line, res);
        }

        // EO coefficients
        for (int k = 2; k < 16; k += 4)
        {
            int32x4_t c0 = vmovl_s16(vld1_s16(&g_t16[k][0]));
            int32x4_t t0 = vmulq_s32(c0, EO[i + 0]);
            int32x4_t t1 = vmulq_s32(c0, EO[i + 1]);
            int32x4_t t2 = vmulq_s32(c0, EO[i + 2]);
            int32x4_t t3 = vmulq_s32(c0, EO[i + 3]);
            int32x4_t t = vpaddq_s32(vpaddq_s32(t0, t1), vpaddq_s32(t2, t3));

            int16x4_t res = vrshrn_n_s32(t, shift);
            vst1_s16(dst + k * line, res);
        }

        // EEE and EEO coefficients
        int32x4_t c0 = vld1q_s32(t8_even[0]);
        int32x4_t c4 = vld1q_s32(t8_even[1]);
        int32x4_t c8 = vld1q_s32(t8_even[2]);
        int32x4_t c12 = vld1q_s32(t8_even[3]);

        int32x4_t t0 = vpaddq_s32(EEE[i / 2 + 0], EEE[i / 2 + 1]);
        int32x4_t t1 = vmulq_s32(c0, t0);
        int16x4_t res0 = vrshrn_n_s32(t1, shift);
        vst1_s16(dst + 0 * line, res0);

        int32x4_t t2 = vmulq_s32(c4, EEO[i / 2 + 0]);
        int32x4_t t3 = vmulq_s32(c4, EEO[i / 2 + 1]);
        int16x4_t res4 = vrshrn_n_s32(vpaddq_s32(t2, t3), shift);
        vst1_s16(dst + 4 * line, res4);

        int32x4_t t4 = vmulq_s32(c8, EEE[i / 2 + 0]);
        int32x4_t t5 = vmulq_s32(c8, EEE[i / 2 + 1]);
        int16x4_t res8 = vrshrn_n_s32(vpaddq_s32(t4, t5), shift);
        vst1_s16(dst + 8 * line, res8);

        int32x4_t t6 = vmulq_s32(c12, EEO[i / 2 + 0]);
        int32x4_t t7 = vmulq_s32(c12, EEO[i / 2 + 1]);
        int16x4_t res12 = vrshrn_n_s32(vpaddq_s32(t6, t7), shift);
        vst1_s16(dst + 12 * line, res12);

        dst += 4;
    }
}

template<int bitDepth>
static void dct16_neon(const int16_t *src, int16_t *dst, intptr_t srcStride)
{
    const int shift_1st = 3 + bitDepth - 8;
    const int shift_2nd = 10;

    ALIGN_VAR_32(int16_t, coef[16 * 16]);
    ALIGN_VAR_32(int16_t, block[16 * 16]);

    // Copy input with stride to contiguous block
    for (int i = 0; i < 16; i++)
    {
        std::memcpy(&block[i * 16], &src[i * srcStride], 16 * sizeof(int16_t));
    }

    // Two-pass transform
    if (shift_1st == 3) {
        partialButterfly16_neon<3>(block, coef);
    } else if (shift_1st == 5) {
        partialButterfly16_neon<5>(block, coef);
    } else {
        partialButterfly16_neon<7>(block, coef);
    }
    partialButterfly16_neon<10>(coef, dst);
}

//------------------------------------------------------------------------------
// DCT32 Implementation
//------------------------------------------------------------------------------

template<int shift>
static inline void partialButterfly32_neon(const int16_t *src, int16_t *dst)
{
    const int line = 32;

    int16x8_t O[line][2];
    int32x4_t EO[line][2];
    int32x4_t EEO[line];
    int32x4_t EEEE[line / 2];
    int32x4_t EEEO[line / 2];

    for (int i = 0; i < line; i += 2)
    {
        int16x8_t in_lo0 = vld1q_s16(src + i * line);
        int16x8_t in_lo1 = vld1q_s16(src + i * line + 8);
        int16x8_t in_hi0 = rev16(vld1q_s16(src + i * line + 24));
        int16x8_t in_hi1 = rev16(vld1q_s16(src + i * line + 16));

        int16x8_t in_lo0_n = vld1q_s16(src + (i + 1) * line);
        int16x8_t in_lo1_n = vld1q_s16(src + (i + 1) * line + 8);
        int16x8_t in_hi0_n = rev16(vld1q_s16(src + (i + 1) * line + 24));
        int16x8_t in_hi1_n = rev16(vld1q_s16(src + (i + 1) * line + 16));

        O[i][0] = vsubq_s16(in_lo0, in_hi0);
        O[i][1] = vsubq_s16(in_lo1, in_hi1);
        O[i + 1][0] = vsubq_s16(in_lo0_n, in_hi0_n);
        O[i + 1][1] = vsubq_s16(in_lo1_n, in_hi1_n);

        int32x4_t E0[4], E1[4];
        E0[0] = vaddl_s16(vget_low_s16(in_lo0), vget_low_s16(in_hi0));
        E0[1] = vaddl_s16(vget_high_s16(in_lo0), vget_high_s16(in_hi0));
        E0[2] = vaddl_s16(vget_low_s16(in_lo1), vget_low_s16(in_hi1));
        E0[3] = vaddl_s16(vget_high_s16(in_lo1), vget_high_s16(in_hi1));

        E1[0] = vaddl_s16(vget_low_s16(in_lo0_n), vget_low_s16(in_hi0_n));
        E1[1] = vaddl_s16(vget_high_s16(in_lo0_n), vget_high_s16(in_hi0_n));
        E1[2] = vaddl_s16(vget_low_s16(in_lo1_n), vget_low_s16(in_hi1_n));
        E1[3] = vaddl_s16(vget_high_s16(in_lo1_n), vget_high_s16(in_hi1_n));

        int32x4_t EE0[2], EE1[2];
        EE0[0] = vaddq_s32(E0[0], rev32(E0[3]));
        EE0[1] = vaddq_s32(E0[1], rev32(E0[2]));
        EE1[0] = vaddq_s32(E1[0], rev32(E1[3]));
        EE1[1] = vaddq_s32(E1[1], rev32(E1[2]));

        EO[i][0] = vsubq_s32(E0[0], rev32(E0[3]));
        EO[i][1] = vsubq_s32(E0[1], rev32(E0[2]));
        EO[i + 1][0] = vsubq_s32(E1[0], rev32(E1[3]));
        EO[i + 1][1] = vsubq_s32(E1[1], rev32(E1[2]));

        int32x4_t EEE0 = vaddq_s32(EE0[0], rev32(EE0[1]));
        int32x4_t EEE1 = vaddq_s32(EE1[0], rev32(EE1[1]));
        EEO[i] = vsubq_s32(EE0[0], rev32(EE0[1]));
        EEO[i + 1] = vsubq_s32(EE1[0], rev32(EE1[1]));

        int32x4_t t0 = vreinterpretq_s32_s64(
            vzip1q_s64(vreinterpretq_s64_s32(EEE0), vreinterpretq_s64_s32(EEE1)));
        int32x4_t t1 = vrev64q_s32(vreinterpretq_s32_s64(vzip2q_s64(
            vreinterpretq_s64_s32(EEE0), vreinterpretq_s64_s32(EEE1))));

        EEEE[i / 2] = vaddq_s32(t0, t1);
        EEEO[i / 2] = vsubq_s32(t0, t1);
    }

    // Process O coefficients (odd rows: 1, 3, 5, ..., 31)
    for (int k = 1; k < 32; k += 2)
    {
        int16_t *d = dst + k * line;

        int16x8_t c0 = vld1q_s16(&g_t32[k][0]);
        int16x8_t c1 = vld1q_s16(&g_t32[k][8]);

        for (int i = 0; i < line; i += 4)
        {
            int32x4_t t[4];
            for (int j = 0; j < 4; ++j)
            {
                t[j] = vmull_s16(vget_low_s16(c0), vget_low_s16(O[i + j][0]));
                t[j] = vmlal_s16(t[j], vget_high_s16(c0), vget_high_s16(O[i + j][0]));
                t[j] = vmlal_s16(t[j], vget_low_s16(c1), vget_low_s16(O[i + j][1]));
                t[j] = vmlal_s16(t[j], vget_high_s16(c1), vget_high_s16(O[i + j][1]));
            }

            int32x4_t t0123 = vpaddq_s32(vpaddq_s32(t[0], t[1]),
                                         vpaddq_s32(t[2], t[3]));
            int16x4_t res = vrshrn_n_s32(t0123, shift);
            vst1_s16(d, res);
            d += 4;
        }
    }

    // Process EO coefficients (rows: 2, 6, 10, ..., 30)
    for (int k = 2; k < 32; k += 4)
    {
        int16_t *d = dst + k * line;

        int32x4_t c0 = vmovl_s16(vld1_s16(&g_t32[k][0]));
        int32x4_t c1 = vmovl_s16(vld1_s16(&g_t32[k][4]));

        for (int i = 0; i < line; i += 4)
        {
            int32x4_t t[4];
            for (int j = 0; j < 4; ++j)
            {
                t[j] = vmulq_s32(c0, EO[i + j][0]);
                t[j] = vmlaq_s32(t[j], c1, EO[i + j][1]);
            }

            int32x4_t t0123 = vpaddq_s32(vpaddq_s32(t[0], t[1]),
                                         vpaddq_s32(t[2], t[3]));
            int16x4_t res = vrshrn_n_s32(t0123, shift);
            vst1_s16(d, res);
            d += 4;
        }
    }

    // Process EEO coefficients (rows: 4, 12, 20, 28)
    for (int k = 4; k < 32; k += 8)
    {
        int16_t *d = dst + k * line;

        int32x4_t c = vmovl_s16(vld1_s16(&g_t32[k][0]));

        for (int i = 0; i < line; i += 4)
        {
            int32x4_t t0 = vmulq_s32(c, EEO[i + 0]);
            int32x4_t t1 = vmulq_s32(c, EEO[i + 1]);
            int32x4_t t2 = vmulq_s32(c, EEO[i + 2]);
            int32x4_t t3 = vmulq_s32(c, EEO[i + 3]);

            int32x4_t t = vpaddq_s32(vpaddq_s32(t0, t1), vpaddq_s32(t2, t3));
            int16x4_t res = vrshrn_n_s32(t, shift);
            vst1_s16(d, res);
            d += 4;
        }
    }

    // Process EEEE and EEEO coefficients (rows: 0, 8, 16, 24)
    int32x4_t c0 = vld1q_s32(t8_even[0]);
    int32x4_t c8 = vld1q_s32(t8_even[1]);
    int32x4_t c16 = vld1q_s32(t8_even[2]);
    int32x4_t c24 = vld1q_s32(t8_even[3]);

    for (int i = 0; i < line; i += 4)
    {
        int32x4_t t0 = vpaddq_s32(EEEE[i / 2 + 0], EEEE[i / 2 + 1]);
        int32x4_t t1 = vmulq_s32(c0, t0);
        int16x4_t res0 = vrshrn_n_s32(t1, shift);
        vst1_s16(dst + 0 * line, res0);

        int32x4_t t2 = vmulq_s32(c8, EEEO[i / 2 + 0]);
        int32x4_t t3 = vmulq_s32(c8, EEEO[i / 2 + 1]);
        int16x4_t res8 = vrshrn_n_s32(vpaddq_s32(t2, t3), shift);
        vst1_s16(dst + 8 * line, res8);

        int32x4_t t4 = vmulq_s32(c16, EEEE[i / 2 + 0]);
        int32x4_t t5 = vmulq_s32(c16, EEEE[i / 2 + 1]);
        int16x4_t res16 = vrshrn_n_s32(vpaddq_s32(t4, t5), shift);
        vst1_s16(dst + 16 * line, res16);

        int32x4_t t6 = vmulq_s32(c24, EEEO[i / 2 + 0]);
        int32x4_t t7 = vmulq_s32(c24, EEEO[i / 2 + 1]);
        int16x4_t res24 = vrshrn_n_s32(vpaddq_s32(t6, t7), shift);
        vst1_s16(dst + 24 * line, res24);

        dst += 4;
    }
}

template<int bitDepth>
static void dct32_neon(const int16_t *src, int16_t *dst, intptr_t srcStride)
{
    const int shift_1st = 4 + bitDepth - 8;
    const int shift_2nd = 11;

    ALIGN_VAR_32(int16_t, coef[32 * 32]);
    ALIGN_VAR_32(int16_t, block[32 * 32]);

    // Copy input with stride to contiguous block
    for (int i = 0; i < 32; i++)
    {
        std::memcpy(&block[i * 32], &src[i * srcStride], 32 * sizeof(int16_t));
    }

    // Two-pass transform
    if (shift_1st == 4) {
        partialButterfly32_neon<4>(block, coef);
    } else if (shift_1st == 6) {
        partialButterfly32_neon<6>(block, coef);
    } else {
        partialButterfly32_neon<8>(block, coef);
    }
    partialButterfly32_neon<11>(coef, dst);
}

} // anonymous namespace

//------------------------------------------------------------------------------
// Public API
//------------------------------------------------------------------------------

extern "C" {

void vca_dct8_8bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride)
{
    dct8_neon<8>(src, dst, srcStride);
}

void vca_dct8_10bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride)
{
    dct8_neon<10>(src, dst, srcStride);
}

void vca_dct8_12bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride)
{
    dct8_neon<12>(src, dst, srcStride);
}

void vca_dct16_8bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride)
{
    dct16_neon<8>(src, dst, srcStride);
}

void vca_dct16_10bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride)
{
    dct16_neon<10>(src, dst, srcStride);
}

void vca_dct16_12bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride)
{
    dct16_neon<12>(src, dst, srcStride);
}

void vca_dct32_8bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride)
{
    dct32_neon<8>(src, dst, srcStride);
}

void vca_dct32_10bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride)
{
    dct32_neon<10>(src, dst, srcStride);
}

void vca_dct32_12bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride)
{
    dct32_neon<12>(src, dst, srcStride);
}

} // extern "C"

#endif // VCA_ARCH_ARM
