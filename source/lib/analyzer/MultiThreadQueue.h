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
};

} // namespace vca
