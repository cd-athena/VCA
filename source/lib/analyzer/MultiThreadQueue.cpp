#include "MultiThreadQueue.h"

#include "common.h"

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
