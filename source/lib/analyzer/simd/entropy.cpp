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

#include <immintrin.h> // Include SIMD intrinsics header for x86 architecture
#include <cmath>
#include <vector>
#include <unordered_map>

// x86 SIMD optimized entropy function

double entropy_avx2(const std::vector<int16_t> &block)
{
    //TODO
    double entropy = 0.0;
    return entropy;
}
