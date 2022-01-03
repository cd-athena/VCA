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
    // Push an item to the queue. Wake one waiting thread.
    void push(T item);

    // Get an item. If the queue is empty, wait until an item is pushed.
    // Will return empty opt if abort is called.
    std::optional<T> waitAndPop();

    void abort();

    bool empty();

private:
    std::queue<T> items;
    std::mutex accessMutex;
    std::condition_variable accessCV;

    bool aborted{};
};

} // namespace vca
