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

#include "EntropyNative.h"

namespace vca {

double entropy_c(const std::vector<int16_t> &block)
{
    std::unordered_map<int, int> pixelCounts;
    int totalPixels = static_cast<int>(block.size());

    // Count occurrences of each pixel value
    for (int pixel : block)
    {
        pixelCounts[pixel]++;
    }

    // Calculate probability of each pixel value
    std::vector<double> probabilities;
    for (const auto &pair : pixelCounts)
    {
        double probability = static_cast<double>(pair.second) / totalPixels;
        probabilities.push_back(probability);
    }

    // Calculate entropy
    double entropy = 0.0;
    for (double probability : probabilities)
    {
        entropy -= probability * log2(probability);
    }

    return entropy;
}

double entropy_lowpass_c(const std::vector<int16_t> &block, int width)
{
    // Check if width and height are divisible by 2
    if (width % 2 != 0)
    {
        return -1.0; // Error: Width is not divisible by 2
    }

    std::unordered_map<int, int> pixelCounts;

    // Downscale the block by averaging 2x2 blocks of pixels into a single pixel
    int downscaledWidth  = width >> 1;
    std::vector<int> downscaledBlock(downscaledWidth * downscaledWidth, 0);

    for (int i = 0; i < width; i += 2)
    {
        for (int j = 0; j < width; j += 2)
        {
            // Compute average pixel value of 2x2 block
            int sum = block[i * width + j] + block[i * width + j + 1] + block[(i + 1) * width + j]
                      + block[(i + 1) * width + j + 1];
            int averagePixel = sum >> 2;

            // Store the average pixel value in the downscaled block
            downscaledBlock[(i / 2) * downscaledWidth + (j / 2)] = averagePixel;

            // Count occurrences of the average pixel value
            pixelCounts[averagePixel]++;
        }
    }

    // Calculate probability of each pixel value in the downscaled block
    int totalPixels = downscaledWidth * downscaledWidth;
    std::vector<double> probabilities;
    for (const auto &pair : pixelCounts)
    {
        double probability = static_cast<double>(pair.second) / totalPixels;
        probabilities.push_back(probability);
    }

    // Calculate entropy of the downscaled block
    double entropy = 0.0;
    for (double probability : probabilities)
    {
        if (probability > 0.0)
        {
            entropy -= probability * log2(probability);
        }
    }

    return entropy;
}

}
