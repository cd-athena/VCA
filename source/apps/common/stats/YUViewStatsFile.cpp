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

#include "YUViewStatsFile.h"

#include "common/common.h"

namespace vca {

using namespace std::string_literals;

YUViewStatsFile::YUViewStatsFile(const std::string &filename,
                                 const std::string &inputFilename,
                                 const vca_frame_info &info)
{
    this->info = info;
    this->file.open(filename);

    vca_log(LogLevel::Info, "Opened YUView csv file " + filename);

    this->file << "%;syntax-version;v1.22\n"s;
    this->file << "%;%;Written by VCA for YUView\n"s;
    this->file
        << "%;%;POC;X-position of the left top pixel in the block;Y-position of the left top pixel in the block;Width of the block;Height of the block; Type-ID;Type specific value\n"s;
    this->file << "%;seq-specs;"s << inputFilename << ";layer0;"s << info.width << ";"s
               << info.height << ";24\n"s;

    this->file << "%;type;0;BlockBrightness;range\n"s;
    this->file << "%;defaultRange;0;300;heat\n"s;
    this->file << "%;type;1;BlockEnergy;range\n"s;
    this->file << "%;defaultRange;0;10000;heat\n"s;
    this->file << "%;type;2;SAD;range\n"s;
    this->file << "%;defaultRange;0;3000;heat\n"s;
}

void YUViewStatsFile::write(const vca_frame_results &results,
                            unsigned blockSize,
                            bool enableDCTenergy,
                            bool enableEntropy)
{
    auto widthInBlocks = (info.width + blockSize - 1) / blockSize;
    auto heightInBlock = (info.height + blockSize - 1) / blockSize;

    if (enableDCTenergy)
    {
        if (auto data = results.brightnessPerBlock)
        {
            for (unsigned y = 0; y < heightInBlock; y++)
                for (unsigned x = 0; x < widthInBlocks; x++)
                    this->file << results.poc << ";" << x * blockSize << ";" << y * blockSize << ";"
                               << blockSize << ";" << blockSize << ";0;" << *(data++) << "\n";
        }
        if (auto data = results.energyPerBlock)
        {
            for (unsigned y = 0; y < heightInBlock; y++)
                for (unsigned x = 0; x < widthInBlocks; x++)
                    this->file << results.poc << ";" << x * blockSize << ";" << y * blockSize << ";"
                               << blockSize << ";" << blockSize << ";1;" << *(data++) << "\n";
        }
        if (auto data = results.energyDiffPerBlock)
        {
            for (unsigned y = 0; y < heightInBlock; y++)
                for (unsigned x = 0; x < widthInBlocks; x++)
                    this->file << results.poc << ";" << x * blockSize << ";" << y * blockSize << ";"
                               << blockSize << ";" << blockSize << ";2;" << *(data++) << "\n";
        }
    }
    if (enableEntropy)
    {
        if (auto data = results.entropyPerBlock)
        {
            for (unsigned y = 0; y < heightInBlock; y++)
                for (unsigned x = 0; x < widthInBlocks; x++)
                    this->file << results.poc << ";" << x * blockSize << ";" << y * blockSize << ";"
                               << blockSize << ";" << blockSize << ";1;" << *(data++) << "\n";
        }
        if (auto data = results.entropyDiffPerBlock)
        {
            for (unsigned y = 0; y < heightInBlock; y++)
                for (unsigned x = 0; x < widthInBlocks; x++)
                    this->file << results.poc << ";" << x * blockSize << ";" << y * blockSize << ";"
                               << blockSize << ";" << blockSize << ";2;" << *(data++) << "\n";
        }
    }
}

} // namespace vca