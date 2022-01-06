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

#pragma once

#include <common/common.h>
#include <lib/vcaLib.h>

#define MIN_FRAME_WIDTH 64
#define MAX_FRAME_WIDTH 8192
#define MIN_FRAME_HEIGHT 64
#define MAX_FRAME_HEIGHT 4320

#include <fstream>

namespace vca {

class IInputFile
{
protected:
    vca_frame_info frameInfo{};
    unsigned frameCount{};

    std::ifstream input;

public:
    virtual ~IInputFile() {}

    virtual bool readFrame(frameWithData &frame) = 0;

    size_t calcualteFrameBytes() const
    {
        size_t framesizeBytes = 0;
        const auto colorspace = this->frameInfo.colorspace;
        auto pixelbytes       = this->frameInfo.bitDepth > 8 ? 2u : 1u;
        for (int i = 0; i < vca_cli_csps.at(colorspace).planes; i++)
        {
            uint32_t w = this->frameInfo.width >> vca_cli_csps.at(colorspace).width[i];
            uint32_t h = this->frameInfo.height >> vca_cli_csps.at(colorspace).height[i];
            framesizeBytes += w * h * pixelbytes;
        }
        return framesizeBytes;
    }

    void updateFramePointers(frameWithData &frame) const
    {
        const auto colorspace = this->frameInfo.colorspace;
        auto pixelbytes       = this->frameInfo.bitDepth > 8 ? 2u : 1u;

        frame.vcaFrame.info = this->frameInfo;

        frame.vcaFrame.planes[0] = frame.data.data();
        frame.vcaFrame.stride[0] = this->frameInfo.width;

        if (vca_cli_csps.at(colorspace).planes > 1)
        {
            uint32_t widthChroma  = this->frameInfo.width >> vca_cli_csps.at(colorspace).width[1];
            uint32_t heightChroma = this->frameInfo.height >> vca_cli_csps.at(colorspace).height[1];
            auto frameSizeBytes   = widthChroma * heightChroma * pixelbytes;

            frame.vcaFrame.planes[1] = frame.data.data() + frameSizeBytes;
            frame.vcaFrame.planes[2] = frame.data.data() + frameSizeBytes * 2;
            frame.vcaFrame.stride[1] = widthChroma;
            frame.vcaFrame.stride[2] = widthChroma;
        }
    }

    bool isEof() const
    {
        return input.eof();
    }
    bool isFail()
    {
        return input.fail();
    }

    vca_frame_info getFrameInfo() const
    {
        return frameInfo;
    }
};

} // namespace vca
