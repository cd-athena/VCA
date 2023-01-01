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

#include <gtest/gtest.h>
#include <random>

#include <analyzer/DCTTransform.h>
#include <test/EnumMapper.h>
#include <test/InverseDCTNative.h>

namespace {

constexpr auto MAX_BLOCKSIZE_SAMPLES = 32 * 32;
constexpr auto MAX_BLOCKSIZE_BYTES   = MAX_BLOCKSIZE_SAMPLES * 2;

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

void assertUnusedValuesAreZero(int16_t *data, const unsigned blockSize)
{
    const auto nrUsedPixels = blockSize * blockSize;
    for (unsigned i = nrUsedPixels; i < MAX_BLOCKSIZE_SAMPLES; i++)
        ASSERT_EQ(data[i], 0);
}

void assertUsedValuesContainNonZeroValues(int16_t *data, const unsigned blockSize)
{
    const auto nrUsedPixels = blockSize * blockSize;
    for (unsigned i = 0; i < nrUsedPixels; i++)
        if (data[i] != 0)
            return;
    FAIL();
}

double calculateMeanSquareError(int16_t *data1, int16_t *data2, const unsigned blockSize)
{
    double sumOfSquaredError = 0.0;
    const auto nrUsedPixels  = blockSize * blockSize;
    for (unsigned i = 0; i < nrUsedPixels; i++)
    {
        const auto diff = data2[i] - data1[i];
        sumOfSquaredError += static_cast<double>(diff * diff);
    }
    return sumOfSquaredError / nrUsedPixels;
}

const auto CpuSimdMapper = EnumMapper<CpuSimd>({{CpuSimd::None, "NoSimd"},
                                                {CpuSimd::SSE2, "SSE2"},
                                                {CpuSimd::SSSE3, "SSSE3"},
                                                {CpuSimd::SSE4, "SSE4"},
                                                {CpuSimd::AVX2, "AVX"}});

} // namespace

using BlockSize = unsigned;
using BitDepth  = unsigned;
using TestCase  = std::tuple<BlockSize, BitDepth, CpuSimd>;

class DCTTestFixture : public testing::TestWithParam<TestCase>
{
public:
    static std::string generateName(const ::testing::TestParamInfo<TestCase> &info)
    {
        const auto blockSize = std::get<0>(info.param);
        const auto bitDepth  = std::get<1>(info.param);
        const auto cpuSimd   = std::get<2>(info.param);
        return "BlockSize" + std::to_string(blockSize) + "_BitDpeht" + std::to_string(bitDepth)
               + "_" + CpuSimdMapper.getName(cpuSimd);
    }
};

TEST_P(DCTTestFixture, TransformTest)
{
    const auto param = GetParam();

    const auto blockSize        = std::get<0>(param);
    const auto bitDepth         = std::get<1>(param);
    const auto cpuSimd          = std::get<2>(param);
    const auto enableLowpassDCT = false;

    ALIGN_VAR_32(int16_t, pixelBuffer[MAX_BLOCKSIZE_SAMPLES]);
    ALIGN_VAR_32(int16_t, coeffBuffer[MAX_BLOCKSIZE_SAMPLES]);
    ALIGN_VAR_32(int16_t, reconstructedPixels[MAX_BLOCKSIZE_SAMPLES]);

    std::memset(pixelBuffer, 0, MAX_BLOCKSIZE_BYTES);
    std::memset(coeffBuffer, 0, MAX_BLOCKSIZE_BYTES);
    std::memset(reconstructedPixels, 0, MAX_BLOCKSIZE_BYTES);

    fillBlockWithRandomData(pixelBuffer, blockSize, bitDepth);
    assertUnusedValuesAreZero(pixelBuffer, blockSize);

    vca::performDCT(blockSize, bitDepth, pixelBuffer, coeffBuffer, cpuSimd, enableLowpassDCT);
    assertUnusedValuesAreZero(coeffBuffer, blockSize);
    assertUsedValuesContainNonZeroValues(coeffBuffer, blockSize);

    test::performIDCT(blockSize, bitDepth, coeffBuffer, reconstructedPixels);
    assertUnusedValuesAreZero(coeffBuffer, blockSize);
    const auto mse = calculateMeanSquareError(pixelBuffer, reconstructedPixels, blockSize);
    ASSERT_LE(mse, 0.3);
}

INSTANTIATE_TEST_SUITE_P(DCRTransformTest,
                         DCTTestFixture,
                         testing::Combine(testing::ValuesIn({8u, 16u, 32u}),
                                          testing::ValuesIn({8u, 10u, 12u}),
                                          testing::ValuesIn({CpuSimd::None, CpuSimd::SSE2})),
                         &DCTTestFixture::generateName);

// TEST(TransformTest, TestMaxMSE)
// {
//     const auto blockSize        = 8u;
//     const auto bitDepth         = 8u;
//     const auto cpuSimd          = CpuSimd::None;
//     const auto enableLowpassDCT = false;

//     ALIGN_VAR_32(int16_t, pixelBuffer[MAX_BLOCKSIZE_SAMPLES]);
//     ALIGN_VAR_32(int16_t, coeffBuffer[MAX_BLOCKSIZE_SAMPLES]);
//     ALIGN_VAR_32(int16_t, reconstructedPixels[MAX_BLOCKSIZE_SAMPLES]);

//     std::memset(pixelBuffer, 0, MAX_BLOCKSIZE_BYTES);
//     std::memset(coeffBuffer, 0, MAX_BLOCKSIZE_BYTES);
//     std::memset(reconstructedPixels, 0, MAX_BLOCKSIZE_BYTES);

//     auto maxMSE = 0.0;
//     for (int i = 0; i < 10000; i++)
//     {
//         fillBlockWithRandomData(pixelBuffer, blockSize, bitDepth);
//         assertUnusedValuesAreZero(pixelBuffer, blockSize);

//         vca::performDCT(blockSize, bitDepth, pixelBuffer, coeffBuffer, cpuSimd,
//         enableLowpassDCT); assertUnusedValuesAreZero(coeffBuffer, blockSize);
//         assertUsedValuesContainNonZeroValues(coeffBuffer, blockSize);

//         test::performIDCT(blockSize, bitDepth, coeffBuffer, reconstructedPixels);
//         assertUnusedValuesAreZero(coeffBuffer, blockSize);
//         const auto mse = calculateMeanSquareError(pixelBuffer, reconstructedPixels, blockSize);
//         if (mse > maxMSE)
//             maxMSE = mse;
//     }

//     std::cout << "MaxMSE " << maxMSE;

//     int debugStop = 123;
//     (void) debugStop;
// }