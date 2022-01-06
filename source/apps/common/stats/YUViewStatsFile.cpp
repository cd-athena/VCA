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

    this->file << "%;type;0;BlockEnergy;range\n"s;
    this->file << "%;defaultRange;0;10000;heat\n"s;
}

void YUViewStatsFile::write(const vca_frame_results &results, unsigned blockSize)
{
    if (results.energyPerBlock == nullptr)
        return;

    auto widthInBlocks = (info.width + blockSize - 1) / blockSize;
    auto heightInBlock = (info.height + blockSize - 1) / blockSize;

    auto data = results.energyPerBlock;
    for (unsigned y = 0; y < heightInBlock; y++)
        for (unsigned x = 0; x < widthInBlocks; x++)
            this->file << results.poc << ";" << x * blockSize << ";" << y * blockSize << ";"
                       << blockSize << ";" << blockSize << ";0;" << *(data++) << "\n";
}

} // namespace vca