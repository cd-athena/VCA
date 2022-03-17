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

    std::istream *input{};
    std::ifstream inputFile;

public:
    virtual ~IInputFile() {}

    virtual bool readFrame(FrameWithData &frame) = 0;

    bool isEof() const;
    bool isFail() const;

    vca_frame_info getFrameInfo() const;
    virtual double getFPS() const = 0;

    bool openInput(std::string &fileName);
    bool isStdin() const;
};

} // namespace vca
