
#pragma once

#include "vcaLib.h"

#include <queue>

namespace vca {

class Analyzer
{
public:
    Analyzer(vca_param cfg);
    ~Analyzer() = default;

    push_result push(vca_frame *frame);
    vca_frame_results pullResult();

private:
    vca_param cfg{};

    std::queue<vca_frame_results> readyResults;
};

} // namespace vca
