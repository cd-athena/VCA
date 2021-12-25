
#pragma once

#include "vcaLib.h"

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>


namespace vca {

class Analyzer
{
public:
    Analyzer(vca_param cfg);
    ~Analyzer() = default;

    void abort();

    vca_result pushFrame(vca_frame *frame);
    bool resultAvailable();
    std::optional<vca_frame_results> pullResult();

private:
    vca_param cfg{};

    std::mutex resultsMutex;
    std::queue<vca_frame_results> results;
    std::condition_variable resultsCV;

    bool aborted{};
};

} // namespace vca
