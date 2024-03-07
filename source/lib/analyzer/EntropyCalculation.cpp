/*****************************************************************************
 * Copyright (C) 2024 Christian Doppler Laboratory ATHENA
 *
 * Authors: Amritha Premkumar <amritha.premkumar@ieee.org>
 *          Prajit T Rajendran <prajit.rajendran@ieee.org>
 *          Vignesh V Menon <vignesh.menon@hhi.fraunhofer.de>
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
#include <analyzer/EntropyCalculation.h>
#include <analyzer/EntropyNative.h>
#include <analyzer/common/common.h>
#include <analyzer/simd/entropy.h>

#include <cstring>

namespace vca {

double performEntropy(const unsigned blockSize,
                      const unsigned bitDepth,
                      const int16_t *pixelBuffer,
                      CpuSimd cpuSimd,
                      bool enableLowpass)
{
    std::vector<int16_t> block(blockSize * blockSize);

    // Copy pixels from pixelBuffer to block
    for (uint32_t i = 0; i < blockSize; ++i)
    {
        for (uint32_t j = 0; j < blockSize; ++j)
        {
            block[i * blockSize + j] = pixelBuffer[i * blockSize + j];
        }
    }

    double entropy    = 0;
    // Calculate entropy
    //if (cpuSimd == CpuSimd::AVX2)
    //{
    //    entropy = entropy_avx2(block);
    //}
    //else
    if (enableLowpass)
        entropy = vca::entropy_lowpass_c(block, blockSize);
    else
        entropy = vca::entropy_c(block);

    return entropy;
}

double performEdgeDensity(const unsigned blockSize,
                          const unsigned bitDepth,
                          const int16_t *pixelBuffer,
                          CpuSimd cpuSimd,
                          bool enableLowpass)
{
    // Calculate the total number of pixels in the block
    unsigned blockSizeSq = blockSize * blockSize;

    // Threshold for edge detection based on bit depth
    int threshold = (1 << (bitDepth - 1)) - 1;

    // Initialize edge count to 0
    unsigned edgeCount = 0;

    // Iterate through the pixel buffer
    for (unsigned i = 0; i < blockSizeSq; ++i)
    {
        // Check edge conditions for pixels in the buffer
        if (i % blockSize < blockSize - 1 && abs(pixelBuffer[i] - pixelBuffer[i + 1]) > threshold)
        {
            // Horizontal edge detected
            edgeCount++;
        }
        if (i / blockSize < blockSize - 1
            && abs(pixelBuffer[i] - pixelBuffer[i + blockSize]) > threshold)
        {
            // Vertical edge detected
            edgeCount++;
        }
    }

    // Calculate edge density
    double density = static_cast<double>(edgeCount) / (2 * blockSize * (blockSize - 1));

    return density;
}

} // namespace vca
