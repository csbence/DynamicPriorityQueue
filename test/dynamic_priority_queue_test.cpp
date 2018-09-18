#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "../include/dynamic_priority_queue.hpp"

namespace cserna {
namespace {

struct TestItem {
    explicit TestItem(int value) : value(value), index(std::numeric_limits<std::size_t>::max()) {}

    int value;
    mutable std::size_t index;
};

struct IndexFunction {
    std::size_t& operator()(TestItem* testNode) { return testNode->index; }
    std::size_t operator()(const TestItem* testNode) const { return testNode->index; }
};

struct ItemCompare {
    int operator()(const TestItem* lhs, const TestItem* rhs) const {
        if (lhs->value < rhs->value)
            return -1;
        if (lhs->value > rhs->value)
            return 1;
        return 0;
    }
};

struct NodeCompareRef {
    int operator()(const TestItem& lhs, const TestItem& rhs) const {
        if (lhs.value < rhs.value)
            return -1;
        if (lhs.value > rhs.value)
            return 1;
        return 0;
    }
};

struct NodeHash {
    std::size_t operator()(const TestItem& node) const { return static_cast<size_t>(node.value); }
};

struct NodeEqual {
    bool operator()(const TestItem& lhs, const TestItem& rhs) const { return lhs.value == rhs.value; }
};

TEST_CASE("IndexFunctionTest", "[DynamicPriorityQueue]") {
    // Index function example

    IndexFunction indexFunction;
    TestItem testNode(1);
    testNode.index = 1;
    ItemCompare nodeCompare;
    nodeCompare(&testNode, &testNode);

    auto& index = indexFunction(&testNode);

    REQUIRE(index == 1);

    index = 2;

    REQUIRE(testNode.index == 2);

    indexFunction(&testNode) = 3;

    REQUIRE(testNode.index == 3);
}

TEST_CASE("DynamicPriorityQueue add/clear tests", "[DynamicPriorityQueue]") {
    DynamicPriorityQueue<TestItem*, IndexFunction, ItemCompare, 100, 100> queue;

    SECTION("Add items to queue") {
        auto node1 = TestItem(1);
        auto node2 = TestItem(2);

        REQUIRE(queue.empty());
        REQUIRE(queue.empty());

        queue.push(&node1);

        REQUIRE(queue.size() == 1);
        REQUIRE(node1.index == 0);
        REQUIRE(queue.top() == &node1);
        REQUIRE(queue.top() != &node2);
        REQUIRE(!queue.empty());

        queue.push(&node2);

        REQUIRE(queue.size() == 2);
    }

    SECTION("Clear queue") {
        queue.clear();
        REQUIRE(queue.empty());
    }
}

TEST_CASE("DynamicPriorityQueue remove test", "[DynamicPriorityQueue]") {
    DynamicPriorityQueue<TestItem*, IndexFunction, ItemCompare, 100, 100> queue;

    auto node0 = TestItem(0);
    auto node1 = TestItem(1);
    auto node2 = TestItem(2);

    REQUIRE(queue.empty());

    queue.push(&node1);
    queue.push(&node2);

    REQUIRE(node1.index == 0);
    REQUIRE(node2.index == 1);

    queue.push(&node0);

    REQUIRE(queue.size() == 3);
    REQUIRE(node0.index != std::numeric_limits<std::size_t>::max());

    // Remove last element
    queue.remove(&node1);

    REQUIRE(queue.size() == 2);
    REQUIRE(node0.index == 0);
    REQUIRE(node2.index == 1);
    REQUIRE(node1.index == std::numeric_limits<std::size_t>::max());

    // Remove first element

    queue.remove(&node0);

    REQUIRE(queue.size() == 1);
    REQUIRE(node2.index == 0);
    REQUIRE(node0.index == std::numeric_limits<std::size_t>::max());
    REQUIRE(node1.index == std::numeric_limits<std::size_t>::max());
}

TEST_CASE("DynamicPriorityQueue order test", "[DynamicPriorityQueue]") {
    DynamicPriorityQueue<TestItem*, IndexFunction, ItemCompare, 100, 100> queue;

    SECTION("Add items to queue") {
        auto node0 = TestItem(0);
        auto node1 = TestItem(1);
        auto node2 = TestItem(2);

        REQUIRE(queue.empty());

        queue.push(&node1);
        REQUIRE(queue.size() == 1);
        REQUIRE(node1.index == 0);

        queue.push(&node2);
        REQUIRE(queue.size() == 2);
        REQUIRE(node1.index == 0);
        REQUIRE(node2.index == 1);
        REQUIRE(node0.index == std::numeric_limits<std::size_t>::max());

        queue.push(&node0);

        REQUIRE(queue.size() == 3);

        REQUIRE(queue.pop() == &node0);
        REQUIRE(queue.pop() == &node1);
        REQUIRE(queue.pop() == &node2);
    }

    SECTION("Add several items") {
        auto node3 = TestItem(12);
        auto node4 = TestItem(16);
        auto node5 = TestItem(-1);
        auto node6 = TestItem(5);
        auto node7 = TestItem(9);
        auto node8 = TestItem(9);

        queue.push(&node3);
        queue.push(&node4);
        queue.push(&node5);
        queue.push(&node6);
        queue.push(&node7);
        queue.push(&node8);

        int value = -10;
        while (!queue.empty()) {
            REQUIRE(queue.top()->value >= value);
            value = queue.pop()->value;
        }
    }

    SECTION("Update item") {
        auto node3 = TestItem(12);
        auto node4 = TestItem(16);
        auto node5 = TestItem(-1);
        auto node6 = TestItem(5);
        auto node7 = TestItem(9);
        auto node8 = TestItem(9);

        queue.push(&node3);
        queue.push(&node4);
        queue.push(&node5);
        queue.push(&node6);
        queue.push(&node7);
        queue.push(&node8);

        node4.value = -2;
        REQUIRE(node4.index == 3);
        queue.update(&node4);
        REQUIRE(node4.index == 0);

        int value = -10;
        while (!queue.empty()) {
            REQUIRE(queue.top()->value >= value);
            value = queue.pop()->value;
        }
    }

    SECTION("Clear queue") {
        queue.clear();
        REQUIRE(queue.empty());
    }
}

TEST_CASE("DynamicPriorityQueue forEach test", "[DynamicPriorityQueue]") {
    DynamicPriorityQueue<TestItem*, IndexFunction, ItemCompare, 100, 100> queue;

    auto node1 = TestItem(1);
    auto node2 = TestItem(2);
    auto node0 = TestItem(0);

    queue.push(&node0);
    queue.push(&node1);
    queue.push(&node2);

    int counter = 0;

    auto action = [&](TestItem* node) {
        node->value = -1;
        ++counter;
    };
    queue.forEach(action);

    REQUIRE(counter == 3);
    REQUIRE(node0.value == -1);
    REQUIRE(node1.value == -1);
    REQUIRE(node2.value == -1);
}

TEST_CASE("NonIntrusiveIndexFunction test", "[DynamicPriorityQueue]") {
    DynamicPriorityQueue<TestItem, NonIntrusiveIndexFunction<TestItem, NodeHash, NodeEqual>, NodeCompareRef, 100, 100>
            queue;

    auto node1 = TestItem(1);
    auto node2 = TestItem(2);
    auto node0 = TestItem(0);

    // Note the following queue items are detached from the original items:
    queue.push(node0);
    queue.push(node1);
    queue.push(node2);

    REQUIRE(NodeEqual()(queue.pop(), node0));
    REQUIRE(NodeEqual()(queue.pop(), node1));
    REQUIRE(NodeEqual()(queue.pop(), node2));
}

TEST_CASE("NonIntrusiveIndexFunction contains test", "[DynamicPriorityQueue]") {
    DynamicPriorityQueue<TestItem, NonIntrusiveIndexFunction<TestItem, NodeHash, NodeEqual>, NodeCompareRef, 100, 100>
            queue;

    auto node1 = TestItem(1);

    queue.contains(node1);
}


struct NoCopyItem {
    explicit NoCopyItem(int value) : value(value), index(std::numeric_limits<std::size_t>::max()) {}
    NoCopyItem(const NoCopyItem&) = delete;
    NoCopyItem(NoCopyItem&&) = default;
    NoCopyItem& operator=(const NoCopyItem&) = default;
    NoCopyItem& operator=(NoCopyItem&&) = default;
    ~NoCopyItem() = default;

