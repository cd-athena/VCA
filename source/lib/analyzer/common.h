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

#pragma once

#include "vcaLib.h"

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
    std::vector<uint32_t> sadPerBlock;
    uint32_t averageBrightness{};
    uint32_t averageEnergy{};
    double sad{};
    double epsilon{};
    int poc{};
    unsigned jobID{};
};

} // namespace vca
