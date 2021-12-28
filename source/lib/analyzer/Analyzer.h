
#pragma once

#include "vcaLib.h"

#include "common.h"
#include "MultiThreadQueue.h"
#include "ProcessingThread.h"

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

    vca_result pushFrame(vca_frame *frame);
    bool resultAvailable();
    vca_result pullResult(vca_frame_results *result);

private:
    vca_param cfg{};
    bool checkFrameSize(vca_frame_info frameInfo);
    std::optional<vca_frame_info> frameInfo;
    unsigned frameCounter{0};

    std::vector<ProcessingThread> threadPool;

    MultiThreadQueue<Job> jobs;
    MultiThreadQueue<Result> results;
};

} // namespace vca
