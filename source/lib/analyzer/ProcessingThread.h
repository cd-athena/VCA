#pragma once

#include "vcaLib.h"

#include "MultiThreadQueue.h"
#include "common.h"
#include <thread>

namespace vca {

class ProcessingThread
{
public:
    ProcessingThread(vca_param cfg,
                     MultiThreadQueue<Job> &jobs,
                     MultiThreadQueue<Result> &results,
                     unsigned id);

    void abort();
    void join();

private:
    bool computeWeightedDCTEnergy(const vca_param &cfg,
                                  vca_frame *frame,
                                  vca_frame_results *m_texture);
    void threadFunction(MultiThreadQueue<Job> &jobQueue, MultiThreadQueue<Result> &results);

    std::thread thread;
    bool aborted{};
    unsigned id{};
    vca_param cfg;
};

} // namespace vca
