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

#include "TraactComponentSyncSource.h"
#include "dataflow/intern/NetworkGraph.h"

namespace traact::dataflow::intern {

    TraactComponentSyncSource::TraactComponentSyncSource(DefaultPatternPtr pattern_base,
                                                         DefaultComponentPtr component_base,
                                                         DefaultTimeDomainManagerPtr buffer_manager,
                                                         NetworkGraph *network_graph) : TraactComponentBase(std::move(
            pattern_base),
                                                                                                            std::move(
                                                                                                                    component_base),
                                                                                                            std::move(
                                                                                                                    buffer_manager),
                                                                                                            network_graph),
                                                                                        node_(
                                                                                                nullptr),
                                                                                        join_node_(nullptr) {

        buffer_manager_->RegisterBufferSource(this);
    }

    bool TraactComponentSyncSource::init() {
        using namespace tbb::flow;

        TraactComponentBase::init();

        node_ = new function_node<TraactMessage, TraactMessage>(this->network_graph_->getTBBGraph(),
                                                                this->pattern_base_->getConcurrency(),
                                                                *this);

//        int count_input = this->pattern_base_->getConsumerPorts().size();
//        if (count_input > 1) {
//            join_node_ = new DynamicJoinNode(this->network_graph_->getTBBGraph(), count_input);
//            make_edge(join_node_->getSender(), *node_);
//
//        }

        broadcast_node_ = new broadcast_node<TraactMessage>(this->network_graph_->getTBBGraph());
        make_edge(*node_, *broadcast_node_);

        return true;
    }

    bool TraactComponentSyncSource::teardown() {
        TraactComponentBase::teardown();

        delete node_;
//        if(join_node_)
//            delete join_node_;

        return true;
    }

    TraactMessage TraactComponentSyncSource::operator()(const TraactMessage &in) {

        TraactMessage result = in;

        DefaultComponentBuffer &component_buffer = in.domain_buffer->getComponentBuffer(component_base_->getName());

        SPDLOG_DEBUG("Component {0}; ts {1}; {2}",component_base_->getName(),component_buffer.getTimestamp().time_since_epoch().count(), in.toString());

        switch (in.message_type) {
            case MessageType::Configure:{
                component_base_->configure(pattern_base_->pattern_pointer.parameter, nullptr);
                break;
            }
            case MessageType::Start:{
                component_base_->start();
                break;
            }
            case MessageType::Stop:{
                component_base_->stop();
                break;
            }
            case MessageType::Teardown:{
                component_base_->teardown();
                break;
            }

            case MessageType::Data: {
                if(in.valid_data) {
                    spdlog::trace("input for component valid, processing: {0}; MeaIndex: {1}", component_base_->getName(), in.event_idx);
                    result.valid_data = component_base_->processTimePoint(component_buffer);
                } else {
                    spdlog::trace("input for component not valid, skip processing: {0}; MeaIndex: {1}", component_base_->getName(), in.event_idx);
                    component_base_->invalidTimePoint(component_buffer.getTimestamp(), in.event_idx);
                }

                break;
            }


            case MessageType::Invalid:
            default:
            {
                SPDLOG_ERROR("Component {0}; idx {1}; {2}",component_base_->getName(),in.event_idx, "invalid message");
                break;
            }

        }


        return result;
    }

    void TraactComponentSyncSource::connect() {
        return;
//        auto tmp = this->pattern_base_->getConsumerPorts();
//        for (auto port : tmp) {
//            for (auto producer_port : port->connectedToPtr()) {
//                tbb::flow::make_edge(this->network_graph_->getSender(producer_port), getReceiver(port->getPortIndex()));
//            }
//        }
    }

    void TraactComponentSyncSource::disconnect() {

    }

    tbb::flow::receiver<TraactMessage> &TraactComponentSyncSource::getReceiver(int index) {
        using namespace tbb::flow;
//
//        if (join_node_ == nullptr) {
//            return *node_;
//        } else {
//            return join_node_->getReceiver(index);
//        }

        return *node_;
    }
    tbb::flow::sender<TraactMessage> &TraactComponentSyncSource::getSender(int index) {
        return *broadcast_node_;
    }

    component::ComponentType TraactComponentSyncSource::getComponentType(){
        return component::ComponentType::SyncSource;
    }

    std::string TraactComponentSyncSource::getComponentName() {
        return component_base_->getName();
    }

}