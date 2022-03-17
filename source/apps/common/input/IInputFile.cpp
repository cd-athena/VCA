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

#include "IInputFile.h"

#include <iostream>

#if _WIN32
#include <fcntl.h>
#include <io.h>

#if defined(_MSC_VER)
#pragma warning(disable : 4996) // POSIX setmode and fileno deprecated
#endif
#endif

namespace vca {

bool IInputFile::isEof() const
{
    return !this->input || this->input->eof();
}

bool IInputFile::isFail() const
{
    return !this->input || this->input->fail();
}

vca_frame_info IInputFile::getFrameInfo() const
{
    return this->frameInfo;
}

bool IInputFile::openInput(std::string &fileName)
{
    if (fileName == "stdin" || fileName == "stdin:y4m")
    {
        vca_log(LogLevel::Info, "Opening input from pipe");
        this->input = &std::cin;
#if _WIN32
        setmode(fileno(stdin), O_BINARY);
#endif
    }
    else
    {
        vca_log(LogLevel::Info, "Opening input from file " + fileName);
        this->inputFile.open(fileName, std::ios::binary);
        this->input = &this->inputFile;
    }

    return this->input && this->input->good();
}

bool IInputFile::isStdin() const
{
    return this->input == &std::cin;
}

} // namespace vca