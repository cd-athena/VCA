#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <optional>

namespace vca {

template<class T>
class MultiThreadQueue
{
public:

    void abort();

    void push(T item);
    std::optional<T> pop();
    
    bool empty();

private:
    std::queue<T> items;
    std::mutex accessMutex;
    std::condition_variable accessCV;

    bool aborted{};
};

} // namespace vca
