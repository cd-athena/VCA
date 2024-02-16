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
#include <analyzer/common/common.h>
#include <vcaLib.h>

#include <thread>

namespace vca {

class ProcessingThread
{
public:
    ProcessingThread()                     = delete;
    ProcessingThread(ProcessingThread &&o) = delete;
    ProcessingThread(vca_param cfg,
                     MultiThreadQueue<Job> &jobs,
                     MultiThreadQueue<Result> &results,
                     unsigned id);
    ~ProcessingThread() = default;

    void abort();
    void join();

private:
    void threadFunction(MultiThreadQueue<Job> &jobQueue, MultiThreadQueue<Result> &results);

    std::thread thread;
    bool aborted{};
    unsigned id{};
    vca_param cfg;
};

} // namespace vca
