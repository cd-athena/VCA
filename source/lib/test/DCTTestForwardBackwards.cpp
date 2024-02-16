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
#include <test/InverseDCTNative.h>
#include <test/common/functions.h>

#include <cstring>

namespace {

constexpr auto MAX_BLOCKSIZE_SAMPLES = 32 * 32;
constexpr auto MAX_BLOCKSIZE_BYTES   = MAX_BLOCKSIZE_SAMPLES * 2;

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

std::tuple<double, int> calculateMeanSquareErrorAndMaxDiff(int16_t *data1,
                                                           int16_t *data2,
                                                           const unsigned blockSize)
{
    double sumOfSquaredError = 0.0;
    const auto nrUsedPixels  = blockSize * blockSize;
    int maxDiff              = 0;
    for (unsigned i = 0; i < nrUsedPixels; i++)
    {
        const auto diff = data2[i] - data1[i];
        if (diff > maxDiff)
            maxDiff = diff;
        sumOfSquaredError += static_cast<double>(diff * diff);
    }
    return {sumOfSquaredError / nrUsedPixels, maxDiff};
}

} // namespace

using BlockSize = unsigned;
using BitDepth  = unsigned;
using MSE       = double;
using MaxDiff   = int;
using TestCase  = std::tuple<BlockSize, BitDepth, CpuSimd>;

class DCTTestForwardBackwardsFixture : public testing::TestWithParam<TestCase>
{
public:
    static std::string generateName(const ::testing::TestParamInfo<TestCase> &info)
    {
        const auto blockSize = std::get<0>(info.param);
        const auto bitDepth  = std::get<1>(info.param);
        const auto cpuSimd   = std::get<2>(info.param);
        return "BlockSize" + std::to_string(blockSize) + "_BitDpeht" + std::to_string(bitDepth)
               + "_" + vca::CpuSimdMapper.getName(cpuSimd);
    }
};

TEST_P(DCTTestForwardBackwardsFixture, TransformTest)
{
    const auto param = GetParam();

    const auto blockSize        = std::get<0>(param);
    const auto bitDepth         = std::get<1>(param);
    const auto cpuSimd          = std::get<2>(param);
    const auto enableLowpassDCT = false;

    if (!vca::isSimdSupported(cpuSimd))
        GTEST_SKIP() << "Skipping testing of " << vca::CpuSimdMapper.getName(cpuSimd)
                     << " because it is not supported on this platform.";

    ALIGN_VAR_32(int16_t, pixelBuffer[MAX_BLOCKSIZE_SAMPLES]);
    ALIGN_VAR_32(int16_t, coeffBuffer[MAX_BLOCKSIZE_SAMPLES]);
    ALIGN_VAR_32(int16_t, reconstructedPixels[MAX_BLOCKSIZE_SAMPLES]);

    std::memset(pixelBuffer, 0, MAX_BLOCKSIZE_BYTES);
    std::memset(coeffBuffer, 0, MAX_BLOCKSIZE_BYTES);
    std::memset(reconstructedPixels, 0, MAX_BLOCKSIZE_BYTES);

    test::fillBlockWithRandomData(pixelBuffer, blockSize, bitDepth);
    assertUnusedValuesAreZero(pixelBuffer, blockSize);

    vca::performDCT(blockSize, bitDepth, pixelBuffer, coeffBuffer, cpuSimd, enableLowpassDCT);
    assertUnusedValuesAreZero(coeffBuffer, blockSize);
    assertUsedValuesContainNonZeroValues(coeffBuffer, blockSize);

    test::performIDCT(blockSize, bitDepth, coeffBuffer, reconstructedPixels);
    assertUnusedValuesAreZero(coeffBuffer, blockSize);
    const auto [mse, maxDiff] = calculateMeanSquareErrorAndMaxDiff(pixelBuffer,
                                                                   reconstructedPixels,
                                                                   blockSize);

    // I got this table by experimentation (see commented code below).
    // There is probably a theoretical maximum error value one can calculate for this
    // forward/bacward transform combination.
    using BlockSizeAndBitDepth = std::tuple<BlockSize, BitDepth>;
    using MseAndDiff           = std::tuple<MSE, MaxDiff>;
    const std::map<BlockSizeAndBitDepth, MseAndDiff> ExpectedMaximumValues(
        {{{BlockSize(8u), BitDepth(8u)}, {MSE(0.2), MaxDiff(1)}},
         {{BlockSize(8u), BitDepth(10u)}, {MSE(2.0), MaxDiff(3)}},
         {{BlockSize(8u), BitDepth(12u)}, {MSE(25.0), MaxDiff(13)}},
         {{BlockSize(16u), BitDepth(8u)}, {MSE(0.5), MaxDiff(2)}},
         {{BlockSize(16u), BitDepth(10u)}, {MSE(7.0), MaxDiff(8)}},
         {{BlockSize(16u), BitDepth(12u)}, {MSE(90.0), MaxDiff(32)}},
         {{BlockSize(32u), BitDepth(8u)}, {MSE(0.5), MaxDiff(3)}},
         {{BlockSize(32u), BitDepth(10u)}, {MSE(6.0), MaxDiff(10)}},
         {{BlockSize(32u), BitDepth(12u)}, {MSE(90.0), MaxDiff(41)}}});

    const auto [maxExpectedMSE, maxExpectedDiff] = ExpectedMaximumValues.at({blockSize, bitDepth});
    ASSERT_LE(maxDiff, maxExpectedDiff);
    ASSERT_LE(mse, maxExpectedMSE);
}

