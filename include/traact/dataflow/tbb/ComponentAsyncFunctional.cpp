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

#include "ComponentAsyncFunctional.h"
#include "NetworkGraph.h"

namespace traact::dataflow {

    ComponentAsyncFunctional::ComponentAsyncFunctional(DefaultPatternPtr pattern_base,
                                                     DefaultComponentPtr component_base,
                                                     TBBTimeDomainManager* buffer_manager,
                                                     NetworkGraph *network_graph) : ComponentBase(std::move(
            pattern_base),
                                                                                                  std::move(
                                                                                                          component_base),
                                                                                                  std::move(
                                                                                                          buffer_manager),
                                                                                                  network_graph),
                                                                                    node_(
                                                                                            nullptr),
                                                                                    join_node_(nullptr) {
        this->component_base_->setReleaseAsyncCallback(std::bind(&ComponentAsyncFunctional::ReleaseGateway,
                                                                 this,
                                                                 std::placeholders::_1,std::placeholders::_2));
    }

    bool ComponentAsyncFunctional::init() {
        using namespace tbb::flow;

        ComponentBase::init();

        node_ = new AsyncNodeType (this->network_graph_->getTBBGraph(),
                                   this->pattern_base_->getConcurrency(),
                                   std::bind(&ComponentAsyncFunctional::submit, this, std::placeholders::_1, std::placeholders::_2));

        int count_input = this->pattern_base_->getConsumerPorts().size();
        if (count_input > 1) {
            join_node_ = new DynamicJoinNode(this->network_graph_->getTBBGraph(), count_input);
            make_edge(join_node_->getSender(), *node_);

        }

        broadcast_node_ = new broadcast_node<TraactMessage>(this->network_graph_->getTBBGraph());
        make_edge(*node_, *broadcast_node_);

        return true;
    }

    bool ComponentAsyncFunctional::teardown() {
        ComponentBase::teardown();

        delete node_;
        if(join_node_)
            delete join_node_;

        return true;
    }

    void ComponentAsyncFunctional::connect() {
        auto tmp = this->pattern_base_->getConsumerPorts();
        for (auto port : tmp) {
            for (auto producer_port : port->connectedToPtr()) {
                tbb::flow::make_edge(this->network_graph_->getSender(producer_port), getReceiver(port->getPortIndex()));
            }
        }
    }

    void ComponentAsyncFunctional::disconnect() {

    }

    tbb::flow::receiver<TraactMessage> &ComponentAsyncFunctional::getReceiver(int index) {
        using namespace tbb::flow;

        if (join_node_ == nullptr) {
            return *node_;
        } else {
            return join_node_->getReceiver(index);
        }
    }
    tbb::flow::sender<TraactMessage> &ComponentAsyncFunctional::getSender(int index) {
        return *broadcast_node_;
    }

    component::ComponentType ComponentAsyncFunctional::getComponentType(){
        return component::ComponentType::AsyncFunctional;
    }

    void ComponentAsyncFunctional::ReleaseGateway(TimestampType ts, bool valid) {
        MapDataType::accessor result;
        async_messages_.find(result,ts);
        if(!result.empty()) {
            result->second.input.valid_data = valid;
            result->second.gateway->try_put(result->second.input);
            result->second.gateway->release_wait();
            async_messages_.erase(result);
        } else {
            spdlog::error("{0}: ReleaseGateway timestamp {1} not found", GetName(), ts.time_since_epoch().count());
        }
    }

    void ComponentAsyncFunctional::submit(TraactMessage in,
                                          tbb::flow::async_node<TraactMessage, TraactMessage>::gateway_type &gateway) {
        gateway.reserve_wait();
        try{

            auto &component_buffer = in.domain_buffer->GetComponentBuffer(component_base_->getName());

            SPDLOG_TRACE("Component {0}; ts {1}; {2}",component_base_->getName(),component_buffer.GetTimestamp().time_since_epoch().count(), in.toString());

            switch (in.message_type) {
                case MessageType::Configure:{
                    bool configure_result = component_base_->configure(pattern_base_->local_pattern.parameter, nullptr);
                    spdlog::info("{0}: configure {1}", GetName(), configure_result);
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
                    work_type w = {in, &gateway};
                    async_messages_.emplace(component_buffer.GetTimestamp(), w);

                    if (in.valid_data) {
                        this->component_base_->processTimePoint(component_buffer);
                    } else {
                        SPDLOG_TRACE("input for component not valid, skip processing: {0}; MeaIndex: {1}", component_base_->getName(), in.event_idx);
                        component_base_->invalidTimePoint(component_buffer.GetTimestamp(), in.event_idx);
                    }
                    return;
                }


                case MessageType::Invalid:
                default:
                {
                    SPDLOG_ERROR("Component {0}; idx {1}; {2}",component_base_->getName(),in.event_idx, "invalid message");
                    break;
                }

            }


            //component_buffer.commit();


        } catch ( ... ) {
            spdlog::error("unknown exception in {0}", component_base_->getName());
            in.valid_data = false;
        }




        gateway.try_put(in);
        gateway.release_wait();

    }

}