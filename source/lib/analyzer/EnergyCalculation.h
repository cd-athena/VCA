#pragma once

#include "Common.h"

namespace vca {

void computeWeightedDCTEnergy(const Job &job, Result &result, unsigned blockSize);
double computeTextureSAD(const Result &results, const Result &resultsPreviousFrame);

} // namespace vca
