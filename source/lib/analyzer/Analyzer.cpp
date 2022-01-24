/* Copyright (C) 2022 Christian Doppler Laboratory ATHENA
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

#include "Analyzer.h"
#include "EnergyCalculation.h"
#include "simd/cpu.h"

#include <cstring>
#include <string>

namespace {

const std::map<CpuSimd, std::string> cpuSimdNames = {{CpuSimd::None, "None"},
                                                     {CpuSimd::SSE2, "SSE2"},
                                                     {CpuSimd::SSSE3, "SSSE3"},
                                                     {CpuSimd::SSE4, "SSE4"},
                                                     {CpuSimd::AVX2, "AVX2"}};

}

namespace vca {

Analyzer::Analyzer(vca_param cfg)
{
    this->cfg = cfg;
    this->jobs.setMaximumQueueSize(5);

    if (this->cfg.cpuSimd == CpuSimd::Autodetect)
    {
        this->cfg.cpuSimd = cpuDetectMaxSimd();
        log(cfg, LogLevel::Info, "Autodetected SIMD.");
    }
    log(cfg, LogLevel::Info, "Using SIMD " + cpuSimdNames.at(this->cfg.cpuSimd));

    if (cfg.nrFrameThreads == 0)
    {
        cfg.nrFrameThreads = std::thread::hardware_concurrency();
        log(cfg, LogLevel::Info, "Autodetect nr threads " + std::to_string(cfg.nrFrameThreads));
    }

    auto nrThreads = cfg.nrFrameThreads;
    log(cfg, LogLevel::Info, "Starting " + std::to_string(nrThreads) + " threads");
    for (unsigned i = 0; i < nrThreads; i++)
    {
        auto newThread = std::make_unique<ProcessingThread>(this->cfg, this->jobs, this->results, i);
        this->threadPool.push_back(std::move(newThread));
    }
}

Analyzer::~Analyzer()
{
    for (auto &thread : this->threadPool)
        thread->abort();
    this->jobs.abort();
    this->results.abort();
    for (auto &thread : this->threadPool)
        thread->join();
}

vca_result Analyzer::pushFrame(vca_frame *frame)
{
    if (!this->checkFrame(frame))
        return vca_result::VCA_ERROR;

    Job job;
    job.frame = frame;
    job.jobID = this->frameCounter;
    // job.macroblockRange = TODO

    this->jobs.waitAndPush(job);
    this->frameCounter++;

    return vca_result::VCA_OK;
}

bool Analyzer::resultAvailable()
{
    return !this->results.empty();
}

vca_result Analyzer::pullResult(vca_frame_results *outputResult)
{
    auto result = this->results.waitAndPop();
    if (!result)
        return vca_result::VCA_ERROR;

    if (this->previousResult)
    {
        computeTextureSAD(*result, *this->previousResult);

        auto sadNormalized     = result->sad / result->averageEnergy;
        auto sadNormalizedPrev = this->previousResult->sad / this->previousResult->averageEnergy;
        if (this->previousResult->sad > 0)
            result->epsilon = abs(sadNormalizedPrev - sadNormalized) / sadNormalizedPrev;
    }

    outputResult->poc           = result->poc;
    outputResult->jobID         = result->jobID;
    outputResult->averageEnergy = result->averageEnergy;
    outputResult->sad           = result->sad;
    outputResult->epsilon       = result->epsilon;
    if (outputResult->energyPerBlock)
        std::memcpy(outputResult->energyPerBlock,
                    result->energyPerBlock.data(),
                    result->energyPerBlock.size() * sizeof(uint32_t));
    if (outputResult->sadPerBlock)
        std::memcpy(outputResult->sadPerBlock,
                    result->sadPerBlock.data(),
                    result->sadPerBlock.size() * sizeof(uint32_t));

    this->previousResult = result;

    return vca_result::VCA_OK;
}

bool Analyzer::checkFrame(const vca_frame *frame)
{
    if (frame == nullptr)
    {
        log(this->cfg, LogLevel::Error, "Nullptr pushed");
        return false;
    }

    if (frame->planes[0] == nullptr || frame->stride[0] == 0)
    {
        log(this->cfg, LogLevel::Error, "No luma data provided");
        return false;
    }

    const auto &info = frame->info;

    if (!this->frameInfo)
    {
        if (info.bitDepth < 8 || info.bitDepth > 16)
        {
            log(this->cfg,
                LogLevel::Error,
                "Frame with invalid bit " + std::to_string(info.bitDepth) + " depth provided");
            return false;
        }
        if (info.width == 0 || info.width % 2 != 0 || info.height == 0 || info.height % 2 != 0)
        {
            log(this->cfg,
                LogLevel::Error,
                "Frame with invalid size " + std::to_string(info.width) + "x"
                    + std::to_string(info.height) + " depth provided");
            return false;
        }
        this->frameInfo = info;
    }

    if (info.bitDepth != this->frameInfo->bitDepth || info.width != this->frameInfo->width
        || info.height != this->frameInfo->height || info.colorspace != this->frameInfo->colorspace)
    {
        log(this->cfg, LogLevel::Error, "Frame with different settings revieved");
        return false;
    }

    return true;
}

} // namespace vca
