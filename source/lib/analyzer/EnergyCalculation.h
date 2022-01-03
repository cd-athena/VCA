#pragma once

#include "Common.h"

namespace vca {

void computeWeightedDCTEnergy(const Job &job, EnergyResult &result, unsigned blockSize);
double computeTextureSAD(const EnergyResult &results, const EnergyResult &resultsPreviousFrame);

} // namespace vca
