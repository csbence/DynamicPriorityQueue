#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "../include/dynamic_priority_queue.hpp"

namespace cserna {
namespace {

struct TestNode {
    explicit TestNode(int value) : value(value), index(std::numeric_limits<std::size_t>::max()) {}

    int value;
    mutable std::size_t index;
};

struct IndexFunction {
    std::size_t& operator()(TestNode* testNode) { return testNode->index; }
};

struct NodeCompare {
    int operator()(const TestNode* lhs, const TestNode* rhs) const {
        if (lhs->value < rhs->value)
            return -1;
        if (lhs->value > rhs->value)
            return 1;
        return 0;
    }
};

struct NodeCompareRef {
    int operator()(const TestNode& lhs, const TestNode& rhs) const {
        if (lhs.value < rhs.value)
            return -1;
        if (lhs.value > rhs.value)
            return 1;
        return 0;
    }
};

struct NodeHash {
    std::size_t operator()(const TestNode& node) const { return static_cast<size_t>(node.value); }
};

struct NodeEqual {
    bool operator()(const TestNode& lhs, const TestNode& rhs) const { return lhs.value == rhs.value; }
};

TEST_CASE("IndexFunctionTest", "[DynamicPriorityQueue]") {
    // Index function example

    IndexFunction indexFunction;
    TestNode testNode(1);
    testNode.index = 1;
    NodeCompare nodeCompare;
    nodeCompare(&testNode, &testNode);

    auto& index = indexFunction(&testNode);

    REQUIRE(index == 1);

    index = 2;

    REQUIRE(testNode.index == 2);

    indexFunction(&testNode) = 3;

    REQUIRE(testNode.index == 3);
}

TEST_CASE("DynamicPriorityQueue add/clear tests", "[DynamicPriorityQueue]") {
    DynamicPriorityQueue<TestNode*, IndexFunction, NodeCompare, 100, 100> queue;

    SECTION("Add items to queue") {
        auto node1 = TestNode(1);
        auto node2 = TestNode(2);

        REQUIRE(queue.getSize() == 0);
        REQUIRE(queue.isEmpty());

        queue.push(&node1);

        REQUIRE(queue.getSize() == 1);
        REQUIRE(node1.index == 0);
        REQUIRE(queue.top() == &node1);
        REQUIRE(queue.top() != &node2);
        REQUIRE(!queue.isEmpty());

        queue.push(&node2);

        REQUIRE(queue.getSize() == 2);
    }

    SECTION("Clear queue") {
        queue.clear();
        REQUIRE(queue.getSize() == 0);
    }
}

TEST_CASE("DynamicPriorityQueue order test", "[DynamicPriorityQueue]") {
    DynamicPriorityQueue<TestNode*, IndexFunction, NodeCompare, 100, 100> queue;

    SECTION("Add items to queue") {
        auto node1 = TestNode(1);
        auto node2 = TestNode(2);
        auto node0 = TestNode(0);

        REQUIRE(queue.getSize() == 0);

        queue.push(&node1);
        REQUIRE(queue.getSize() == 1);
        REQUIRE(node1.index == 0);

        queue.push(&node2);
        queue.push(&node0);

        REQUIRE(queue.getSize() == 3);

        REQUIRE(queue.pop() == &node0);
        REQUIRE(queue.pop() == &node1);
        REQUIRE(queue.pop() == &node2);
    }

    SECTION("Add several items") {
        auto node3 = TestNode(12);
        auto node4 = TestNode(16);
        auto node5 = TestNode(-1);
        auto node6 = TestNode(5);
        auto node7 = TestNode(9);
        auto node8 = TestNode(9);

        queue.push(&node3);
        queue.push(&node4);
        queue.push(&node5);
        queue.push(&node6);
        queue.push(&node7);
        queue.push(&node8);

        int value = -10;
        while (!queue.isEmpty()) {
            REQUIRE(queue.top()->value >= value);
            value = queue.pop()->value;
        }
    }

    SECTION("Update item") {
        auto node3 = TestNode(12);
        auto node4 = TestNode(16);
        auto node5 = TestNode(-1);
        auto node6 = TestNode(5);
        auto node7 = TestNode(9);
        auto node8 = TestNode(9);

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
        while (!queue.isEmpty()) {
            REQUIRE(queue.top()->value >= value);
            value = queue.pop()->value;
        }
    }

    SECTION("Clear queue") {
        queue.clear();
        REQUIRE(queue.getSize() == 0);
    }
}

TEST_CASE("DynamicPriorityQueue forEach test", "[DynamicPriorityQueue]") {
    DynamicPriorityQueue<TestNode*, IndexFunction, NodeCompare, 100, 100> queue;

    auto node1 = TestNode(1);
    auto node2 = TestNode(2);
    auto node0 = TestNode(0);

    queue.push(&node0);
    queue.push(&node1);
    queue.push(&node2);

    int counter = 0;

    auto action = [&](TestNode* node) {
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
    DynamicPriorityQueue<TestNode, NonIntrusiveIndexFunction<TestNode, NodeHash, NodeEqual>, NodeCompareRef, 100, 100>
            queue;

    auto node1 = TestNode(1);
    auto node2 = TestNode(2);
    auto node0 = TestNode(0);

    queue.push(node0);
    queue.push(node1);
    queue.push(node2);

    REQUIRE(NodeEqual()(queue.top(), node0));

    node1.value = -1;

    queue.update(node1);
    REQUIRE(NodeEqual()(queue.top(), node1));

    node2.value = -2;

    queue.update(node0);
    queue.update(node1);

    REQUIRE(NodeEqual()(queue.top(), node1));

    queue.update(node2);
    REQUIRE(NodeEqual()(queue.top(), node2));
}

TEST_CASE("NonIntrusiveIndexFunction contains test", "[DynamicPriorityQueue]") {
    DynamicPriorityQueue<TestNode, NonIntrusiveIndexFunction<TestNode, NodeHash, NodeEqual>, NodeCompareRef, 100, 100>
            queue;

    auto node1 = TestNode(1);

    queue.contains(node1);
}

} // namespace
} // namespace cserna
