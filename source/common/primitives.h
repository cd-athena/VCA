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

#ifndef VCA_PRIMITIVES_H
#define VCA_PRIMITIVES_H

#include "common.h"

namespace VCA_NS {

enum LumaPU
{
    // Square (the first 5 PUs match the block sizes)
    LUMA_4x4,   LUMA_8x8,   LUMA_16x16, LUMA_32x32, LUMA_64x64,
    // Rectangular
    LUMA_8x4,   LUMA_4x8,
    LUMA_16x8,  LUMA_8x16,
    LUMA_32x16, LUMA_16x32,
    LUMA_64x32, LUMA_32x64,
    // Asymmetrical (0.75, 0.25)
    LUMA_16x12, LUMA_12x16, LUMA_16x4,  LUMA_4x16,
    LUMA_32x24, LUMA_24x32, LUMA_32x8,  LUMA_8x32,
    LUMA_64x48, LUMA_48x64, LUMA_64x16, LUMA_16x64,
    NUM_PU_SIZES
};

enum LumaCU // can be indexed using log2n(width)-2
{
    BLOCK_4x4,
    BLOCK_8x8,
    BLOCK_16x16,
    BLOCK_32x32,
    BLOCK_64x64,
    NUM_CU_SIZES
};

enum { NUM_TR_SIZE = 4 }; // TU are 4x4, 8x8, 16x16, and 32x32


typedef void (*dct_t)(const int16_t* src, int16_t* dst, intptr_t srcStride);
typedef void (*idct_t)(const int16_t* src, int16_t* dst, intptr_t dstStride);
typedef void (*denoiseDct_t)(int16_t* dctCoef, uint32_t* resSum, const uint16_t* offset, int numCoeff);
typedef void (*copy_ss_t)(int16_t* dst, intptr_t dstStride, const int16_t* src, intptr_t srcStride);
typedef void (*planecopy_cp_t) (const uint8_t* src, intptr_t srcStride, pixel* dst, intptr_t dstStride, int width, int height, int shift);
typedef void (*planecopy_sp_t) (const uint16_t* src, intptr_t srcStride, pixel* dst, intptr_t dstStride, int width, int height, int shift, uint16_t mask);

struct EncoderPrimitives
{
    struct CU
    {
        dct_t           dct;    // active dct transformation
        idct_t          idct;   // active idct transformation
        dct_t           standard_dct;   // original dct function, used by lowpass_dct
        dct_t           lowpass_dct;    // lowpass dct approximation
        copy_ss_t       copy_ss;
    }
    cu[NUM_CU_SIZES];

    dct_t                 dst4x4;
    idct_t                idst4x4;
    planecopy_cp_t        planecopy_cp;
    planecopy_sp_t        planecopy_sp;
    planecopy_sp_t        planecopy_sp_shl;
};

/* This copy of the table is what gets used by the encoder */
extern EncoderPrimitives primitives;

void setupCPrimitives(EncoderPrimitives &p);

}

#endif // ifndef VCA_PRIMITIVES_H
