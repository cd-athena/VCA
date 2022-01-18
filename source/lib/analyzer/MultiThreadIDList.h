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

#pragma once

#include <condition_variable>
#include <list>
#include <mutex>
#include <optional>

namespace vca {

template<class T>
class MultiThreadIDList
{
public:
    // Add item to the list. Wake the corresponding waiting thread.
    void push(unsigned id, T item);

    // Get the item with the given ID. If not found, wait until the item is pushed.
    // Will return empty opt if abort is called.
    std::optional<T> waitAndPop(unsigned id);

    void abort();
    bool empty();

private:
    std::list<std::pair<unsigned, T>> items;
    std::mutex accessMutex;
    std::condition_variable accessCV;

    bool aborted{};
};

} // namespace vca
