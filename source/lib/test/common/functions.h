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

#pragma once

#include <test/common/EnumMapper.h>
#include <vcaLib.h>

#include <stdint.h>

namespace test {

const auto CpuSimdMapper = EnumMapper<CpuSimd>({{CpuSimd::None, "NoSimd"},
                                                {CpuSimd::SSE2, "SSE2"},
                                                {CpuSimd::SSSE3, "SSSE3"},
                                                {CpuSimd::SSE4, "SSE4"},
                                                {CpuSimd::AVX2, "AVX"}});

void fillBlockWithRandomData(int16_t *data, const unsigned blockSize, const unsigned bitDepth);

} // namespace test
