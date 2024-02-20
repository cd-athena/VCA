/*****************************************************************************
 * Copyright (C) 2024 Christian Doppler Laboratory ATHENA
 *
 * Authors: Mandar Gurav <mandar@multicorewareinc.com>
 *          Deepthi Devaki Akkoorath <deepthidevaki@multicorewareinc.com>
 *          Mahesh Pittala <mahesh@multicorewareinc.com>
 *          Rajesh Paulraj <rajesh@multicorewareinc.com>
 *          Min Chen <min.chen@multicorewareinc.com>
 *          Praveen Kumar Tiwari <praveen@multicorewareinc.com>
 *          Nabajit Deka <nabajit@multicorewareinc.com>
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

#include <analyzer/DCTTransform.h>

#include <analyzer/DCTTransformsNative.h>
#include <analyzer/common/common.h>
#include <analyzer/simd/dct-ssse3.h>
#include <analyzer/simd/dct8.h>

#include <cstring>

namespace vca {

void performDCTBlockSize32(const unsigned bitDepth,
                           int16_t *pixelBuffer,
                           int16_t *coeffBuffer,
                           CpuSimd cpuSimd)
{
    if (cpuSimd == CpuSimd::AVX2)
    {
        if (bitDepth == 8)
            vca_dct32_8bit_avx2(pixelBuffer, coeffBuffer, 32);
        else if (bitDepth == 10)
            vca_dct32_10bit_avx2(pixelBuffer, coeffBuffer, 32);
        else if (bitDepth == 12)
            vca_dct32_12bit_avx2(pixelBuffer, coeffBuffer, 32);
    }
    else if (cpuSimd == CpuSimd::SSSE3)
    {
        if (bitDepth == 8)
            vca_dct32_8bit_ssse3(pixelBuffer, coeffBuffer, 32);
        else if (bitDepth == 10)
            vca_dct32_10bit_ssse3(pixelBuffer, coeffBuffer, 32);
        else if (bitDepth == 12)
            vca_dct32_12bit_ssse3(pixelBuffer, coeffBuffer, 32);
    }
    else
        vca::dct32_c(pixelBuffer, coeffBuffer, 32, bitDepth);
}

void performDCTBlockSize16(const unsigned bitDepth,
                           int16_t *pixelBuffer,
                           int16_t *coeffBuffer,
                           CpuSimd cpuSimd)
{
    if (cpuSimd == CpuSimd::AVX2)
    {
        if (bitDepth == 8)
            vca_dct16_8bit_avx2(pixelBuffer, coeffBuffer, 16);
        else if (bitDepth == 10)
            vca_dct16_10bit_avx2(pixelBuffer, coeffBuffer, 16);
        else if (bitDepth == 12)
            vca_dct16_12bit_avx2(pixelBuffer, coeffBuffer, 16);
    }
    else if (cpuSimd == CpuSimd::SSSE3)
    {
        if (bitDepth == 8)
            vca_dct16_8bit_ssse3(pixelBuffer, coeffBuffer, 16);
        else if (bitDepth == 10)
            vca_dct16_10bit_ssse3(pixelBuffer, coeffBuffer, 16);
        else if (bitDepth == 12)
            vca_dct16_12bit_ssse3(pixelBuffer, coeffBuffer, 16);
    }
    else
        vca::dct16_c(pixelBuffer, coeffBuffer, 16, bitDepth);
}

void performDCTBlockSize8(const unsigned bitDepth,
                          int16_t *pixelBuffer,
                          int16_t *coeffBuffer,
                          CpuSimd cpuSimd)
{
    if (cpuSimd == CpuSimd::AVX2)
    {
        if (bitDepth == 8)
            vca_dct8_8bit_avx2(pixelBuffer, coeffBuffer, 8);
        else if (bitDepth == 10)
            vca_dct8_10bit_avx2(pixelBuffer, coeffBuffer, 8);
        else if (bitDepth == 12)
            vca_dct8_12bit_avx2(pixelBuffer, coeffBuffer, 8);
    }
    else if (cpuSimd == CpuSimd::SSE4)
    {
        if (bitDepth == 8)
            vca_dct8_8bit_sse4(pixelBuffer, coeffBuffer, 8);
        else if (bitDepth == 10)
            vca_dct8_10bit_sse4(pixelBuffer, coeffBuffer, 8);
        else if (bitDepth == 12)
            vca_dct8_12bit_sse4(pixelBuffer, coeffBuffer, 8);
    }
    else if (cpuSimd == CpuSimd::SSE2)
    {
        if (bitDepth == 8)
            vca_dct8_8bit_sse2(pixelBuffer, coeffBuffer, 8);
        else if (bitDepth == 10)
            vca_dct8_10bit_sse2(pixelBuffer, coeffBuffer, 8);
        else if (bitDepth == 12)
            vca_dct8_12bit_sse2(pixelBuffer, coeffBuffer, 8);
    }
    else
        vca::dct8_c(pixelBuffer, coeffBuffer, 8, bitDepth);
}

void performLowpassDCTBlockSize16(const unsigned bitDepth,
                                  const int16_t *src,
                                  int16_t *dst,
                                  CpuSimd cpuSimd)
{
    ALIGN_VAR_32(int16_t, coef[8 * 8]);
    ALIGN_VAR_32(int16_t, avgBlock[8 * 8]);
    int32_t totalSum = 0;
    int16_t sum      = 0;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            sum = src[2 * i * 16 + 2 * j] + src[2 * i * 16 + 2 * j + 1]
                  + src[(2 * i + 1) * 16 + 2 * j] + src[(2 * i + 1) * 16 + 2 * j + 1];
            avgBlock[i * 8 + j] = sum >> 2;
            totalSum += sum;
        }
    }

    if (cpuSimd == CpuSimd::AVX2)
    {
        if (bitDepth == 8)
            vca_dct8_8bit_avx2(avgBlock, coef, 8);
        else if (bitDepth == 10)
            vca_dct8_10bit_avx2(avgBlock, coef, 8);
        else if (bitDepth == 12)
            vca_dct8_12bit_avx2(avgBlock, coef, 8);
    }
    else if (cpuSimd == CpuSimd::SSE4)
    {
        if (bitDepth == 8)
            vca_dct8_8bit_sse4(avgBlock, coef, 8);
        else if (bitDepth == 10)
            vca_dct8_10bit_sse4(avgBlock, coef, 8);
        else if (bitDepth == 12)
            vca_dct8_12bit_sse4(avgBlock, coef, 8);
    }
    else if (cpuSimd == CpuSimd::SSE2)
    {
        if (bitDepth == 8)
            vca_dct8_8bit_sse2(avgBlock, coef, 8);
        else if (bitDepth == 10)
            vca_dct8_10bit_sse2(avgBlock, coef, 8);
        else if (bitDepth == 12)
            vca_dct8_12bit_sse2(avgBlock, coef, 8);
    }
    else
        vca::dct8_c(avgBlock, coef, 8, bitDepth);

    std::memset(dst, 0, 256 * sizeof(int16_t));
    for (int i = 0; i < 8; i++)
    {
        std::memcpy(&dst[i * 16], &coef[i * 8], 8 * sizeof(int16_t));
    }
    dst[0] = static_cast<int16_t>(totalSum >> 1);
}

void performLowpassDCTBlockSize32(const unsigned bitDepth,
                                  const int16_t *src,
                                  int16_t *dst,
                                  CpuSimd cpuSimd)
{
    ALIGN_VAR_32(int16_t, coef[16 * 16]);
    ALIGN_VAR_32(int16_t, avgBlock[16 * 16]);
    int32_t totalSum = 0;
    int16_t sum      = 0;
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++)
        {
            sum = src[2 * i * 32 + 2 * j] + src[2 * i * 32 + 2 * j + 1]
                  + src[(2 * i + 1) * 32 + 2 * j] + src[(2 * i + 1) * 32 + 2 * j + 1];
            avgBlock[i * 16 + j] = sum >> 2;
            totalSum += sum;
        }

    if (cpuSimd == CpuSimd::AVX2)
    {
        if (bitDepth == 8)
            vca_dct16_8bit_avx2(avgBlock, coef, 16);
        else if (bitDepth == 10)
            vca_dct16_10bit_avx2(avgBlock, coef, 16);
        else if (bitDepth == 12)
            vca_dct16_12bit_avx2(avgBlock, coef, 16);
    }
    else if (cpuSimd == CpuSimd::SSSE3)
    {
        if (bitDepth == 8)
            vca_dct16_8bit_ssse3(avgBlock, coef, 16);
        else if (bitDepth == 10)
            vca_dct16_10bit_ssse3(avgBlock, coef, 16);
        else if (bitDepth == 12)
            vca_dct16_12bit_ssse3(avgBlock, coef, 16);
    }
    else
        vca::dct16_c(avgBlock, coef, 16, bitDepth);
    std::memset(dst, 0, 1024 * sizeof(int16_t));
    for (int i = 0; i < 16; i++)
    {
        std::memcpy(&dst[i * 32], &coef[i * 16], 16 * sizeof(int16_t));
    }
    dst[0] = static_cast<int16_t>(totalSum >> 3);
}

void performDCT(const unsigned blockSize,
                const unsigned bitDepth,
                int16_t *pixelBuffer,
                int16_t *coeffBuffer,
                CpuSimd cpuSimd,
                bool enableLowpassDCT)
{
    if (bitDepth != 8 && bitDepth != 10 && bitDepth != 12)
        throw std::invalid_argument("Invalid bit depth " + std::to_string(bitDepth));

    switch (blockSize)
    {
        case 32:
            if (enableLowpassDCT)
                performLowpassDCTBlockSize32(bitDepth, pixelBuffer, coeffBuffer, cpuSimd);
            else
                performDCTBlockSize32(bitDepth, pixelBuffer, coeffBuffer, cpuSimd);
            break;
        case 16:
            if (enableLowpassDCT)
                performLowpassDCTBlockSize16(bitDepth, pixelBuffer, coeffBuffer, cpuSimd);
            else
                performDCTBlockSize16(bitDepth, pixelBuffer, coeffBuffer, cpuSimd);
            break;
        case 8:
            performDCTBlockSize8(bitDepth, pixelBuffer, coeffBuffer, cpuSimd);
            break;
        default:
            throw std::invalid_argument("Invalid block size " + std::to_string(blockSize));
    }
}

} // namespace vca
