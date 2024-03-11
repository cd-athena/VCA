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

#pragma once

#include <analyzer/common/EnumMapper.h>
#include <vcaLib.h>

#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace vca {

#if defined(__GNUC__)
#define ALIGN_VAR_32(T, var) T var __attribute__((aligned(32)))
#elif defined(_MSC_VER)
#define ALIGN_VAR_32(T, var) __declspec(align(32)) T var
#endif

const auto CpuSimdMapper = EnumMapper<CpuSimd>({{CpuSimd::None, "NoSimd"},
                                                {CpuSimd::SSE2, "SSE2"},
                                                {CpuSimd::SSSE3, "SSSE3"},
                                                {CpuSimd::SSE4, "SSE4"},
                                                {CpuSimd::AVX2, "AVX2"}});

inline void log(const vca_param &cfg, LogLevel level, const std::string &message)
{
    static std::mutex loggingMutex;
    std::unique_lock<std::mutex> lock(loggingMutex);
    if (cfg.logFunction)
        cfg.logFunction(cfg.logFunctionPrivateData, level, message.c_str());
}

inline std::pair<unsigned, unsigned> getFrameSizeInBlocks(unsigned blockSize,
                                                          const vca_frame_info &info)
{
    auto widthInBlocks = (info.width + blockSize - 1) / blockSize;
    auto heightInBlock = (info.height + blockSize - 1) / blockSize;
    return {widthInBlocks, heightInBlock};
}

inline std::pair<unsigned, unsigned> getChromaFrameSizeInBlocks(unsigned blockSize,
                                                                int width,
                                                                int height)
{
    auto widthInBlocks = (width + blockSize - 1) / blockSize;
    auto heightInBlock = (height + blockSize - 1) / blockSize;
    return {widthInBlocks, heightInBlock};
}

struct MacroblockRange
{
    unsigned start{};
    unsigned end{};
};

struct Job
{
    vca_frame *frame;
    MacroblockRange macroblockRange;
    unsigned jobID;

    std::string infoString()
    {
        return "Job " + std::to_string(this->jobID) + " POC "
               + std::to_string(this->frame->stats.poc) + " MB "
               + std::to_string(macroblockRange.start) + "-" + std::to_string(macroblockRange.end);
    }
};

struct Result
{
    std::vector<uint32_t> brightnessPerBlock;
    std::vector<uint32_t> energyPerBlock;
    std::vector<uint32_t> energyDiffPerBlock;
    std::vector<int32_t> energyEpsilonPerBlock;
    std::vector<uint32_t> averageUPerBlock;
    std::vector<uint32_t> averageVPerBlock;
    std::vector<uint32_t> energyUPerBlock;
    std::vector<uint32_t> energyVPerBlock;
    uint32_t averageBrightness{};
    uint32_t averageEnergy{};
    uint32_t averageU{};
    uint32_t averageV{};
    uint32_t energyU{};
    uint32_t energyV{};
    double energyDiff{};
    double energyEpsilon{};

    std::vector<double> entropyPerBlock;
    std::vector<double> entropyDiffPerBlock;
    std::vector<double> entropyUPerBlock;
    std::vector<double> entropyVPerBlock;
    double entropyY{};
    double entropyU{};
    double entropyV{};
    double entropyDiff{};
    double entropyEpsilon{};

    std::vector<double> edgeDensityPerBlock;
    double averageEdgeDensity{};

    int poc{};
    unsigned jobID{};
};

} // namespace vca
