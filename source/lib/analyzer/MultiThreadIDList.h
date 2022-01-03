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
