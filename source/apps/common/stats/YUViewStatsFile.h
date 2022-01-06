#pragma once

#include <lib/vcaLib.h>

#include <fstream>
#include <iostream>
#include <string>

namespace vca {

class YUViewStatsFile
{
public:
    YUViewStatsFile(const std::string &filename,
                    const std::string &inputFilename,
                    const vca_frame_info &info);
    ~YUViewStatsFile() = default;

    void write(const vca_frame_results &results, unsigned blockSize);

private:
    std::ofstream file;

    vca_frame_info info;
};

} // namespace vca