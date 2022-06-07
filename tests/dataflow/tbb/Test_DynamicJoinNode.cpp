/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#include <gtest/gtest.h>


//#include "dataflow/tbb/DynamicJoinNode.h"
#include "../../src/dataflow/tbb/DynamicJoinNode.h"

#include "DataflowMessageReceiver.h"

TEST(DynamicJoinNode, Input_1) {
    using namespace traact;
    using namespace traact::dataflow;
    using namespace tbb::flow;
    tbb::flow::graph graph;

    traact::dataflow::DynamicJoinNode node(graph, 1);

    TraactMessage msg;
    msg.message_type = MessageType::Configure;
    msg.event_idx = 0;
    msg.domain_buffer = nullptr;
    msg.valid_data = false;

    DataflowMessageReceiver receiver(&graph);

    make_edge(node.getSender(), *receiver.node_);

    EXPECT_TRUE(node.getReceiver(0).try_put(msg));

    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 1);
    EXPECT_EQ(msg, receiver.msg[0]);

    try {
        node.getReceiver(1);
        FAIL() << "Expected std::out_of_range";
    }
    catch (std::out_of_range const &err) {
        EXPECT_EQ(err.what(), std::string("unsupported index for receiver of dynamic join node"));
    }
    catch (...) {
        FAIL() << "Expected std::out_of_range";
    }
}

TEST(DynamicJoinNode, Input_2) {
    using namespace traact;
    using namespace traact::dataflow;
    using namespace tbb::flow;
    tbb::flow::graph graph;

    traact::dataflow::DynamicJoinNode node(graph, 2);

    TraactMessage msg;
    msg.message_type = MessageType::Configure;
    msg.event_idx = 0;
    msg.domain_buffer = nullptr;
    msg.valid_data = false;

    DataflowMessageReceiver receiver(&graph);

    make_edge(node.getSender(), *receiver.node_);

    EXPECT_TRUE(node.getReceiver(0).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(1).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 1);

    EXPECT_EQ(msg, receiver.msg[0]);

    try {
        node.getReceiver(2);
        FAIL() << "Expected std::out_of_range";
    }
    catch (std::out_of_range const &err) {
        EXPECT_EQ(err.what(), std::string("unsupported index for receiver of dynamic join node"));
    }
    catch (...) {
        FAIL() << "Expected std::out_of_range";
    }
}

TEST(DynamicJoinNode, Input_3) {
    using namespace traact;
    using namespace traact::dataflow;
    using namespace tbb::flow;
    tbb::flow::graph graph;

    traact::dataflow::DynamicJoinNode node(graph, 3);

    TraactMessage msg;
    msg.message_type = MessageType::Configure;
    msg.event_idx = 0;
    msg.domain_buffer = nullptr;
    msg.valid_data = false;

    DataflowMessageReceiver receiver(&graph);

    make_edge(node.getSender(), *receiver.node_);

    EXPECT_TRUE(node.getReceiver(0).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(1).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(2).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 1);

    EXPECT_EQ(msg, receiver.msg[0]);

    try {
        node.getReceiver(3);
        FAIL() << "Expected std::out_of_range";
    }
    catch (std::out_of_range const &err) {
        EXPECT_EQ(err.what(), std::string("unsupported index for receiver of dynamic join node"));
    }
    catch (...) {
        FAIL() << "Expected std::out_of_range";
    }
}

TEST(DynamicJoinNode, Input_4) {
    using namespace traact;
    using namespace traact::dataflow;
    using namespace tbb::flow;
    tbb::flow::graph graph;

    traact::dataflow::DynamicJoinNode node(graph, 4);

    TraactMessage msg;
    msg.message_type = MessageType::Configure;
    msg.event_idx = 0;
    msg.domain_buffer = nullptr;
    msg.valid_data = false;

    DataflowMessageReceiver receiver(&graph);

    make_edge(node.getSender(), *receiver.node_);

    EXPECT_TRUE(node.getReceiver(0).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(1).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(2).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(3).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 1);

    EXPECT_EQ(msg, receiver.msg[0]);

    try {
        node.getReceiver(4);
        FAIL() << "Expected std::out_of_range";
    }
    catch (std::out_of_range const &err) {
        EXPECT_EQ(err.what(), std::string("unsupported index for receiver of dynamic join node"));
    }
    catch (...) {
        FAIL() << "Expected std::out_of_range";
    }
}

TEST(DynamicJoinNode, Input_5) {
    using namespace traact;
    using namespace traact::dataflow;
    using namespace tbb::flow;
    tbb::flow::graph graph;

    traact::dataflow::DynamicJoinNode node(graph, 5);

    TraactMessage msg;
    msg.message_type = MessageType::Configure;
    msg.event_idx = 0;
    msg.domain_buffer = nullptr;
    msg.valid_data = false;

    DataflowMessageReceiver receiver(&graph);

    make_edge(node.getSender(), *receiver.node_);

    EXPECT_TRUE(node.getReceiver(0).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(1).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(2).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(3).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(4).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 1);

    EXPECT_EQ(msg, receiver.msg[0]);

    try {
        node.getReceiver(5);
        FAIL() << "Expected std::out_of_range";
    }
    catch (std::out_of_range const &err) {
        EXPECT_EQ(err.what(), std::string("unsupported index for receiver of dynamic join node"));
    }
    catch (...) {
        FAIL() << "Expected std::out_of_range";
    }
}

