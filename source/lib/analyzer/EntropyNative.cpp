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

}
