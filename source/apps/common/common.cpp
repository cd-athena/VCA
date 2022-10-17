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
    this->vcaFrame.stride[0] = frameInfo.width * pixelbytes;
    this->vcaFrame.height[0] = frameInfo.height;

    if (vca_cli_csps.at(colorspace).planes > 1)
    {
        uint32_t widthChroma  = frameInfo.width >> vca_cli_csps.at(colorspace).width[1];
        uint32_t heightChroma = frameInfo.height >> vca_cli_csps.at(colorspace).height[1];

        this->vcaFrame.planes[1] = this->data.data() + planeSizeBytes[0];
        this->vcaFrame.planes[2] = this->data.data() + planeSizeBytes[0] + planeSizeBytes[1];
        this->vcaFrame.stride[1] = widthChroma * pixelbytes;
        this->vcaFrame.stride[2] = widthChroma * pixelbytes;
        this->vcaFrame.height[1] = heightChroma;
        this->vcaFrame.height[2] = heightChroma;
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

size_t calculateFrameBytesInInput(const vca_frame_info &frameInfo)
{
    size_t framesizeBytes = 0;
    const auto colorspace = frameInfo.colorspace;
    auto pixelbytes       = frameInfo.bitDepth > 8 ? 2u : 1u;
    for (int i = 0; i < vca_cli_csps.at(colorspace).planes; i++)
    {
        uint32_t w = frameInfo.width >> vca_cli_csps.at(colorspace).width[i];
        uint32_t h = frameInfo.height >> vca_cli_csps.at(colorspace).height[i];
        framesizeBytes += w * h * pixelbytes;
    }
    return framesizeBytes;
}

} // namespace vca
