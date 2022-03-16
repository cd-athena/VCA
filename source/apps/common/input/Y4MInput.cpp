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

#include "Y4MInput.h"

#ifdef FILESYSTEM_EXPERIMENTAL
#include <experimental/filesystem>
namespace filesystem = std::experimental::filesystem;
#else
#include <filesystem>
namespace filesystem = std::filesystem;
#endif

#include <iterator>
#include <string>

namespace vca {

Y4MInput::Y4MInput(std::string &fileName)
{
    if (!this->openInput(fileName))
    {
        vca_log(LogLevel::Error, "Error opening input " + fileName);
        return;
    }

    if (!this->parseHeader())
    {
        vca_log(LogLevel::Error, "Error parsing Y4M header");
        return;
    }

    {
        const auto assumedHeaderSize = 6u;
        auto estFrameSize = calculateFrameBytesInInput(this->frameInfo) + assumedHeaderSize;
        auto fileSize     = filesystem::file_size(fileName);
        this->frameCount  = unsigned(fileSize / estFrameSize);
        vca_log(LogLevel::Info, "Detected " + std::to_string(this->frameCount) + " frames in input");
    }
}

bool Y4MInput::parseHeader()
{
    auto it = std::istreambuf_iterator<char>(*this->input);

    auto getNextHeaderField = [&it]() {
        if (*it == '\n')
            return std::string();
        if (*it == ' ')
            it++;
        std::string str;
        while (it != std::istreambuf_iterator<char>() && *it != ' ' && *it != '\n')
            str.push_back(*it++);
        return str;
    };

    if (getNextHeaderField() != "YUV4MPEG2")
    {
        vca_log(LogLevel::Error, "Y4M file must start with YUV4MPEG2");
        return false;
    }

    while (true)
    {
        auto field = getNextHeaderField();
        if (field.empty())
            break;

        auto parameterIndicator = field[0];
        if (parameterIndicator == 'W')
        {
            auto val = std::stoul(field.substr(1));
            if (val < MIN_FRAME_WIDTH || val > MAX_FRAME_WIDTH)
            {
                vca_log(LogLevel::Error, "Invalid width value: " + field);
                return false;
            }
            this->frameInfo.width = val;
            vca_log(LogLevel::Info, "Y4M read hidth " + std::to_string(val));
        }
        else if (parameterIndicator == 'H')
        {
            auto val = std::stoul(field.substr(1));
            if (val < MIN_FRAME_HEIGHT || val > MAX_FRAME_HEIGHT)
            {
                vca_log(LogLevel::Error, "Invalid height value: " + field);
                return false;
            }
            this->frameInfo.height = val;
            vca_log(LogLevel::Info, "Y4M read height " + std::to_string(val));
        }
        else if (parameterIndicator == 'C')
        {
            auto indicator = field.substr(1, 3);
            if (indicator == "420")
            {
                this->frameInfo.colorspace = vca_colorSpace::YUV420;
                vca_log(LogLevel::Info, "Y4M Detected colorspace 4:2:0");
            }
            else if (indicator == "422")
            {
                this->frameInfo.colorspace = vca_colorSpace::YUV422;
                vca_log(LogLevel::Info, "Y4M Detected colorspace 4:2:2");
            }
            else if (indicator == "444")
            {
                this->frameInfo.colorspace = vca_colorSpace::YUV444;
                vca_log(LogLevel::Info, "Y4M Detected colorspace 4:4:4");
            }
            else
                vca_log(LogLevel::Info,
                        "Y4M invalid colorspace indicator (" + indicator + "). Assuming 4:2:0.");
        }
        else if (parameterIndicator == 'F')
        {
            auto colonPos = field.find(':');
            if (colonPos == std::string::npos)
            {
                vca_log(LogLevel::Error, "Invalid fps value: " + field);
                return false;
            }

            try
            {
                auto num  = std::stoi(field.substr(1, colonPos - 1));
                auto den  = std::stoi(field.substr(colonPos + 1));
                this->fps = double(num) / double(den);
                vca_log(LogLevel::Info,
                        "Y4M Detected fps " + std::to_string(num) + "/" + std::to_string(den));
            }
            catch (...)
            {
                vca_log(LogLevel::Error, "Invalid fps value: " + field);
                return false;
            }
        }
    }

    return true;
}

bool Y4MInput::readFrame(FrameWithData &frame)
{
    char c = 0;
    while (this->input->get(c) && c != 'F')
    {}

    if (this->input->eof())
        return false;

    if (!this->input->good())
        throw std::runtime_error("Error reading from file");

    auto getNextChar = [this]() {
        char c;
        if (!this->input->get(c))
            return char(0);
        return c;
    };

    if (getNextChar() != 'R' || getNextChar() != 'A' || getNextChar() != 'M' || getNextChar() != 'E')
        throw std::runtime_error("Error reading FRAME tag");

    while (this->input->get(c) && c != '\n')
    {}

    this->input->read((char *) (frame.getData()), frame.getFrameSize());

    return true;
}

double Y4MInput::getFPS() const
{
    return this->fps;
}

} // namespace vca
