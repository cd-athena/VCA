/*****************************************************************************
 * Copyright (C) 2025 Werner Robitza
 *
 * Authors: Werner Robitza <werner.robitza@gmail.com>
 *
 * Based on x265 HEVC encoder DCT implementation:
 * - https://github.com/videolan/x265/blob/master/source/common/dct.cpp
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

#pragma once

#include <stdint.h>

#if defined(VCA_ARCH_ARM) || defined(__aarch64__) || defined(_M_ARM64)

extern "C" {

// DCT8 functions for different bit depths
void vca_dct8_8bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride);
void vca_dct8_10bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride);
void vca_dct8_12bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride);

// DCT16 functions for different bit depths
void vca_dct16_8bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride);
void vca_dct16_10bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride);
void vca_dct16_12bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride);

// DCT32 functions for different bit depths
void vca_dct32_8bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride);
void vca_dct32_10bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride);
void vca_dct32_12bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride);

} // extern "C"

#else // Non-ARM fallback declarations (should never be called)

extern "C" {

void vca_dct8_8bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride);
void vca_dct8_10bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride);
void vca_dct8_12bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride);
void vca_dct16_8bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride);
void vca_dct16_10bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride);
void vca_dct16_12bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride);
void vca_dct32_8bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride);
void vca_dct32_10bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride);
void vca_dct32_12bit_neon(const int16_t *src, int16_t *dst, intptr_t srcStride);

} // extern "C"

#endif // VCA_ARCH_ARM
