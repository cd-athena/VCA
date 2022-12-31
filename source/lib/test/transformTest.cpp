#include <gtest/gtest.h>

namespace {

std::vector<int16_t> generateRandomBlock(const unsigned blockSize)
{
    const auto nrPixels = blockSize * blockSize;
    std::vector<int16_t> data;
    data.resize(nrPixels);

    return data;
}

} // namespace

TEST(TransformTest, FirstTest)
{
    const auto blockSize = 8u;
    const auto blockData = generateRandomBlock(blockSize);
}
