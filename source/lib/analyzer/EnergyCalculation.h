/*****************************************************************************
 * Copyright (C) 2024 Christian Doppler Laboratory ATHENA
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

#pragma once

#include <analyzer/common/common.h>

namespace vca {

void computeWeightedDCTEnergy(const Job &job,
                              Result &result,
                              const unsigned blockSize,
                              CpuSimd cpuSimd,
                              bool enableChroma,
                              bool enableLowpass);
void computeTextureSAD(Result &results, const Result &resultsPreviousFrame);
void computeEntropy(const Job &job,
                    Result &result,
                    const unsigned blockSize,
                    CpuSimd cpuSimd,
                    bool enableLowpass,
                    bool enableChroma);
void computeEntropySAD(Result &results, const Result &resultsPreviousFrame);

} // namespace vca
