/* Copyright (C) 2024 Christian Doppler Laboratory ATHENA
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

#include <analyzer/MultiThreadQueue.h>
#include <analyzer/ProcessingThread.h>
#include <analyzer/common/common.h>
#include <vcaLib.h>

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace vca {

class Analyzer
{
public:
    Analyzer(vca_param cfg);
    ~Analyzer();

    vca_result pushFrame(vca_frame *frame);
    bool resultAvailable();
    vca_result pullResult(vca_frame_results *result);

private:
    vca_param cfg{};
    bool checkFrame(const vca_frame *frame);
    std::optional<vca_frame_info> frameInfo;
    unsigned frameCounter{0};

    std::vector<std::unique_ptr<ProcessingThread>> threadPool;

    MultiThreadQueue<Job> jobs;
    MultiThreadQueue<Result> results;

    std::optional<Result> previousResult;
};

} // namespace vca