INSTANTIATE_TEST_SUITE_P(
    DCRTransformTest,
    DCTTestForwardBackwardsFixture,
    testing::Combine(
        testing::ValuesIn({BlockSize(8u), BlockSize(16u), BlockSize(32u)}),
        testing::ValuesIn({BitDepth(8u), BitDepth(10u), BitDepth(12u)}),
        testing::ValuesIn(
            {CpuSimd::None, CpuSimd::SSE2, CpuSimd::SSSE3, CpuSimd::SSE4, CpuSimd::AVX2})),
    &DCTTestForwardBackwardsFixture::generateName);

// This code was used to get the results table above.
// TEST(TransformTest, TestMaxMSE)
// {
//     const auto cpuSimd          = CpuSimd::None;
//     const auto enableLowpassDCT = false;

//     ALIGN_VAR_32(int16_t, pixelBuffer[MAX_BLOCKSIZE_SAMPLES]);
//     ALIGN_VAR_32(int16_t, coeffBuffer[MAX_BLOCKSIZE_SAMPLES]);
//     ALIGN_VAR_32(int16_t, reconstructedPixels[MAX_BLOCKSIZE_SAMPLES]);

//     for (const auto blockSize : {8, 16, 32})
//     {
//         for (const auto bitDepth : {8, 10, 12})
//         {
//             const auto cpuSimd = CpuSimd::None;
//             auto maxMSE        = 0.0;
//             auto maxDiff       = 0;
//             for (int i = 0; i < 10000; i++)
//             {
//                 std::memset(pixelBuffer, 0, MAX_BLOCKSIZE_BYTES);
//                 std::memset(coeffBuffer, 0, MAX_BLOCKSIZE_BYTES);
//                 std::memset(reconstructedPixels, 0, MAX_BLOCKSIZE_BYTES);

//                 test::fillBlockWithRandomData(pixelBuffer, blockSize, bitDepth);
//                 assertUnusedValuesAreZero(pixelBuffer, blockSize);

//                 vca::performDCT(blockSize,
//                                 bitDepth,
//                                 pixelBuffer,
//                                 coeffBuffer,
//                                 cpuSimd,
//                                 enableLowpassDCT);
//                 assertUnusedValuesAreZero(coeffBuffer, blockSize);
//                 assertUsedValuesContainNonZeroValues(coeffBuffer, blockSize);

//                 test::performIDCT(blockSize, bitDepth, coeffBuffer, reconstructedPixels);
//                 assertUnusedValuesAreZero(coeffBuffer, blockSize);
//                 const auto [mse, maxDiffForIteration]
//                     = calculateMeanSquareErrorAndMaxDiff(pixelBuffer,
//                                                          reconstructedPixels,
//                                                          blockSize);
//                 if (mse > maxMSE)
//                     maxMSE = mse;
//                 if (maxDiffForIteration > maxDiff)
//                     maxDiff = maxDiffForIteration;
//             }

//             std::cout << "BlockSize " << blockSize << " BitDepth " << bitDepth << " "
//                       << vca::CpuSimdMapper.getName(cpuSimd) << " MaxMSE " << maxMSE << " maxDiff "
//                       << maxDiff << "\n";
//         }
//     }
// }
