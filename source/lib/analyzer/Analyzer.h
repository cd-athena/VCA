
#pragma once

#include "vcaLib.h"

#include "common.h"

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace vca {

class Analyzer
{
public:
    Analyzer(vca_param cfg);
    ~Analyzer();

    void abort();

    vca_result pushFrame(vca_frame *frame);
    bool resultAvailable();
    std::optional<vca_frame_results> pullResult();

private:
    vca_param cfg{};
    bool checkFrameSize(vca_frame_info frameInfo);
    std::optional<vca_frame_info> frameInfo;
    unsigned frameCounter{0};

    std::vector<std::thread> threadPool;
    void threadFunction(unsigned threadID);

    std::mutex resultsMutex;
    std::queue<vca_frame_results> results;
    std::condition_variable resultsCV;

    std::mutex jobsMutex;
    std::queue<Job> jobs;
    std::condition_variable jobsCV;

    bool aborted{};
};

} // namespace vca
