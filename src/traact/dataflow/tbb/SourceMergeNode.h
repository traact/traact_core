/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACTMULTI_SOURCEMERGENODE_H
#define TRAACTMULTI_SOURCEMERGENODE_H

#include <tbb/flow_graph.h>
#include <traact/datatypes.h>
#include <tuple>
#include <traact/buffer/TimeStepBuffer.h>
#include <traact/buffer/SourceComponentBuffer.h>
#include "DataflowMessage.h"

namespace traact::dataflow {

struct TimeDomainEventMessage {
    size_t event_idx = 0;
    buffer::TimeStepBuffer *buffer = nullptr;

    uint64_t key() const {
        return event_idx;
    }
};

struct SourceEventMessage {
    size_t event_idx = 0;
    MessageType message_type = MessageType::Invalid;
    bool valid_data = false;
    buffer::SourceComponentBuffer *buffer = nullptr;

    uint64_t key() const {
        return event_idx;
    }

};

class SourceMergeNode {
 public:
    typedef typename std::shared_ptr<SourceMergeNode> Ptr;

    typedef typename tbb::flow::join_node<std::tuple<SourceEventMessage, TimeDomainEventMessage>,
                                          tbb::flow::key_matching<uint64_t> > JoinNode;
    typedef typename tbb::flow::function_node<std::tuple<SourceEventMessage, TimeDomainEventMessage>, TraactMessage>
        FunctionNode;

    SourceMergeNode(tbb::flow::graph &graph, size_t source_index) : source_index_(source_index) {
        using namespace tbb::flow;

        join_node_ = new JoinNode(graph);
        function_node_ = new FunctionNode(graph, unlimited, std::bind(&SourceMergeNode::JoinMessages,
                                                                      this,
                                                                      std::placeholders::_1));

        make_edge(*join_node_, *function_node_);

        sequencer_node_ =
            new sequencer_node<TraactMessage>(graph, [](const TraactMessage &msg) -> size_t {
                return msg.event_idx;
            });

        make_edge(*function_node_, *sequencer_node_);

    }
    ~SourceMergeNode() {
        delete sequencer_node_;
        delete function_node_;
        delete join_node_;
    }

    tbb::flow::sender<TraactMessage> &getSender() {
        return *sequencer_node_;

    }

    tbb::flow::receiver<SourceEventMessage> &getSourceEventMessageReceiver() {
        using namespace tbb::flow;
        return input_port<0>(*join_node_);

    }

    tbb::flow::receiver<TimeDomainEventMessage> &getTimeDomainEventMessage() {
        using namespace tbb::flow;
        return input_port<1>(*join_node_);

    }

    TraactMessage JoinMessages(const std::tuple<SourceEventMessage, TimeDomainEventMessage> &in) {
        const SourceEventMessage &source_event = std::get<0>(in);
        const TimeDomainEventMessage &td_event = std::get<1>(in);
        __TBB_ASSERT(source_event.event_idx == td_event.event_idx, "event index of sync input differ in JoinMessages");

        SPDLOG_TRACE("Send TraactMessage into graph: source event idx {0}, time domain idx {1}, message type {2}",
                     source_event.event_idx,
                     td_event.event_idx,
                     source_event.message_type);

        TraactMessage result;


//            if(source_event.valid_data && source_event.buffer)
//                td_event.buffer->SetSourceBuffer(source_event.buffer);
//            else
//                td_event.buffer->decreaseUse();

        result.event_idx = source_event.event_idx;
        result.message_type = source_event.message_type;
        result.valid_data = source_event.valid_data;
        result.domain_buffer = td_event.buffer;

        return result;

    }

    size_t GetSourceIndex() { return source_index_; };

 private:
    JoinNode *join_node_;
    FunctionNode *function_node_;
    tbb::flow::sequencer_node<TraactMessage> *sequencer_node_;
    size_t source_index_;

};

}

#endif //TRAACTMULTI_SOURCEMERGENODE_H
