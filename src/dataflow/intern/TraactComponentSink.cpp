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
#include "TraactComponentSink.h"

#include <dataflow/intern/NetworkGraph.h>

namespace traact::dataflow::intern {
TraactComponentSink::TraactComponentSink(DefaultPatternPtr pattern_base,
                                         DefaultComponentPtr component_base,
                                         DefaultTimeDomainManagerPtr buffer_manager,
                                         NetworkGraph *network_graph) : TraactComponentBase(std::move(pattern_base),
                                                                                            std::move(component_base),
                                                                                            std::move(buffer_manager),
                                                                                            network_graph) {
  node_ = nullptr;
  join_node_ = nullptr;
  sequencer_node_ = nullptr;
}

bool TraactComponentSink::init() {
  using namespace tbb::flow;

  TraactComponentBase::init();

  node_ = new function_node<TraactMessage>(this->network_graph_->getTBBGraph(),
                                           this->pattern_base_->getConcurrency(),
                                           *this);

  int count_input = this->pattern_base_->getConsumerPorts().size();
  if (count_input > 1) {
    join_node_ = new DynamicJoinNode(this->network_graph_->getTBBGraph(), count_input);

  }

  if (pattern_base_->getConcurrency() != unlimited) {
    sequencer_node_ =
        new sequencer_node<TraactMessage>(network_graph_->getTBBGraph(), [](const TraactMessage &msg) -> size_t {
          return msg.domain_measurement_index;
        });

    if (count_input > 1) {
      make_edge(join_node_->getSender(), *sequencer_node_);
    }

    make_edge(*sequencer_node_, *node_);

  } else {
    if (count_input > 1) {
      make_edge(join_node_->getSender(), *node_);
    }
  }

  return true;
}

bool TraactComponentSink::teardown() {
  TraactComponentBase::teardown();

  delete node_;
  delete join_node_;
  delete sequencer_node_;

  return true;
}

void TraactComponentSink::operator()(const TraactMessage &in) {
    SPDLOG_INFO("Component {0}; ts {1}; {2}",component_base_->getName(),in.timestamp.time_since_epoch().count(), in.toString());

    DefaultComponentBuffer
            &component_buffer = this->buffer_manager_->acquireBuffer(in.timestamp, this->component_base_->getName());
    switch (in.message_type) {
        case MessageType::Parameter: {
            init_component(component_buffer);
            break;
        }
            //case MessageType::Parameter:
        case MessageType::Data: {
            if (in.valid) {
                this->component_base_->processTimePoint(component_buffer);
            } else {
                spdlog::trace("input for component not valid, skip processing: {0}", component_base_->getName());
            }
            break;
        }
        case MessageType::AbortTs:{
            spdlog::warn("abort ts message: component {0}", component_base_->getName());
            break;
        }
        case MessageType::Invalid:
        default: {
            spdlog::error("invalid message: {0}", component_base_->getName());
            break;
        }

    }

    component_buffer.commit();


}
void TraactComponentSink::connect() {
  auto tmp = this->pattern_base_->getConsumerPorts();

  for (auto port : tmp) {
    for (auto producer_port : port->connectedToPtr()) {
      tbb::flow::make_edge(this->network_graph_->getSender(producer_port), getReceiver(port->getPortIndex()));
    }
  }
}
void TraactComponentSink::disconnect() {

}

tbb::flow::receiver<TraactMessage> &TraactComponentSink::getReceiver(int index) {
  using namespace tbb::flow;


  if (pattern_base_->getConcurrency() != unlimited) {
    if (join_node_ == nullptr) {
      return *sequencer_node_;
    } else {
      return join_node_->getReceiver(index);
    }
  } else {
    if (join_node_ == nullptr) {
      return *node_;
    } else {
      return join_node_->getReceiver(index);
    }
  };

}

    component::ComponentType TraactComponentSink::getComponentType(){
    return component::ComponentType::SyncSink;
}
}