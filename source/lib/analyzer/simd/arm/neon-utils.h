/*****************************************************************************
 * Copyright (C) 2025 Werner Robitza
 * Copyright (C) 2026 Christian Doppler Laboratory ATHENA
 *
 * Authors: Werner Robitza <werner.robitza@gmail.com>
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

#if defined(VCA_ARCH_ARM) || defined(__aarch64__) || defined(_M_ARM64)

#include <arm_neon.h>

namespace vca {
namespace neon {

// Lookup tables for reversing 16-bit and 32-bit elements within 128-bit vectors
alignas(16) const uint8_t rev16_tbl[16] = {
    14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1
};

alignas(16) const uint8_t rev32_tbl[16] = {
    12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3
};

// Reverse 8 int16 elements in a 128-bit vector (for 16x16 and 32x32 DCT)
static inline int16x8_t rev16(const int16x8_t a)
{
    const uint8x16_t tbl = vld1q_u8(rev16_tbl);
    const int8x16_t a_s8 = vreinterpretq_s8_s16(a);
    return vreinterpretq_s16_s8(vqtbx1q_s8(a_s8, a_s8, tbl));
}

// Reverse 4 int32 elements in a 128-bit vector
static inline int32x4_t rev32(const int32x4_t a)
{
    const uint8x16_t tbl = vld1q_u8(rev32_tbl);
    const int8x16_t a_s8 = vreinterpretq_s8_s32(a);
    return vreinterpretq_s32_s8(vqtbx1q_s8(a_s8, a_s8, tbl));
}

// Transpose 4x4 matrix of int16 elements
static inline void transpose_4x4_s16(int16x4_t &s0, int16x4_t &s1,
                                      int16x4_t &s2, int16x4_t &s3)
{
    int16x8_t s0q = vcombine_s16(s0, vdup_n_s16(0));
    int16x8_t s1q = vcombine_s16(s1, vdup_n_s16(0));
    int16x8_t s2q = vcombine_s16(s2, vdup_n_s16(0));
    int16x8_t s3q = vcombine_s16(s3, vdup_n_s16(0));

    int16x8_t s02 = vzip1q_s16(s0q, s2q);
    int16x8_t s13 = vzip1q_s16(s1q, s3q);

    int16x8x2_t s0123 = vzipq_s16(s02, s13);

    s0 = vget_low_s16(s0123.val[0]);
    s1 = vget_high_s16(s0123.val[0]);
    s2 = vget_low_s16(s0123.val[1]);
    s3 = vget_high_s16(s0123.val[1]);
}

// Transpose 4x8 matrix (4 rows of 8 elements, but only lower 4 elements used)
// Result is 8 rows of 4 elements, stored in 4 int16x8_t vectors
static inline void transpose_4x8_s16(int16x4_t s0, int16x4_t s1, int16x4_t s2, int16x4_t s3,
                                     int16x4_t s4, int16x4_t s5, int16x4_t s6, int16x4_t s7,
                                     int16x8_t &d0, int16x8_t &d1, int16x8_t &d2, int16x8_t &d3)
{
    int16x8_t s0q = vcombine_s16(s0, vdup_n_s16(0));
    int16x8_t s1q = vcombine_s16(s1, vdup_n_s16(0));
    int16x8_t s2q = vcombine_s16(s2, vdup_n_s16(0));
    int16x8_t s3q = vcombine_s16(s3, vdup_n_s16(0));
    int16x8_t s4q = vcombine_s16(s4, vdup_n_s16(0));
    int16x8_t s5q = vcombine_s16(s5, vdup_n_s16(0));
    int16x8_t s6q = vcombine_s16(s6, vdup_n_s16(0));
    int16x8_t s7q = vcombine_s16(s7, vdup_n_s16(0));

    int16x8_t s04 = vzip1q_s16(s0q, s4q);
    int16x8_t s15 = vzip1q_s16(s1q, s5q);
    int16x8_t s26 = vzip1q_s16(s2q, s6q);
    int16x8_t s37 = vzip1q_s16(s3q, s7q);

    int16x8x2_t s0246 = vzipq_s16(s04, s26);
    int16x8x2_t s1357 = vzipq_s16(s15, s37);

    d0 = vzip1q_s16(s0246.val[0], s1357.val[0]);
    d1 = vzip2q_s16(s0246.val[0], s1357.val[0]);
    d2 = vzip1q_s16(s0246.val[1], s1357.val[1]);
    d3 = vzip2q_s16(s0246.val[1], s1357.val[1]);
}

// Transpose 8x8 matrix of int16 elements
static inline void transpose_8x8_s16(int16x8_t &r0, int16x8_t &r1, int16x8_t &r2, int16x8_t &r3,
                                     int16x8_t &r4, int16x8_t &r5, int16x8_t &r6, int16x8_t &r7)
{
    // Transpose pairs of rows
    int16x8x2_t t01 = vtrnq_s16(r0, r1);
    int16x8x2_t t23 = vtrnq_s16(r2, r3);
    int16x8x2_t t45 = vtrnq_s16(r4, r5);
    int16x8x2_t t67 = vtrnq_s16(r6, r7);

    // Reinterpret as 32-bit and transpose again
    int32x4x2_t u02 = vtrnq_s32(vreinterpretq_s32_s16(t01.val[0]),
                                 vreinterpretq_s32_s16(t23.val[0]));
    int32x4x2_t u13 = vtrnq_s32(vreinterpretq_s32_s16(t01.val[1]),
                                 vreinterpretq_s32_s16(t23.val[1]));
    int32x4x2_t u46 = vtrnq_s32(vreinterpretq_s32_s16(t45.val[0]),
                                 vreinterpretq_s32_s16(t67.val[0]));
    int32x4x2_t u57 = vtrnq_s32(vreinterpretq_s32_s16(t45.val[1]),
                                 vreinterpretq_s32_s16(t67.val[1]));

    // Final combining using 64-bit operations
    r0 = vreinterpretq_s16_s64(vcombine_s64(
        vget_low_s64(vreinterpretq_s64_s32(u02.val[0])),
        vget_low_s64(vreinterpretq_s64_s32(u46.val[0]))));
    r1 = vreinterpretq_s16_s64(vcombine_s64(
        vget_low_s64(vreinterpretq_s64_s32(u13.val[0])),
        vget_low_s64(vreinterpretq_s64_s32(u57.val[0]))));
    r2 = vreinterpretq_s16_s64(vcombine_s64(
        vget_low_s64(vreinterpretq_s64_s32(u02.val[1])),
        vget_low_s64(vreinterpretq_s64_s32(u46.val[1]))));
    r3 = vreinterpretq_s16_s64(vcombine_s64(
        vget_low_s64(vreinterpretq_s64_s32(u13.val[1])),
        vget_low_s64(vreinterpretq_s64_s32(u57.val[1]))));
    r4 = vreinterpretq_s16_s64(vcombine_s64(
        vget_high_s64(vreinterpretq_s64_s32(u02.val[0])),
        vget_high_s64(vreinterpretq_s64_s32(u46.val[0]))));
    r5 = vreinterpretq_s16_s64(vcombine_s64(
        vget_high_s64(vreinterpretq_s64_s32(u13.val[0])),
        vget_high_s64(vreinterpretq_s64_s32(u57.val[0]))));
    r6 = vreinterpretq_s16_s64(vcombine_s64(
        vget_high_s64(vreinterpretq_s64_s32(u02.val[1])),
        vget_high_s64(vreinterpretq_s64_s32(u46.val[1]))));
    r7 = vreinterpretq_s16_s64(vcombine_s64(
        vget_high_s64(vreinterpretq_s64_s32(u13.val[1])),
        vget_high_s64(vreinterpretq_s64_s32(u57.val[1]))));
}

} // namespace neon
} // namespace vca

#endif // VCA_ARCH_ARM
