/*****************************************************************************
 * Copyright (C) 2022 Christian Doppler Laboratory ATHENA
 *
 * Authors: Steve Borho <steve@borho.org>
 *          Christian Feldmann <christian.feldmann@bitmovin.com>
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

    virtual bool readFrame(FrameWithData &frame) = 0;

    static size_t calculateFrameBytesInInput(const vca_frame_info &frameInfo)
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
