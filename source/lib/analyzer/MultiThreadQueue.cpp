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

#include "MultiThreadQueue.h"

#include <analyzer/common/common.h>

namespace vca {

template<class T>
void MultiThreadQueue<T>::abort()
{
    this->aborted = true;
    this->pushJobCV.notify_all();
    this->popJobCV.notify_all();
}

template<class T>
void MultiThreadQueue<T>::waitAndPush(T item)
{
    if (this->aborted)
        return;

    std::unique_lock<std::mutex> lock(this->accessMutex);
    this->popJobCV.wait(lock, [this]() {
        return this->maximumQueueSize == 0 || this->items.size() < this->maximumQueueSize
               || this->aborted;
    });

    if (this->aborted)
        return;

    this->items.push(std::move(item));
    this->pushJobCV.notify_one();
}

template<class T>
void MultiThreadQueue<T>::waitAndPushInOrder(T item, size_t orderCounter)
{
    if (this->aborted)
        return;

    std::unique_lock<std::mutex> lock(this->accessMutex);
    this->popJobCV.wait(lock, [this, orderCounter]() {
        if (this->aborted)
            return true;
        auto slotFree = this->maximumQueueSize == 0 || this->items.size() < this->maximumQueueSize;
        return slotFree && orderCounter == this->pushCounter;
    });

    if (this->aborted)
        return;

    this->items.push(std::move(item));
    this->pushCounter++;
    this->popJobCV.notify_all();
    this->pushJobCV.notify_one();
}

template<class T>
std::optional<T> MultiThreadQueue<T>::waitAndPop()
{
    std::unique_lock<std::mutex> lock(this->accessMutex);
    this->pushJobCV.wait(lock, [this]() { return !this->items.empty() || this->aborted; });

    if (this->aborted)
        return {};

    auto item = this->items.front();
    this->items.pop();
    this->popJobCV.notify_one();
    return item;
}

template<class T>
bool MultiThreadQueue<T>::empty()
{
    if (this->aborted)
        return false;

    std::unique_lock<std::mutex> lock(this->accessMutex);
    return this->items.empty();
}

template<class T>
void MultiThreadQueue<T>::setMaximumQueueSize(size_t max)
{
    this->maximumQueueSize = max;
}

template class MultiThreadQueue<Job>;
template class MultiThreadQueue<Result>;

} // namespace vca
