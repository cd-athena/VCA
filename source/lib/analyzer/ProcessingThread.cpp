#include "ProcessingThread.h"

#include "EnergyCalculation.h"
#include "common.h"

namespace vca {

ProcessingThread::ProcessingThread(vca_param cfg,
                                   MultiThreadQueue<Job> &jobs,
                                   MultiThreadQueue<Result> &results,
                                   unsigned id)
{
    this->cfg = cfg;
    this->id  = id;

    this->thread = std::thread(&ProcessingThread::threadFunction,
                               this,
                               std::ref(jobs),
                               std::ref(results));
}

void ProcessingThread::threadFunction(MultiThreadQueue<Job> &jobQueue,
                                      MultiThreadQueue<Result> &results)
{
    while (!this->aborted)
    {
        auto job = jobQueue.waitAndPop();
        if (!job)
            break;

        log(this->cfg,
            LogLevel::Debug,
            "Thread " + std::to_string(this->id) + ": Start work on job " + job->infoString());

        Result result;
        result.poc = job->frame->stats.poc;
        computeWeightedDCTEnergy(*job, result, this->cfg.blockSize, this->cfg.cpuSimd);

        log(this->cfg,
            LogLevel::Debug,
            "Thread " + std::to_string(this->id) + ": Finished work on job " + job->infoString());

        results.waitAndPush(result);
    }

    log(this->cfg, LogLevel::Info, "Thread " + std::to_string(this->id) + " quit");
}

void ProcessingThread::abort()
{
    this->aborted = true;
}

void ProcessingThread::join()
{
    this->aborted = true;
    this->thread.join();
}

} // namespace vca
