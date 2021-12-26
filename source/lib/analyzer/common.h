#pragma once

#include "vcaLib.h"

#include <string>

namespace vca {

inline void log(const vca_param &cfg, LogLevel level, const std::string &message)
{
    if (!cfg.logFunction)
        return;
    cfg.logFunction(cfg.logFunctionPrivateData, level, message.c_str());
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

} // namespace vca
