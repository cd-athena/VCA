#include "MultiThreadQueue.h"

#include "common.h"

namespace vca {

template<class T>
void MultiThreadQueue<T>::abort()
{
    this->aborted = true;
    this->accessCV.notify_all();
}

template<class T>
void MultiThreadQueue<T>::push(T item)
{
    if (this->aborted)
        return;

    std::unique_lock<std::mutex> lock(this->accessMutex);
    this->items.push(std::move(item));
    this->accessCV.notify_one();
}

template<class T>
std::optional<T> MultiThreadQueue<T>::waitAndPop()
{
    std::unique_lock<std::mutex> lock(this->accessMutex);
    this->accessCV.wait(lock, [this]() { return !this->items.empty() || this->aborted; });

    if (this->aborted)
        return {};

    auto item = this->items.front();
    this->items.pop();
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

template class MultiThreadQueue<Job>;
template class MultiThreadQueue<Result>;

} // namespace vca
