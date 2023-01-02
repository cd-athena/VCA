/* Copyright (C) 2022 Christian Doppler Laboratory ATHENA
 *
 * Authors: Christian Feldmann <christian.feldmann@bitmovin.com>
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

#include "functions.h"

#include <random>

namespace test {

void fillBlockWithRandomData(int16_t *data, const unsigned blockSize, const unsigned bitDepth)
{
    const auto nrPixels = blockSize * blockSize;
    const auto maxValue = (1 << bitDepth) - 1;

    static std::random_device randomDevice;
    static std::default_random_engine randomEngine(randomDevice());
    static std::uniform_int_distribution<unsigned> uniform_dist(0, maxValue);

    for (size_t i = 0; i < nrPixels; i++)
        data[i] = int16_t(uniform_dist(randomEngine));
}

} // namespace test