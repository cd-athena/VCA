/*****************************************************************************
 * Copyright (C) 2021 Christian Doppler Laboratory ATHENA
 *
 * Authors: Steve Borho <steve@borho.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include "YUVInput.h"

#include <filesystem>
#include <iostream>

namespace vca {

YUVInput::YUVInput(std::string &fileName, vca_frame_info &openFrameInfo, unsigned skipFrames)
{
    frameInfo = openFrameInfo;

    if (frameInfo.width == 0 || frameInfo.height == 0 || frameInfo.bitDepth == 0)
    {
        vca_log(LogLevel::Error,
                "yuv: width, height, and bitDepth must be specified to open a raw YUV file");
        return;
    }

    input.open(fileName, std::ios::binary);
    if (!input.good())
    {
        vca_log(LogLevel::Error, "Error opening file");
        return;
    }

    auto frameSizeBytes = this->calcualteFrameBytes();

    {
        auto fileSize    = std::filesystem::file_size(fileName);
        this->frameCount = unsigned(fileSize / frameSizeBytes);
        vca_log(LogLevel::Info, "Detected " + std::to_string(this->frameCount) + " frames in input");
    }

    if (skipFrames)
    {
        auto filePos = std::streampos(frameSizeBytes * skipFrames);
        vca_log(LogLevel::Info, "Seeking file to pos " + std::to_string(filePos));
        input.seekg(filePos);
    }
}

bool YUVInput::readFrame(frameWithData &frame)
{
    if (!this->input.good() || this->input.eof())
        return false;

    auto frameSizeBytes = this->calcualteFrameBytes();
    if (frame.data.size() < frameSizeBytes)
        frame.data.resize(frameSizeBytes);

    uint32_t pixelbytes      = frameInfo.bitDepth > 8 ? 2 : 1;
    frame.vcaFrame.info      = this->frameInfo;
    frame.vcaFrame.stride[0] = this->frameInfo.width * pixelbytes;
    frame.vcaFrame.stride[1] = frame.vcaFrame.stride[0]
                               >> vca_cli_csps.at(this->frameInfo.colorspace).width[1];
    frame.vcaFrame.stride[2] = frame.vcaFrame.stride[0]
                               >> vca_cli_csps.at(this->frameInfo.colorspace).width[2];
    frame.vcaFrame.planes[0] = frame.vcaFrame.planes[1] = (char *) frame.vcaFrame.planes[0]
                                                          + frame.vcaFrame.stride[0]
                                                                * this->frameInfo.height;
    frame.vcaFrame.planes[2] = (char *) frame.vcaFrame.planes[1]
                               + frame.vcaFrame.stride[1]
                                     * (this->frameInfo.height
                                        >> vca_cli_csps.at(this->frameInfo.colorspace).height[1]);

    return true;
}

} // namespace vca