    int value;
    std::size_t index = std::numeric_limits<std::size_t>::max();
};

struct NoCopyIndexFunction {
    std::size_t& operator()(NoCopyItem* item) { return item->index; }
    std::size_t operator()(const NoCopyItem* item) const { return item->index; }
};

struct NoCopyCompare {
    int operator()(const TestItem* lhs, const TestItem* rhs) const {
        if (lhs->value < rhs->value)
            return -1;
        if (lhs->value > rhs->value)
            return 1;
        return 0;
    }
};

struct NoCopyRefIndexFunction {
  std::size_t& operator()(NoCopyItem& item) { return item.index; }
  std::size_t operator()(const NoCopyItem& item) const { return item.index; }
};

struct NoCopyRefCompare {
    int operator()(const NoCopyItem& lhs, const NoCopyItem& rhs) const {
        if (lhs.value < rhs.value)
            return -1;
        if (lhs.value > rhs.value)
            return 1;
        return 0;
    }
};

struct NoCopyRefPrint {
  void operator()(NoCopyItem& item) {
      WARN("Item: " << item.value << "index: " << item.index);
  }
};

TEST_CASE("Copy restricted test", "[DynamicPriorityQueue]") {
    constexpr int size = 10000;
    DynamicPriorityQueue<NoCopyItem, NoCopyRefIndexFunction, NoCopyRefCompare, size, size>
            queue;


    for (int i = 0; i < size; ++i) {
        queue.push(NoCopyItem{size - 1 - i});
    }

    for (int i = 0; i < size; ++i) {
        REQUIRE(queue.top().value == i);
        REQUIRE(queue.top().index == 0);
        NoCopyItem item = queue.pop();

        REQUIRE(item.value == i);
        REQUIRE(item.index == std::numeric_limits<std::size_t>::max());
    }
}

TEST_CASE("Overflow expection test", "[DynamicPriorityQueue]") {
    DynamicPriorityQueue<NoCopyItem, NoCopyRefIndexFunction, NoCopyRefCompare, 0, 0>
        queue_zero;

    REQUIRE_THROWS_AS(queue_zero.push(NoCopyItem{-1}), std::overflow_error);

    constexpr int size = 10;
    DynamicPriorityQueue<NoCopyItem, NoCopyRefIndexFunction, NoCopyRefCompare, size, size>
        queue;

    for (int i = 0; i < size; ++i) {
        REQUIRE_NOTHROW(queue.push(NoCopyItem{i}));
    }

    REQUIRE_THROWS_AS(queue.push(NoCopyItem{-1}), std::overflow_error);
}

TEST_CASE("Pop underflow expection test", "[DynamicPriorityQueue]") {
    DynamicPriorityQueue<NoCopyItem, NoCopyRefIndexFunction, NoCopyRefCompare, 0, 0>
        queue_zero;

    REQUIRE_THROWS_AS(queue_zero.pop(), std::underflow_error);
    REQUIRE_THROWS_AS(queue_zero.top(), std::underflow_error);

    constexpr int size = 10;
    DynamicPriorityQueue<NoCopyItem, NoCopyRefIndexFunction, NoCopyRefCompare, size, size>
        queue;

    REQUIRE_THROWS_AS(queue.pop(), std::underflow_error);
    REQUIRE_THROWS_AS(queue.top(), std::underflow_error);

    for (int i = 0; i < size; ++i) {
        REQUIRE_NOTHROW(queue.push(NoCopyItem{i}));
    }

    for (int i = 0; i < size; ++i) {
        REQUIRE_NOTHROW(queue.top());
        REQUIRE_NOTHROW(queue.pop());
    }

    REQUIRE_THROWS_AS(queue.pop(), std::underflow_error);
    REQUIRE_THROWS_AS(queue.top(), std::underflow_error);
}

} // namespace
} // namespace cserna
