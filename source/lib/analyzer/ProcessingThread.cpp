#include "ProcessingThread.h"

#include "EnergyCalculation.h"
#include "common.h"

namespace vca {

MultiThreadIDList<Result> ProcessingThread::tempResultStorag;

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
        computeWeightedDCTEnergy(*job, result, this->cfg.blockSize);

        if (job->jobID > 0)
        {
            auto previousFrameJobID = job->jobID - 1;
            auto prevJobResults     = this->tempResultStorag.waitAndPop(previousFrameJobID);
            if (!prevJobResults)
            {
                break;
            }

            computeTextureSAD(result, *prevJobResults);

            auto sadNormalized     = result.sad / result.averageEnergy;
            auto sadNormalizedPrev = prevJobResults->sad / prevJobResults->averageEnergy;
            if (prevJobResults->sad > 0)
                result.epsilon = abs(sadNormalizedPrev - sadNormalized) / sadNormalizedPrev;
        }

        this->tempResultStorag.push(job->jobID, result);

        log(this->cfg,
            LogLevel::Debug,
            "Thread " + std::to_string(this->id) + ": Finished work on job " + job->infoString());

        results.push(result);
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
