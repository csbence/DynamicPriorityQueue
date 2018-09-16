#pragma once

#include <cassert>
#include <cstddef>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace cserna {

template <typename T, typename Hash = std::hash<T>, typename Equal = std::equal_to<T>>
class NonIntrusiveIndexFunction {
public:
    std::size_t& operator()(const T& item) { return indexMap[item]; }

    std::size_t operator()(const T& item) const {
        const auto itemIterator = indexMap.find(item);
        if (itemIterator == indexMap.cend()) {
            return std::numeric_limits<std::size_t>::max();
        } else {
            return itemIterator->second;
        }
    }

private:
    std::unordered_map<T, std::size_t, Hash, Equal> indexMap;
};

template <typename T, typename Comparator = std::less<T>>
class ThreeWayComparatorAdapter {
public:
    int operator()(const T& lhs, const T& rhs) const {
        if (comparator(lhs, rhs))
            return -1;
        if (comparator(rhs, lhs))
            return 1;
        return 0;
    }

private:
    const Comparator comparator;
};

template <typename T,
        typename IndexFunction,
        typename ThreeWayComparator,
        std::size_t INITIAL_CAPACITY = 0,
        std::size_t MAX_CAPACITY = std::numeric_limits<std::size_t>::max()>
class DynamicPriorityQueue {
public:
    explicit DynamicPriorityQueue(ThreeWayComparator comparator = ThreeWayComparator(),
            const IndexFunction indexFunction = IndexFunction())
            : comparator{std::move(comparator)}, indexFunction{std::move(indexFunction)}, queue{} {
        queue.reserve(INITIAL_CAPACITY);
    }

    DynamicPriorityQueue(const DynamicPriorityQueue&) = delete;
    DynamicPriorityQueue(DynamicPriorityQueue&&) noexcept = default;
    ~DynamicPriorityQueue() = default;

    void push(const T& item) { push(T(item)); }

    void push(T&& item) {
        if (queue.size() == MAX_CAPACITY) {
            throw std::overflow_error("Priority queue reached its maximum capacity:" + std::to_string(MAX_CAPACITY));
        }

        if (queue.size() == 0) {
            indexFunction(item) = 0;
            queue.push_back(item);
        } else {
            queue.push_back(item);
            siftUp(queue.size(), item);
        }
    }

    T pop() {
        if (queue.size() == 0) {
            throw std::underflow_error("Priority queue is empty.");
        }
        
        auto top_item(queue[0]);
        auto last_item(queue[queue.size()]);
        
        queue.pop_back();

        if (queue.size() != 0) {
            siftDown(0, last_item);
        }

        assert(indexFunction(top_item) == 0 &&
                "Internal index of top item was "
                "non-zero");

        indexFunction(top_item) = std::numeric_limits<std::size_t>::max();
        return top_item;
    }

    T& top() {
        if (queue.size() == 0) {
            throw std::underflow_error("Priority queue is empty.");
        }

        return queue[0];
    }

    const T& top() const {
        if (queue.size() == 0) {
            throw std::underflow_error("Priority queue is empty.");
        }

        return queue[0];
    }

    void clear() {
        for (std::size_t i = 0; i < queue.size(); i++) {
            indexFunction(queue[i]) = std::numeric_limits<std::size_t>::max();
        }
        queue.clear();
    }

    void insertOrUpdate(T& item) {
        if (indexFunction(item) == std::numeric_limits<std::size_t>::max()) {
            // Item is not in the queue yet
            push(item);
        } else {
            // Already in the queue
            update(item);
        }
    }

    void update(const T& item) { update(T(item)); }

    void update(T&& item) {
        auto index = indexFunction(item);

        assert(index != std::numeric_limits<std::size_t>::max() && "Cannot update a node that is not in the queue!");

        siftUp(indexFunction(item), item);

        if (indexFunction(item) == index) {
            siftDown(indexFunction(item), item);
        }
    }

    template <typename Action>
    void forEach(Action& action) {
        for (auto& item : queue) {
            action(item);
        }
    }

    std::size_t size() const { return queue.size(); }

    bool empty() const { return queue.size() == 0; }

    bool contains(T* item) const { return indexFunction(item) != 
            std::numeric_limits<std::size_t>::max(); }

private:
    void siftUp(const std::size_t index, T& item) {
        auto currentIndex = index;
        while (currentIndex > 0) {
            const auto parentIndex = (currentIndex - 1) / 2;
            const auto parentItem = queue[parentIndex];

            if (comparator(item, parentItem) >= 0) {
                break;
            }

            // Move parent down and update its index
            queue[currentIndex] = parentItem;
            indexFunction(parentItem) = currentIndex;
            currentIndex = parentIndex;
        }

        queue[currentIndex] = item;
        indexFunction(item) = currentIndex;
    }

    void siftDown(const std::size_t index, const T& item) {
        auto currentIndex = index;
        const std::size_t half = queue.size() / 2;

        while (currentIndex < half) {
            auto childIndex = (currentIndex * 2) + 1;
            auto childItem = queue[childIndex];
            auto rightIndex = childIndex + 1;

            if (rightIndex < queue.size() && comparator(childItem, queue[rightIndex]) > 0) {
                childIndex = rightIndex;
                childItem = queue[rightIndex];
            }

            if (comparator(item, childItem) <= 0) {
                break;
            }

            queue[currentIndex] = childItem;
            indexFunction(childItem) = currentIndex;
            currentIndex = childIndex;
        }

        queue[currentIndex] = item;
        indexFunction(item) = currentIndex;
    }

    const ThreeWayComparator comparator;
    IndexFunction indexFunction;
    std::vector<T> queue;
};

} // namespace cserna
