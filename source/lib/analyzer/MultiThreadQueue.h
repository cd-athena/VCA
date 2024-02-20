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

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace vca {

template<class T>
class MultiThreadQueue
{
public:
    // Push an item to the queue. Wait if the queue reached a maximum size.
    // Wake one waiting thread.
    void waitAndPush(T item);
    // Push items in order. Increase the counter by one in the order of items.
    // Pushing threads will be paused until the pushs are in order.
    // Don't mix calls to these two push functions.
    void waitAndPushInOrder(T item, size_t counter);

    // Get an item. If the queue is empty, wait until an item is pushed.
    // Will return empty opt if abort is called.
    std::optional<T> waitAndPop();

    void abort();
    bool empty();

    // If the queue is fuller then this limit, the push function will wait until
    // there is enought space. 0 means no limit.
    void setMaximumQueueSize(size_t max);

private:
    std::queue<T> items;
    std::mutex accessMutex;
    std::condition_variable pushJobCV;
    std::condition_variable popJobCV;

    bool aborted{};
    size_t maximumQueueSize{};
    size_t pushCounter{};
};

} // namespace vca
