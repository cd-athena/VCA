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

template class MultiThreadIDList<EnergyResult>;

} // namespace vca
