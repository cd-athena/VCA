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
 
#include "MultiThreadIDList.h"
#include "common.h"

namespace vca {

template<class T>
void MultiThreadIDList<T>::abort()
{
    this->aborted = true;
    this->accessCV.notify_all();
}

template<class T>
void MultiThreadIDList<T>::push(unsigned id, T item)
{
    if (this->aborted)
        return;

    std::unique_lock<std::mutex> lock(this->accessMutex);
    this->items.push_back({id, std::move(item)});
    this->accessCV.notify_all();
}

template<class T>
std::optional<T> MultiThreadIDList<T>::waitAndPop(unsigned id)
{
    std::unique_lock<std::mutex> lock(this->accessMutex);

    auto abortedOrItemFound = [this, id]() {
        if (this->aborted)
            return true;
        for (const auto &item : this->items)
            if (item.first == id)
                return true;
        return false;
    };

    this->accessCV.wait(lock, abortedOrItemFound);

    if (this->aborted)
        return {};

    for (auto it = this->items.begin(); it != this->items.end(); it++)
    {
        if (it->first == id)
        {
            auto item = it->second;
            this->items.erase(it);
            return item;
        }
    }

    throw std::out_of_range("Error retrieving item with ID " + std::to_string(id));
}

template<class T>
bool MultiThreadIDList<T>::empty()
{
    if (this->aborted)
        return false;

    std::unique_lock<std::mutex> lock(this->accessMutex);
    return this->items.empty();
}

template class MultiThreadIDList<Result>;

} // namespace vca
