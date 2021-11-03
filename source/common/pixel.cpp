/*****************************************************************************
 * Copyright (C) 2021 Christian Doppler Laboratory ATHENA
 *
 * Authors: Steve Borho <steve@borho.org>
 *          Min Chen <chenm003@163.com>
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
#include <cstdlib> // abs()

using namespace VCA_NS;

namespace {
// place functions in anonymous namespace (file static)

static void planecopy_cp_c(const uint8_t* src, intptr_t srcStride, pixel* dst, intptr_t dstStride, int width, int height, int shift)
{
    for (int r = 0; r < height; r++)
    {
        for (int c = 0; c < width; c++)
            dst[c] = ((pixel)src[c]) << shift;

        dst += dstStride;
        src += srcStride;
    }
}

static void planecopy_sp_c(const uint16_t* src, intptr_t srcStride, pixel* dst, intptr_t dstStride, int width, int height, int shift, uint16_t mask)
{
    for (int r = 0; r < height; r++)
    {
        for (int c = 0; c < width; c++)
            dst[c] = (pixel)((src[c] >> shift) & mask);

        dst += dstStride;
        src += srcStride;
    }
}

static void planecopy_sp_shl_c(const uint16_t* src, intptr_t srcStride, pixel* dst, intptr_t dstStride, int width, int height, int shift, uint16_t mask)
{
    for (int r = 0; r < height; r++)
    {
        for (int c = 0; c < width; c++)
            dst[c] = (pixel)((src[c] << shift) & mask);

        dst += dstStride;
        src += srcStride;
    }
}

template<int bx, int by>
void blockcopy_ss_c(int16_t* a, intptr_t stridea, const int16_t* b, intptr_t strideb)
{
    for (int y = 0; y < by; y++)
    {
        for (int x = 0; x < bx; x++)
            a[x] = b[x];

        a += stridea;
        b += strideb;
    }
}

}  // end anonymous namespace

namespace VCA_NS {
// vca private namespace

/* Initialize entries for pixel functions defined in this file */
void setupPixelPrimitives_c(EncoderPrimitives &p)
{
#define LUMA_CU(W, H) \
    p.cu[BLOCK_ ## W ## x ## H].copy_ss       = blockcopy_ss_c<W, H>;

    LUMA_CU(4, 4);
    LUMA_CU(8, 8);
    LUMA_CU(16, 16);
    LUMA_CU(32, 32);
    LUMA_CU(64, 64)
    p.planecopy_cp = planecopy_cp_c;
    p.planecopy_sp = planecopy_sp_c;
    p.planecopy_sp_shl = planecopy_sp_shl_c;
}
}
