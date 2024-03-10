/* Copyright (C) 2024 Christian Doppler Laboratory ATHENA
 *
 * Authors: Christian Feldmann <christian.feldmann@bitmovin.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 *****************************************************************************/

#include "ProcessingThread.h"

#include <analyzer/EnergyCalculation.h>
#include <analyzer/EntropyCalculation.h>

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
        result.poc   = job->frame->stats.poc;
        result.jobID = job->jobID;
        if (this->cfg.enableDCTenergy)
        {
            computeWeightedDCTEnergy(*job,
                                     result,
                                     this->cfg.blockSize,
                                     this->cfg.cpuSimd,
                                     this->cfg.enableEnergyChroma,
                                     this->cfg.enableLowpass);
        }
        if (this->cfg.enableEntropy)
        {
            computeEntropy(*job,
                           result,
                           this->cfg.blockSize,
                           this->cfg.cpuSimd,
                           this->cfg.enableLowpass,
                           this->cfg.enableEntropyChroma);
        }
        if (this->cfg.enableEdgeDensity)
        {
            computeEdgeDensity(*job,
                               result,
                               this->cfg.blockSize,
                               this->cfg.cpuSimd,
                               this->cfg.enableLowpass);
        }
        log(this->cfg,
            LogLevel::Debug,
            "Thread " + std::to_string(this->id) + ": Finished work on job " + job->infoString());

        results.waitAndPushInOrder(result, result.jobID);
    }

    log(this->cfg, LogLevel::Debug, "Thread " + std::to_string(this->id) + " quit");
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
