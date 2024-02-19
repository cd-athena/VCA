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

#include <analyzer/Analyzer.h>
#include <analyzer/EnergyCalculation.h>
#include <analyzer/EntropyCalculation.h>
#include <analyzer/simd/cpu.h>

#include <cstring>
#include <map>
#include <string>

namespace vca {

Analyzer::Analyzer(vca_param cfg)
{
    this->cfg = cfg;
    this->jobs.setMaximumQueueSize(5);

    const auto blockSize = this->cfg.blockSize;
    if (blockSize != 8 && blockSize != 16 && blockSize != 32)
    {
        log(cfg, LogLevel::Error, "Invalid block size: " + std::to_string(this->cfg.blockSize));
        throw std::invalid_argument("Invalid block size");
    }
    log(cfg, LogLevel::Info, "Block size: " + std::to_string(this->cfg.blockSize));

    const auto bitDepth = this->cfg.frameInfo.bitDepth;
    if (bitDepth != 8 && bitDepth != 10 && bitDepth != 12)
    {
        log(cfg, LogLevel::Error, "Invalid bit depth: " + std::to_string(bitDepth));
        throw std::invalid_argument("Invalid bit depth");
    }

    if (this->cfg.cpuSimd == CpuSimd::Autodetect)
    {
        this->cfg.cpuSimd = cpuDetectMaxSimd();
        log(cfg, LogLevel::Info, "Autodetected SIMD.");
    }
    else if (this->cfg.cpuSimd != CpuSimd::None)
    {
        if (!isSimdSupported(this->cfg.cpuSimd))
        {
            this->cfg.cpuSimd = cpuDetectMaxSimd();
            log(cfg,
                LogLevel::Warning,
                "The selected SIMD is not available on this CPU (). Lowering it.");
        }
    }
    log(cfg, LogLevel::Info, "Using SIMD " + CpuSimdMapper.getName(this->cfg.cpuSimd));

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
        if (this->cfg.enableDCTenergy)
        {
            computeTextureSAD(*result, *this->previousResult);
            auto sadNormalized     = result->energyDiff / result->averageEnergy;
            auto sadNormalizedPrev = this->previousResult->energyDiff
                                     / this->previousResult->averageEnergy;
            if (this->previousResult->energyDiff > 0)
                result->epsilon = abs(sadNormalizedPrev - sadNormalized) / sadNormalizedPrev;
        }
        if (this->cfg.enableEntropy)
        {
            computeEntropySAD(*result, *this->previousResult);
            auto entropyDiff     = result->entropyDiff;
            auto entropyDiffPrev = this->previousResult->entropyDiff;
            if (this->previousResult->entropyDiff > 0)
                result->entropyEpsilon = abs(entropyDiffPrev - entropyDiff);
        }
    }

    outputResult->poc               = result->poc;
    outputResult->jobID             = result->jobID;

    if (this->cfg.enableDCTenergy)
    {
        outputResult->averageBrightness = result->averageBrightness;
        outputResult->averageEnergy     = result->averageEnergy;
        outputResult->energyDiff        = result->energyDiff;
        outputResult->epsilon           = result->epsilon;

        if (outputResult->brightnessPerBlock)
            std::memcpy(outputResult->brightnessPerBlock,
                        result->brightnessPerBlock.data(),
                        result->brightnessPerBlock.size() * sizeof(uint32_t));
        if (outputResult->energyPerBlock)
            std::memcpy(outputResult->energyPerBlock,
                        result->energyPerBlock.data(),
                        result->energyPerBlock.size() * sizeof(uint32_t));
        if (outputResult->energyDiffPerBlock)
            std::memcpy(outputResult->energyDiffPerBlock,
                        result->energyDiffPerBlock.data(),
                        result->energyDiffPerBlock.size() * sizeof(uint32_t));
        if (this->cfg.enableChroma)
        {
            outputResult->averageU = result->averageU;
            outputResult->averageV = result->averageV;
            outputResult->energyU  = result->energyU;
            outputResult->energyV  = result->energyV;
            if (outputResult->averageUPerBlock)
                std::memcpy(outputResult->averageUPerBlock,
                            result->averageUPerBlock.data(),
                            result->averageUPerBlock.size() * sizeof(uint32_t));
            if (outputResult->averageVPerBlock)
                std::memcpy(outputResult->averageVPerBlock,
                            result->averageVPerBlock.data(),
                            result->averageVPerBlock.size() * sizeof(uint32_t));
            if (outputResult->energyUPerBlock)
                std::memcpy(outputResult->energyUPerBlock,
                            result->energyUPerBlock.data(),
                            result->energyUPerBlock.size() * sizeof(uint32_t));
            if (outputResult->energyVPerBlock)
                std::memcpy(outputResult->energyVPerBlock,
                            result->energyVPerBlock.data(),
                            result->energyVPerBlock.size() * sizeof(uint32_t));
        }
    }
    if (this->cfg.enableEntropy)
    {
        outputResult->averageEntropy = result->entropyY;
        outputResult->entropyDiff     = result->entropyDiff;
        outputResult->entropyEpsilon = result->entropyEpsilon;

        if (outputResult->entropyPerBlock)
            std::memcpy(outputResult->entropyPerBlock,
                        result->entropyPerBlock.data(),
                        result->entropyPerBlock.size() * sizeof(double));
        if (outputResult->entropyDiffPerBlock)
            std::memcpy(outputResult->entropyDiffPerBlock,
                        result->entropyDiffPerBlock.data(),
                        result->entropyDiffPerBlock.size() * sizeof(double));
        if (this->cfg.enableChroma)
        {
            outputResult->entropyU = result->entropyU;
            outputResult->entropyV = result->entropyV;
            if (outputResult->entropyUPerBlock)
                std::memcpy(outputResult->entropyUPerBlock,
                            result->entropyUPerBlock.data(),
                            result->entropyUPerBlock.size() * sizeof(double));
            if (outputResult->entropyVPerBlock)
                std::memcpy(outputResult->entropyVPerBlock,
                            result->entropyVPerBlock.data(),
                            result->entropyVPerBlock.size() * sizeof(double));
        }

    }

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
        if (info.bitDepth != 8 && info.bitDepth != 10 && info.bitDepth != 12)
        {
            log(this->cfg,
                LogLevel::Error,
                "Frame with invalid bit " + std::to_string(info.bitDepth)
                    + " depth provided. Must be 8, 10, or 12.");
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
        log(this->cfg,
            LogLevel::Error,
            "Frame settings differ from the settings that the library was configured with");
        return false;
    }

    return true;
}

} // namespace vca
