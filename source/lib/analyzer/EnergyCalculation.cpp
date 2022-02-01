/*****************************************************************************
 * Copyright (C) 2022 Christian Doppler Laboratory ATHENA
 *
 * Authors: Vignesh V Menon <vignesh.menon@aau.at>
 *          Christian Feldmann <christian.feldmann@bitmovin.com>
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

#include "EnergyCalculation.h"
#include "DCTTransforms.h"
#include "simd/dct-ssse3.h"

extern "C" {
#include "simd/dct8.h"
}

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace {

// TODO: Convert this into a integer operation. That should be possible in 16 bit
//       arithmetic with the same precision.

static const int16_t weights_dct8[64] = {
    0,  27, 94,  94,  94,  94,  94,  95,  27, 94, 94,  95,  96,  97,  98,  99,
    94, 94, 95,  97,  99,  101, 104, 107, 94, 95, 97,  99,  103, 107, 113, 120,
    94, 96, 99,  103, 109, 116, 126, 138, 94, 97, 101, 107, 116, 128, 144, 164,
    94, 98, 104, 113, 126, 144, 168, 201, 95, 99, 107, 120, 138, 164, 201, 255,
};

static const int16_t weights_dct16[256] = {
    0,   27,  93,  93,  93,  93,  93,  93,  93,  93,  93,  94,  94,  94,  94,  94,  27,  93,  93,
    93,  93,  94,  94,  94,  94,  94,  94,  94,  94,  94,  95,  95,  93,  93,  93,  94,  94,  94,
    94,  94,  94,  95,  95,  95,  96,  96,  96,  97,  93,  93,  94,  94,  94,  94,  94,  95,  95,
    96,  96,  97,  97,  98,  99,  99,  93,  93,  94,  94,  94,  95,  95,  96,  96,  97,  98,  99,
    100, 101, 102, 103, 93,  94,  94,  94,  95,  95,  96,  97,  98,  99,  100, 101, 102, 104, 106,
    107, 93,  94,  94,  94,  95,  96,  97,  98,  99,  101, 102, 104, 106, 108, 110, 113, 93,  94,
    94,  95,  96,  97,  98,  99,  101, 103, 105, 107, 110, 113, 116, 120, 93,  94,  94,  95,  96,
    98,  99,  101, 103, 106, 108, 112, 115, 119, 123, 128, 93,  94,  95,  96,  97,  99,  101, 103,
    106, 109, 112, 116, 121, 126, 132, 138, 93,  94,  95,  96,  98,  100, 102, 105, 108, 112, 117,
    122, 128, 134, 142, 150, 94,  94,  95,  97,  99,  101, 104, 107, 112, 116, 122, 128, 135, 144,
    153, 164, 94,  94,  96,  97,  100, 102, 106, 110, 115, 121, 128, 135, 145, 155, 167, 181, 94,
    94,  96,  98,  101, 104, 108, 113, 119, 126, 134, 144, 155, 168, 183, 201, 94,  95,  96,  99,
    102, 106, 110, 116, 123, 132, 142, 153, 167, 183, 203, 225, 94,  95,  97,  99,  103, 107, 113,
    120, 128, 138, 150, 164, 181, 201, 225, 255,
};

static const int16_t weights_dct32[1024] = {
    0,   27,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,
    93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  27,  93,  93,  93,  93,  93,
    93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  94,  94,
    94,  94,  94,  94,  94,  94,  94,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,
    93,  93,  93,  94,  94,  94,  94,  94,  94,  94,  94,  94,  94,  94,  94,  94,  94,  94,  94,
    94,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  93,  94,  94,  94,  94,  94,  94,  94,
    94,  94,  94,  94,  94,  94,  94,  94,  94,  94,  95,  95,  95,  95,  93,  93,  93,  93,  93,
    93,  93,  93,  93,  94,  94,  94,  94,  94,  94,  94,  94,  94,  94,  94,  94,  94,  94,  95,
    95,  95,  95,  95,  95,  95,  95,  96,  93,  93,  93,  93,  93,  93,  93,  94,  94,  94,  94,
    94,  94,  94,  94,  94,  94,  94,  94,  95,  95,  95,  95,  95,  95,  96,  96,  96,  96,  96,
    96,  97,  93,  93,  93,  93,  93,  93,  94,  94,  94,  94,  94,  94,  94,  94,  94,  94,  95,
    95,  95,  95,  95,  95,  96,  96,  96,  96,  97,  97,  97,  97,  98,  98,  93,  93,  93,  93,
    93,  94,  94,  94,  94,  94,  94,  94,  94,  94,  95,  95,  95,  95,  95,  96,  96,  96,  96,
    97,  97,  97,  98,  98,  98,  99,  99,  99,  93,  93,  93,  93,  93,  94,  94,  94,  94,  94,
    94,  94,  95,  95,  95,  95,  95,  96,  96,  96,  97,  97,  97,  98,  98,  98,  99,  99,  100,
    100, 101, 101, 93,  93,  93,  93,  94,  94,  94,  94,  94,  94,  94,  95,  95,  95,  95,  96,
    96,  96,  97,  97,  97,  98,  98,  99,  99,  100, 100, 101, 101, 102, 102, 103, 93,  93,  93,
    93,  94,  94,  94,  94,  94,  94,  95,  95,  95,  95,  96,  96,  96,  97,  97,  98,  98,  99,
    99,  100, 100, 101, 102, 102, 103, 104, 104, 105, 93,  93,  93,  94,  94,  94,  94,  94,  94,
    95,  95,  95,  96,  96,  96,  97,  97,  98,  98,  99,  99,  100, 100, 101, 102, 102, 103, 104,
    105, 106, 107, 107, 93,  93,  93,  94,  94,  94,  94,  94,  95,  95,  95,  96,  96,  96,  97,
    97,  98,  98,  99,  100, 100, 101, 102, 102, 103, 104, 105, 106, 107, 108, 109, 110, 93,  93,
    93,  94,  94,  94,  94,  94,  95,  95,  95,  96,  96,  97,  97,  98,  99,  99,  100, 101, 101,
    102, 103, 104, 105, 106, 107, 108, 109, 110, 112, 113, 93,  93,  93,  94,  94,  94,  94,  95,
    95,  95,  96,  96,  97,  97,  98,  99,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
    110, 112, 113, 115, 116, 93,  93,  94,  94,  94,  94,  94,  95,  95,  96,  96,  97,  97,  98,
    99,  99,  100, 101, 102, 103, 104, 105, 106, 107, 109, 110, 112, 113, 115, 116, 118, 120, 93,
    93,  94,  94,  94,  94,  95,  95,  95,  96,  96,  97,  98,  99,  99,  100, 101, 102, 103, 104,
    105, 107, 108, 109, 111, 113, 114, 116, 118, 120, 122, 124, 93,  93,  94,  94,  94,  94,  95,
    95,  96,  96,  97,  98,  98,  99,  100, 101, 102, 103, 104, 106, 107, 108, 110, 112, 113, 115,
    117, 119, 121, 123, 126, 128, 93,  93,  94,  94,  94,  94,  95,  95,  96,  97,  97,  98,  99,
    100, 101, 102, 103, 104, 106, 107, 109, 110, 112, 114, 116, 118, 120, 122, 125, 127, 130, 133,
    93,  93,  94,  94,  94,  95,  95,  96,  96,  97,  98,  99,  100, 101, 102, 103, 104, 106, 107,
    109, 110, 112, 114, 116, 119, 121, 123, 126, 129, 132, 135, 138, 93,  93,  94,  94,  94,  95,
    95,  96,  97,  97,  98,  99,  100, 101, 103, 104, 105, 107, 109, 110, 112, 114, 117, 119, 122,
    124, 127, 130, 133, 136, 140, 144, 93,  93,  94,  94,  94,  95,  95,  96,  97,  98,  99,  100,
    101, 102, 104, 105, 107, 108, 110, 112, 114, 117, 119, 122, 125, 128, 131, 134, 138, 142, 146,
    150, 93,  93,  94,  94,  94,  95,  96,  96,  97,  98,  99,  100, 102, 103, 105, 106, 108, 110,
    112, 114, 117, 119, 122, 125, 128, 131, 135, 139, 143, 147, 152, 157, 93,  94,  94,  94,  95,
    95,  96,  97,  98,  99,  100, 101, 102, 104, 106, 107, 109, 112, 114, 116, 119, 122, 125, 128,
    132, 135, 140, 144, 148, 153, 159, 164, 93,  94,  94,  94,  95,  95,  96,  97,  98,  99,  100,
    102, 103, 105, 107, 109, 111, 113, 116, 119, 122, 125, 128, 132, 136, 140, 144, 149, 154, 160,
    166, 172, 93,  94,  94,  94,  95,  96,  96,  97,  98,  100, 101, 102, 104, 106, 108, 110, 113,
    115, 118, 121, 124, 128, 131, 135, 140, 145, 150, 155, 161, 167, 174, 181, 93,  94,  94,  94,
    95,  96,  97,  98,  99,  100, 102, 103, 105, 107, 109, 112, 114, 117, 120, 123, 127, 131, 135,
    140, 144, 150, 155, 161, 168, 175, 182, 191, 93,  94,  94,  94,  95,  96,  97,  98,  99,  101,
    102, 104, 106, 108, 110, 113, 116, 119, 122, 126, 130, 134, 139, 144, 149, 155, 161, 168, 175,
    183, 192, 201, 93,  94,  94,  95,  95,  96,  97,  98,  100, 101, 103, 105, 107, 109, 112, 115,
    118, 121, 125, 129, 133, 138, 143, 148, 154, 161, 168, 175, 184, 193, 202, 213, 93,  94,  94,
    95,  95,  96,  97,  99,  100, 102, 104, 106, 108, 110, 113, 116, 120, 123, 127, 132, 136, 142,
    147, 153, 160, 167, 175, 183, 193, 203, 214, 225, 93,  94,  94,  95,  95,  96,  98,  99,  101,
    102, 104, 107, 109, 112, 115, 118, 122, 126, 130, 135, 140, 146, 152, 159, 166, 174, 182, 192,
    202, 214, 226, 239, 93,  94,  94,  95,  96,  97,  98,  99,  101, 103, 105, 107, 110, 113, 116,
    120, 124, 128, 133, 138, 144, 150, 157, 164, 172, 181, 191, 201, 213, 225, 239, 255,
};

static const double E_norm_factor = 90;
static const double h_norm_factor = 18;

uint32_t calculateWeightedCoeffSum(unsigned blockSize, int16_t *coeffBuffer)
{
    uint32_t weightedSum = 0;

    auto weightFactorMatrix = weights_dct32;
    switch (blockSize)
    {
        case 32:
            weightFactorMatrix = weights_dct32;
            break;
        case 16:
            weightFactorMatrix = weights_dct16;
            break;
        case 8:
            weightFactorMatrix = weights_dct8;
            break;
    }

    for (unsigned i = 0; i < blockSize * blockSize; i++)
    {
        auto weightedCoeff = (uint32_t)((weightFactorMatrix[i] * std::abs(coeffBuffer[i])) >> 8);
        weightedSum += weightedCoeff;
    }

    return weightedSum;
}

void copyPixelValuesToBuffer(unsigned blockOffsetLuma,
                             unsigned blockSize,
                             unsigned bitDepth,
                             uint8_t *src,
                             unsigned srcStride,
                             int16_t *buffer)
{
    if (bitDepth == 8)
    {
        src += blockOffsetLuma;
        for (unsigned y = 0; y < blockSize; y++, src += srcStride)
            for (unsigned x = 0; x < blockSize; x++)
                *(buffer++) = int16_t(src[x]);
    }
    else
    {
        auto input = (int16_t *) (src) + blockOffsetLuma;
        for (unsigned y = 0; y < blockSize; y++, input += srcStride / 2, buffer += blockSize)
            std::memcpy(buffer, input, blockSize * sizeof(int16_t));
    }
}

template<int bitDepth>
void copyPixelValuesToBufferWithPadding(unsigned blockOffsetLuma,
                                        unsigned blockSize,
                                        uint8_t *srcData,
                                        unsigned srcStride,
                                        int16_t *buffer,
                                        unsigned paddingRight,
                                        unsigned paddingBottom)
{
    typedef typename std::conditional<bitDepth == 8, uint8_t *, int16_t *>::type InValueType;
    static_assert(bitDepth >= 8 && bitDepth <= 16);

    const auto *__restrict src = InValueType(srcData);

    src += blockOffsetLuma;
    unsigned y          = 0;
    auto bufferLastLine = buffer;
    for (; y < blockSize - paddingBottom; y++, src += srcStride)
    {
        unsigned x     = 0;
        bufferLastLine = buffer;
        for (; x < blockSize - paddingRight; x++)
            *(buffer++) = int16_t(src[x]);
        const auto lastValue = int16_t(src[x]);
        for (; x < blockSize; x++)
            *(buffer++) = lastValue;
    }
    for (; y < blockSize; y++)
    {
        for (unsigned x = 0; x < blockSize; x++)
            *(buffer++) = bufferLastLine[x];
    }
}

void performDCT(unsigned blockSize, int16_t *pixelBuffer, int16_t *coeffBuffer, CpuSimd cpuSimd)
{
    switch (blockSize)
    {
        case 32:
#if ENABLE_ASSEMBLY
            if (cpuSimd == CpuSimd::AVX2)
                vca_dct32_avx2(pixelBuffer, coeffBuffer, 32);
            else if (cpuSimd == CpuSimd::SSSE3)
                vca_dct32_ssse3(pixelBuffer, coeffBuffer, 32);
            else
#endif
                vca::dct32_c(pixelBuffer, coeffBuffer, 32);
            break;
        case 16:
#if ENABLE_ASSEMBLY
            if (cpuSimd == CpuSimd::AVX2)
                vca_dct16_avx2(pixelBuffer, coeffBuffer, 16);
            else if (cpuSimd == CpuSimd::SSSE3)
                vca_dct16_ssse3(pixelBuffer, coeffBuffer, 16);
            else
#endif
                vca::dct16_c(pixelBuffer, coeffBuffer, 16);
            break;
        case 8:
#if ENABLE_ASSEMBLY
            if (cpuSimd == CpuSimd::AVX2)
                vca_dct8_avx2(pixelBuffer, coeffBuffer, 8);
            else if (cpuSimd == CpuSimd::SSE4)
                vca_dct8_sse4(pixelBuffer, coeffBuffer, 8);
            else if (cpuSimd == CpuSimd::SSE2)
                vca_dct8_sse2(pixelBuffer, coeffBuffer, 8);
            else
#endif
                vca::dct8_c(pixelBuffer, coeffBuffer, 8);
            break;
        default:
            throw std::invalid_argument("Invalid block size " + std::to_string(blockSize));
    }
}

} // namespace

namespace vca {

void computeWeightedDCTEnergy(const Job &job, Result &result, unsigned blockSize, CpuSimd cpuSimd)
{
    const auto frame = job.frame;
    if (frame == nullptr)
        throw std::invalid_argument("Invalid frame pointer");

    const auto bitDepth = frame->info.bitDepth;

    if (frame->info.bitDepth > 8)
        throw std::invalid_argument("16 bit input not implemented yet");

    auto src       = frame->planes[0];
    auto srcStride = frame->stride[0];

    auto [widthInBlocks, heightInBlock] = getFrameSizeInBlocks(blockSize, frame->info);
    auto totalNumberBlocks              = widthInBlocks * heightInBlock;
    auto widthInPixels                  = widthInBlocks * blockSize;
    auto heightInPixels                 = heightInBlock * blockSize;

    if (result.energyPerBlock.size() < totalNumberBlocks)
        result.energyPerBlock.resize(totalNumberBlocks);

    // First, we will copy the source to a temporary buffer which has one int16_t value
    // per sample.
    //   - This may only be needed for 8 bit values. For 16 bit values we could also
    //     perform this directly from the source buffer. However, we should check the
    //     performance of that approach (i.e. the buffer may not be aligned)

    ALIGN_VAR_32(int16_t, pixelBuffer[32 * 32]);
    ALIGN_VAR_32(int16_t, coeffBuffer[32 * 32]);

    auto blockIndex       = 0u;
    uint32_t frameTexture = 0;
    for (unsigned blockY = 0; blockY < heightInPixels; blockY += blockSize)
    {
        auto paddingBottom = std::max(int(blockY + blockSize) - int(frame->info.height), 0);
        for (unsigned blockX = 0; blockX < widthInPixels; blockX += blockSize)
        {
            auto paddingRight    = std::max(int(blockX + blockSize) - int(frame->info.width), 0);
            auto blockOffsetLuma = blockX + (blockY * srcStride);

            if (paddingRight > 0 || paddingBottom > 0)
            {
                if (bitDepth == 8)
                    copyPixelValuesToBufferWithPadding<8>(blockOffsetLuma,
                                                          blockSize,
                                                          src,
                                                          srcStride,
                                                          pixelBuffer,
                                                          unsigned(paddingRight),
                                                          unsigned(paddingBottom));
                else
                    copyPixelValuesToBufferWithPadding<16>(blockOffsetLuma,
                                                           blockSize,
                                                           src,
                                                           srcStride / 2,
                                                           pixelBuffer,
                                                           unsigned(paddingRight),
                                                           unsigned(paddingBottom));
            }
            else
            {
                copyPixelValuesToBuffer(blockOffsetLuma,
                                        blockSize,
                                        bitDepth,
                                        src,
                                        srcStride,
                                        pixelBuffer);
            }

            performDCT(blockSize, pixelBuffer, coeffBuffer, cpuSimd);

            result.energyPerBlock[blockIndex] = calculateWeightedCoeffSum(blockSize, coeffBuffer);
            frameTexture += result.energyPerBlock[blockIndex];

            blockIndex++;
        }
    }

    result.averageEnergy = uint32_t((double) (frameTexture) / (totalNumberBlocks * E_norm_factor));
}

void computeTextureSAD(Result &result, const Result &resultsPreviousFrame)
{
    if (result.energyPerBlock.size() != resultsPreviousFrame.energyPerBlock.size())
        throw std::out_of_range("Size of energy result vector must match");

    auto totalNumberBlocks = result.energyPerBlock.size();
    if (result.sadPerBlock.size() < totalNumberBlocks)
        result.sadPerBlock.resize(totalNumberBlocks);

    double textureSad = 0.0;
    for (size_t i = 0; i < totalNumberBlocks; i++)
    {
        result.sadPerBlock[i] = uint32_t(
            std::abs(int(result.energyPerBlock[i]) - int(resultsPreviousFrame.energyPerBlock[i])));
        textureSad += result.sadPerBlock[i];
    }

    result.sad = textureSad / (totalNumberBlocks * h_norm_factor);
}

} // namespace vca