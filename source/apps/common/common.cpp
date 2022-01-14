
#include "common.h"

#include <iostream>
#include <map>

namespace vca {

FrameWithData::FrameWithData(const vca_frame_info &frameInfo)
{
    this->vcaFrame.info = frameInfo;

    const auto colorspace = frameInfo.colorspace;
    auto pixelbytes       = frameInfo.bitDepth > 8 ? 2u : 1u;

    size_t planeSizeBytes[3] = {0, 0, 0};
    for (int i = 0; i < vca_cli_csps.at(colorspace).planes; i++)
    {
        uint32_t w        = frameInfo.width >> vca_cli_csps.at(colorspace).width[i];
        uint32_t h        = frameInfo.height >> vca_cli_csps.at(colorspace).height[i];
        planeSizeBytes[i] = w * h * pixelbytes;
    }

    const auto frameSizeBytes = planeSizeBytes[0] + planeSizeBytes[1] + planeSizeBytes[2];
    if (this->data.size() < frameSizeBytes)
        this->data.resize(frameSizeBytes);

    this->vcaFrame.planes[0] = this->data.data();
    this->vcaFrame.stride[0] = frameInfo.width;

    if (vca_cli_csps.at(colorspace).planes > 1)
    {
        uint32_t widthChroma  = frameInfo.width >> vca_cli_csps.at(colorspace).width[1];
        uint32_t heightChroma = frameInfo.height >> vca_cli_csps.at(colorspace).height[1];

        this->vcaFrame.planes[1] = this->data.data() + planeSizeBytes[0];
        this->vcaFrame.planes[2] = this->data.data() + planeSizeBytes[0] + planeSizeBytes[1];
        this->vcaFrame.stride[1] = widthChroma;
        this->vcaFrame.stride[2] = widthChroma;
    }
}

void vca_log(LogLevel level, std::string error)
{
    static LogLevel appLogLevel     = level;
    static const auto logLevelToInt = std::map<LogLevel, unsigned>(
        {{LogLevel::Debug, 0}, {LogLevel::Info, 1}, {LogLevel::Warning, 2}, {LogLevel::Error, 3}});
    static const auto logLevelName = std::map<LogLevel, std::string>(
        {{LogLevel::Debug, "[Debug] "},
         {LogLevel::Info, "[Info] "},
         {LogLevel::Warning, "[Warning] "},
         {LogLevel::Error, "[Error] "}});

    if (logLevelToInt.at(level) >= logLevelToInt.at(appLogLevel))
        std::cout << logLevelName.at(level) << error << std::endl;
}

} // namespace vca
