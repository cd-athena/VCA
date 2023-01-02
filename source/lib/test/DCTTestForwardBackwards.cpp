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

#include <analyzer/DCTTransform.h>
#include <test/InverseDCTNative.h>
#include <test/common/functions.h>

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
               + "_" + test::CpuSimdMapper.getName(cpuSimd);
    }
};

TEST_P(DCTTestForwardBackwardsFixture, TransformTest)
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

    ASSERT_LE(maxDiff, 2);
    if (blockSize == 32 && bitDepth == 12)
        ASSERT_LE(mse, 3.0);
    else
        ASSERT_LE(mse, 1.0);
}

INSTANTIATE_TEST_SUITE_P(
    DCRTransformTest,
    DCTTestForwardBackwardsFixture,
    testing::Combine(testing::ValuesIn({BlockSize(8u), BlockSize(16u), BlockSize(32u)}),
                     testing::ValuesIn({BitDepth(8u), BitDepth(10u), BitDepth(12u)}),
                     testing::ValuesIn({CpuSimd::None, CpuSimd::SSE2})),
    &DCTTestForwardBackwardsFixture::generateName);

// TEST(TransformTest, TestMaxMSE)
// {
//     const auto cpuSimd          = CpuSimd::None;
//     const auto enableLowpassDCT = false;

//     ALIGN_VAR_32(int16_t, pixelBuffer[MAX_BLOCKSIZE_SAMPLES]);
//     ALIGN_VAR_32(int16_t, coeffBuffer[MAX_BLOCKSIZE_SAMPLES]);
//     ALIGN_VAR_32(int16_t, reconstructedPixels[MAX_BLOCKSIZE_SAMPLES]);

//     std::memset(pixelBuffer, 0, MAX_BLOCKSIZE_BYTES);
//     std::memset(coeffBuffer, 0, MAX_BLOCKSIZE_BYTES);
//     std::memset(reconstructedPixels, 0, MAX_BLOCKSIZE_BYTES);

//     for (const auto blockSize : {8, 16, 32})
//     {
//         for (const auto bitDepth : {8, 10, 12})
//         {
//             auto maxMSE = 0.0;
//             for (int i = 0; i < 10000; i++)
//             {
//                 fillBlockWithRandomData(pixelBuffer, blockSize, bitDepth);
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
//                 const auto mse = calculateMeanSquareError(pixelBuffer,
//                                                           reconstructedPixels,
//                                                           blockSize);
//                 if (mse > maxMSE)
//                     maxMSE = mse;
//             }

//             std::cout << "BlockSize " << blockSize << " BitDepth " << bitDepth << "MaxMSE "
//                       << maxMSE << "\n";
//         }
//     }

//     int debugStop = 123;
//     (void) debugStop;
// }