
#include "Analyzer.h"

#include <string>

namespace vca {

Analyzer::Analyzer(vca_param cfg)
{
    this->cfg      = cfg;
    auto nrThreads = cfg.nrFrameThreads * cfg.nrSliceThreads;
    log(cfg, LogLevel::Info, "Starting " + std::to_string(nrThreads) + " threads");
    for (unsigned i = 0; i < nrThreads; i++)
        this->threadPool.push_back(std::thread(&Analyzer::threadFunction, this, i));
}

Analyzer::~Analyzer()
{
    this->abort();
}

void Analyzer::abort()
{
    this->aborted = true;
    this->jobsCV.notify_all();
    for (auto &thread : this->threadPool)
        thread.join();
    this->threadPool.clear();
}

vca_result Analyzer::pushFrame(vca_frame *frame)
{
    if (!this->checkFrameSize(frame->info))
        return vca_result::VCA_ERROR;

    Job job;
    job.frame = frame;
    job.jobID = this->frameCounter;
    // job.macroblockRange = TODO

    std::unique_lock<std::mutex> lock(this->jobsMutex);
    this->jobs.push(job);
    this->jobsCV.notify_all();

    this->frameCounter++;

    return vca_result::VCA_OK;
}

bool Analyzer::resultAvailable()
{
    if (this->aborted)
        return false;

    std::unique_lock<std::mutex> lock(this->resultsMutex);
    return this->results.empty();
}

std::optional<vca_frame_results> Analyzer::pullResult()
{
    std::unique_lock<std::mutex> lock(this->resultsMutex);
    if (this->results.empty())
        this->resultsCV.wait(lock, [this]() { return !this->results.empty() || this->aborted; });

    if (this->aborted)
        return {};

    auto res = this->results.front();
    this->results.pop();
    return res;
}

bool Analyzer::checkFrameSize(vca_frame_info frameInfo)
{
    if (!this->frameInfo)
    {
        if (frameInfo.bitDepth < 8 || frameInfo.bitDepth > 16)
        {
            log(this->cfg,
                LogLevel::Error,
                "Frame with invalid bit " + std::to_string(frameInfo.bitDepth) + " depth provided");
            return false;
        }
        if (frameInfo.width == 0 || frameInfo.width % 2 != 0 || frameInfo.height == 0
            || frameInfo.height % 2 != 0)
        {
            log(this->cfg,
                LogLevel::Error,
                "Frame with invalid size " + std::to_string(frameInfo.width) + "x"
                    + std::to_string(frameInfo.height) + " depth provided");
            return false;
        }
        this->frameInfo = frameInfo;
    }

    if (frameInfo.bitDepth != this->frameInfo->bitDepth || frameInfo.width != this->frameInfo->width
        || frameInfo.height != this->frameInfo->height
        || frameInfo.colorspace != this->frameInfo->colorspace)
    {
        log(this->cfg, LogLevel::Error, "Frame with different settings revieved");
        return false;
    }

    return true;
}

void Analyzer::threadFunction(unsigned threadID)
{
    Job job;
    while (!this->aborted)
    {
        {
            std::unique_lock<std::mutex> lock(this->jobsMutex);
            this->jobsCV.wait(lock, [this]() { return !this->jobs.empty() || this->aborted; });

            job = this->jobs.front();
            this->jobs.pop();
        }

        log(this->cfg,
            LogLevel::Debug,
            "Thread " + std::to_string(threadID) + ": Start work on job " + job.infoString());

        // TODO: Actually process
        vca_frame_results result;
        result.poc        = job.frame->stats.poc;
        result.complexity = 22;

        log(this->cfg,
            LogLevel::Debug,
            "Thread " + std::to_string(threadID) + ": Finished work on job " + job.infoString());

        {
            std::unique_lock<std::mutex> lock(this->resultsMutex);
            this->results.push(result);
            this->resultsCV.notify_one();
        }
    }
}

} // namespace vca