TEST(DynamicJoinNode, Input_6) {
    using namespace traact;
    using namespace traact::dataflow;
    using namespace tbb::flow;
    tbb::flow::graph graph;

    traact::dataflow::DynamicJoinNode node(graph, 6);

    TraactMessage msg;
    msg.message_type = MessageType::Configure;
    msg.event_idx = 0;
    msg.domain_buffer = nullptr;
    msg.valid_data = false;

    DataflowMessageReceiver receiver(&graph);

    make_edge(node.getSender(), *receiver.node_);

    EXPECT_TRUE(node.getReceiver(0).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(1).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(2).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(3).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(4).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(5).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 1);

    EXPECT_EQ(msg, receiver.msg[0]);

    try {
        node.getReceiver(6);
        FAIL() << "Expected std::out_of_range";
    }
    catch (std::out_of_range const &err) {
        EXPECT_EQ(err.what(), std::string("unsupported index for receiver of dynamic join node"));
    }
    catch (...) {
        FAIL() << "Expected std::out_of_range";
    }
}

TEST(DynamicJoinNode, Input_7) {
    using namespace traact;
    using namespace traact::dataflow;
    using namespace tbb::flow;
    tbb::flow::graph graph;

    traact::dataflow::DynamicJoinNode node(graph, 7);

    TraactMessage msg;
    msg.message_type = MessageType::Configure;
    msg.event_idx = 0;
    msg.domain_buffer = nullptr;
    msg.valid_data = false;

    DataflowMessageReceiver receiver(&graph);

    make_edge(node.getSender(), *receiver.node_);

    EXPECT_TRUE(node.getReceiver(0).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(1).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(2).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(3).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(4).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(5).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(6).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 1);

    EXPECT_EQ(msg, receiver.msg[0]);

    try {
        node.getReceiver(7);
        FAIL() << "Expected std::out_of_range";
    }
    catch (std::out_of_range const &err) {
        EXPECT_EQ(err.what(), std::string("unsupported index for receiver of dynamic join node"));
    }
    catch (...) {
        FAIL() << "Expected std::out_of_range";
    }
}

TEST(DynamicJoinNode, Input_8) {
    using namespace traact;
    using namespace traact::dataflow;
    using namespace tbb::flow;
    tbb::flow::graph graph;

    traact::dataflow::DynamicJoinNode node(graph, 8);

    TraactMessage msg;
    msg.message_type = MessageType::Configure;
    msg.event_idx = 0;
    msg.domain_buffer = nullptr;
    msg.valid_data = false;

    DataflowMessageReceiver receiver(&graph);

    make_edge(node.getSender(), *receiver.node_);

    EXPECT_TRUE(node.getReceiver(0).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(1).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(2).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(3).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(4).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(5).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(6).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(7).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 1);

    EXPECT_EQ(msg, receiver.msg[0]);

    try {
        node.getReceiver(8);
        FAIL() << "Expected std::out_of_range";
    }
    catch (std::out_of_range const &err) {
        EXPECT_EQ(err.what(), std::string("unsupported index for receiver of dynamic join node"));
    }
    catch (...) {
        FAIL() << "Expected std::out_of_range";
    }
}

TEST(DynamicJoinNode, Input_9) {
    using namespace traact;
    using namespace traact::dataflow;
    using namespace tbb::flow;
    tbb::flow::graph graph;

    traact::dataflow::DynamicJoinNode node(graph, 9);

    TraactMessage msg;
    msg.message_type = MessageType::Configure;
    msg.event_idx = 0;
    msg.domain_buffer = nullptr;
    msg.valid_data = false;

    DataflowMessageReceiver receiver(&graph);

    make_edge(node.getSender(), *receiver.node_);

    EXPECT_TRUE(node.getReceiver(0).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(1).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(2).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(3).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(4).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(5).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(6).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(7).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 0);

    EXPECT_TRUE(node.getReceiver(8).try_put(msg));
    graph.wait_for_all();
    EXPECT_EQ(receiver.msg.size(), 1);

    EXPECT_EQ(msg, receiver.msg[0]);

    try {
        node.getReceiver(9);
        FAIL() << "Expected std::out_of_range";
    }
    catch (std::out_of_range const &err) {
        EXPECT_EQ(err.what(), std::string("unsupported index for receiver of dynamic join node"));
    }
    catch (...) {
        FAIL() << "Expected std::out_of_range";
    }
}

