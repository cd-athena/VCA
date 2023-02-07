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

#pragma once

#include <common/EnumMapper.h>
#include <lib/vcaLib.h>

#include <string>
#include <vector>

#pragma once

namespace vca {

class FrameWithData
{
public:
    FrameWithData() = delete;
    FrameWithData(const vca_frame_info &frameInfo);
    ~FrameWithData() = default;

    uint8_t *getData() const
    {
        return (uint8_t *) (this->data.data());
    }
    size_t getFrameSize() const
    {
        return this->data.size();
    }
    vca_frame *getFrame()
    {
        return &this->vcaFrame;
    }

private:
    std::vector<uint8_t> data;
    vca_frame vcaFrame;
};

const auto vca_colorSpaceMapper = EnumMapper<vca_colorSpace>({{vca_colorSpace::YUV400, "4:0:0"},
                                                              {vca_colorSpace::YUV420, "4:2:0"},
                                                              {vca_colorSpace::YUV422, "4:2:2"},
                                                              {vca_colorSpace::YUV444, "4:4:4"}});

void vca_log(LogLevel level, std::string error);
size_t calculateFrameBytesInInput(const vca_frame_info &frameInfo);

} // namespace vca
