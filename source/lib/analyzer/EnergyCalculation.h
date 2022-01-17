#pragma once

#include "common.h"

namespace vca {

void computeWeightedDCTEnergy(const Job &job, Result &result, unsigned blockSize, CpuSimd cpuSimd);
void computeTextureSAD(Result &results, const Result &resultsPreviousFrame);

} // namespace vca
