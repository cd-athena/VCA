#pragma once

#include <lib/vcaLib.h>

#include <string>
#include <vector>

#pragma once

namespace vca {

struct frameWithData
{
    std::vector<uint8_t> data;

    vca_frame vcaFrame;
};

enum class LogLevel
{
    Error,
    Warning,
    Info,
    Debug
};

void vca_log(LogLevel level, std::string error);

} // namespace vca