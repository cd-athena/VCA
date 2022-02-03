/*****************************************************************************
 * Copyright (C) 2022 Christian Doppler Laboratory ATHENA
 *
 * Authors: Steve Borho <steve@borho.org>
 *          Christian Feldmann  <christian.feldmann@bitmovin.com>
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

#include "YUVInput.h"

#ifdef FILESYSTEM_EXPERIMENTAL
#include <experimental/filesystem>
namespace filesystem = std::experimental::filesystem;
#else
#include <filesystem>
namespace filesystem = std::filesystem;
#endif

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

    auto frameSizeBytes = IInputFile::calculateFrameBytesInInput(this->frameInfo);

    {
        auto fileSize    = filesystem::file_size(fileName);
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

bool YUVInput::readFrame(FrameWithData &frame)
{
    if (!this->input.good() || this->input.eof())
        return false;

    this->input.read((char *) (frame.getData()), frame.getFrameSize());

    if (!this->input)
        return false;

    return true;
}

double YUVInput::getFPS() const
{
    return 0.0;
}

} // namespace vca
