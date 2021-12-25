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

void vca_log(LogLevel level, std::string error);

} // namespace vca