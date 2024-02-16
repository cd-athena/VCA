/* Copyright (C) 2024 Christian Doppler Laboratory ATHENA
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

#include <gtest/gtest.h>

#include <analyzer/DCTTransform.h>
#include <analyzer/common/common.h>
#include <analyzer/simd/cpu.h>
#include <test/common/functions.h>

#include <cstring>

namespace {

constexpr auto MAX_BLOCKSIZE_SAMPLES = 32 * 32;
constexpr auto MAX_BLOCKSIZE_BYTES   = MAX_BLOCKSIZE_SAMPLES * 2;

void assertUsedValuesAreIdentical(int16_t *data1, int16_t *data2, const unsigned blockSize)
{
    const auto nrUsedPixels = blockSize * blockSize;
    for (unsigned i = 0; i < nrUsedPixels; i++)
        ASSERT_EQ(data1[i], data2[i]);
}

} // namespace

using BlockSize = unsigned;
using BitDepth  = unsigned;
using TestCase  = std::tuple<BlockSize, BitDepth>;

class DCTTestImplementationsIdenticalOutputFixture : public testing::TestWithParam<TestCase>
{
public:
    static std::string generateName(const ::testing::TestParamInfo<TestCase> &info)
    {
        const auto blockSize = std::get<0>(info.param);
        const auto bitDepth  = std::get<1>(info.param);
        return "BlockSize" + std::to_string(blockSize) + "_BitDpeht" + std::to_string(bitDepth);
    }
};

TEST_P(DCTTestImplementationsIdenticalOutputFixture,
       TestThatAllImplementationsProduceIdenticalResults)
{
    const auto param = GetParam();

    const auto blockSize        = std::get<0>(param);
    const auto bitDepth         = std::get<1>(param);
    const auto enableLowpassDCT = false;

    ALIGN_VAR_32(int16_t, pixelBuffer[MAX_BLOCKSIZE_SAMPLES]);
    ALIGN_VAR_32(int16_t, coeffBufferNative[MAX_BLOCKSIZE_SAMPLES]);
    ALIGN_VAR_32(int16_t, coeffBufferTest[MAX_BLOCKSIZE_SAMPLES]);

    std::memset(pixelBuffer, 0, MAX_BLOCKSIZE_BYTES);
    std::memset(coeffBufferNative, 0, MAX_BLOCKSIZE_BYTES);
    std::memset(coeffBufferTest, 0, MAX_BLOCKSIZE_BYTES);

    test::fillBlockWithRandomData(pixelBuffer, blockSize, bitDepth);
    vca::performDCT(blockSize,
                    bitDepth,
                    pixelBuffer,
                    coeffBufferNative,
                    CpuSimd::None,
                    enableLowpassDCT);

    for (const auto cpuSimd : {CpuSimd::SSE2, CpuSimd::SSSE3, CpuSimd::SSE4, CpuSimd::AVX2})
    {
        if (!vca::isSimdSupported(cpuSimd))
        {
            std::cout << "Skipping testing of " << vca::CpuSimdMapper.getName(cpuSimd)
                      << " because it is not supported on this platform.";
            continue;
        }

        vca::performDCT(blockSize, bitDepth, pixelBuffer, coeffBufferTest, cpuSimd, enableLowpassDCT);
        assertUsedValuesAreIdentical(coeffBufferNative, coeffBufferTest, blockSize);
    }
}

INSTANTIATE_TEST_SUITE_P(
    DCRTransformTest,
    DCTTestImplementationsIdenticalOutputFixture,
    testing::Combine(testing::ValuesIn({BlockSize(8u), BlockSize(16u), BlockSize(32u)}),
                     testing::ValuesIn({BitDepth(8u), BitDepth(10u), BitDepth(12u)})),
    &DCTTestImplementationsIdenticalOutputFixture::generateName);
