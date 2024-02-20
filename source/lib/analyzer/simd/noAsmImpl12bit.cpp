/*****************************************************************************
 * Copyright (C) 2024 Christian Doppler Laboratory ATHENA
 *
 * Authors: Vignesh V Menon <vignesh.menon@aau.at>
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

#include "dct8.h"

#include <assert.h>

/// In case we have no NASM available or disabled, we use these dummy functions to link (which
/// should never be called)

extern "C" {

void vca_dct4_12bit_sse2(const int16_t *src, int16_t *dst, intptr_t srcStride)
{
    assert(false);
}
void vca_dct8_12bit_sse2(const int16_t *src, int16_t *dst, intptr_t srcStride)
{
    assert(false);
}
void vca_dct8_12bit_sse4(const int16_t *src, int16_t *dst, intptr_t srcStride)
{
    assert(false);
}
void vca_dct4_12bit_avx2(const int16_t *src, int16_t *dst, intptr_t srcStride)
{
    assert(false);
}
void vca_dct8_12bit_avx2(const int16_t *src, int16_t *dst, intptr_t srcStride)
{
    assert(false);
}
void vca_dct16_12bit_avx2(const int16_t *src, int16_t *dst, intptr_t srcStride)
{
    assert(false);
}
void vca_dct32_12bit_avx2(const int16_t *src, int16_t *dst, intptr_t srcStride)
{
    assert(false);
}

}
