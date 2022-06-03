/*  BSD 3-Clause License
 *
 *  Copyright (c) 2020, FriederPankratz <frieder.pankratz@gmail.com>
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/

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
        std::size_t event_idx = 0;
        buffer::TimeStepBuffer* buffer = nullptr;

        uint64_t key() const {
            return event_idx;
        }
    };


    struct SourceEventMessage {
        std::size_t event_idx = 0;
        MessageType message_type = MessageType::Invalid;
        bool valid_data = false;
        buffer::SourceComponentBuffer* buffer = nullptr;

        uint64_t key() const {
            return event_idx;
        }

    };


    class SourceMergeNode {
    public:
        typedef typename std::shared_ptr<SourceMergeNode> Ptr;

        typedef typename tbb::flow::join_node<std::tuple<SourceEventMessage, TimeDomainEventMessage>, tbb::flow::key_matching<uint64_t> > JoinNode;
        typedef typename tbb::flow::function_node<std::tuple<SourceEventMessage, TimeDomainEventMessage>, TraactMessage> FunctionNode;

        SourceMergeNode(tbb::flow::graph &graph, std::size_t source_index) : source_index_(source_index)  {
            using namespace tbb::flow;

            join_node_ = new JoinNode(graph);
            function_node_ = new FunctionNode (graph,unlimited,std::bind(&SourceMergeNode::JoinMessages,
                                                                                                          this,
                                                                                                          std::placeholders::_1));



            make_edge(*join_node_, *function_node_);

            sequencer_node_ =
                    new sequencer_node<TraactMessage>(graph, [](const TraactMessage &msg) -> size_t {
                        return msg.event_idx;
                    });

            make_edge( *function_node_, *sequencer_node_);

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
            const SourceEventMessage& source_event = std::get<0>(in);
            const TimeDomainEventMessage& td_event = std::get<1>(in);
            __TBB_ASSERT(source_event.event_idx == td_event.event_idx, "event index of sync input differ in JoinMessages");

            SPDLOG_TRACE("Send TraactMessage into graph: source event idx {0}, time domain idx {1}, message type {2}", source_event.event_idx, td_event.event_idx, source_event.message_type);

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

        std::size_t GetSourceIndex() { return source_index_; };

    private:
        JoinNode *join_node_;
        FunctionNode *function_node_;
        tbb::flow::sequencer_node<TraactMessage> *sequencer_node_;
        std::size_t source_index_;

    };

}

#endif //TRAACTMULTI_SOURCEMERGENODE_H
