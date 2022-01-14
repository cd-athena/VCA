#pragma once

#include "vcaLib.h"

#include "MultiThreadIDList.h"
#include "MultiThreadQueue.h"
#include "common.h"
#include <thread>

namespace vca {

class ProcessingThread
{
public:
    ProcessingThread()                     = delete;
    ProcessingThread(ProcessingThread &&o) = delete;
    ProcessingThread(vca_param cfg,
                     MultiThreadQueue<Job> &jobs,
                     MultiThreadQueue<Result> &results,
                     unsigned id);
    ~ProcessingThread() = default;

    void abort();
    void join();

private:
    void threadFunction(MultiThreadQueue<Job> &jobQueue, MultiThreadQueue<Result> &results);

    std::thread thread;
    bool aborted{};
    unsigned id{};
    vca_param cfg;

    static MultiThreadIDList<Result> tempResultStorag;
};

} // namespace vca
