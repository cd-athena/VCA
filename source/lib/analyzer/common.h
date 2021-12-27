#pragma once

#include "vcaLib.h"

#include <string>
#include <vector>
#include <utility>

namespace vca {

#if defined(__GNUC__)
#define ALIGN_VAR_32(T, var) T var __attribute__((aligned(32)))
#elif defined(_MSC_VER)
#define ALIGN_VAR_32(T, var) __declspec(align(32)) T var
#endif

inline void log(const vca_param &cfg, LogLevel level, const std::string &message)
{
    if (!cfg.logFunction)
        return;
    cfg.logFunction(cfg.logFunctionPrivateData, level, message.c_str());
}

inline std::pair<unsigned, unsigned> getFrameSizeInBlocks(const vca_param &cfg, const vca_frame_info &info)
{
    auto widthInBlocks = (info.width + cfg.blockSize - 1) / cfg.blockSize;
    auto heightInBlock = (info.height + cfg.blockSize - 1) / cfg.blockSize;
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
    std::vector<int32_t> energyPerBlock;
    int32_t averageEnergy{};

    int poc;
};

} // namespace vca
