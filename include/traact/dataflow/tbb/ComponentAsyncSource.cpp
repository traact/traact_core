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

#include "ComponentAsyncSource.h"

#include "NetworkGraph.h"

namespace traact::dataflow {
    ComponentAsyncSource::ComponentAsyncSource(DefaultPatternPtr pattern_base,
                                                           DefaultComponentPtr component_base,
                                                           TBBTimeDomainManager* buffer_manager,
                                                           NetworkGraph *network_graph) : ComponentBase(
            std::move(pattern_base),
            std::move(component_base),
            std::move(buffer_manager),
            network_graph)//, node_(nullptr)
            {

                this->component_base_->setRequestCallback(std::bind(&TBBTimeDomainManager::RequestSourceBuffer,
                                                                    buffer_manager,
                                                                    std::placeholders::_1, component_index_));


        //buffer_manager_->RegisterBufferSource(this);

    }

    bool ComponentAsyncSource::init() {
        ComponentBase::init();
        using namespace tbb::flow;

        node_ = new function_node<TraactMessage, TraactMessage>(this->network_graph_->getTBBGraph(),
                                                                this->pattern_base_->getConcurrency(),
                                                                *this);

        broadcast_node_ = new broadcast_node<TraactMessage>(this->network_graph_->getTBBGraph());
        make_edge(*node_, *broadcast_node_);

        return true;
    }

    bool ComponentAsyncSource::start() {
        return ComponentBase::start();
    }

    bool ComponentAsyncSource::stop() {
        ComponentBase::stop();

        return true;
    }

    bool ComponentAsyncSource::teardown() {
        ComponentBase::teardown();

        delete broadcast_node_;

        return true;
    }

    tbb::flow::sender<TraactMessage> &ComponentAsyncSource::getSender(int index) {
        return *broadcast_node_;
    }

    tbb::flow::receiver<TraactMessage> &ComponentAsyncSource::getReceiver(int index) {
        return *node_;
    }

    void ComponentAsyncSource::connect() {

    }

    void ComponentAsyncSource::disconnect() {

    }


    std::string ComponentAsyncSource::getComponentName(){
        return component_base_->getName();
    }

    component::ComponentType ComponentAsyncSource::getComponentType(){
        return component::ComponentType::AsyncSource;
    }

    TraactMessage ComponentAsyncSource::operator()(const TraactMessage &in) {
        TraactMessage result = in;
        try {


        //DefaultComponentBuffer &component_buffer = in.domain_buffer->getComponentBuffer(component_base_->getName());

        SPDLOG_TRACE("ComponentAsyncSource {0}: {1}",component_base_->getName(), in.toString());

        switch (in.message_type) {
            case MessageType::Configure:{
                bool configure_result = component_base_->configure(pattern_base_->local_pattern.parameter, nullptr);
                spdlog::info("{0}: configure {1}", GetName(), configure_result);
                break;
            }
            case MessageType::Start:{
                //component_base_->start();
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

                break;
            }


            case MessageType::Invalid:
            default:
            {
                SPDLOG_ERROR("Component {0}; idx {1}; {2}",component_base_->getName(),in.event_idx, "invalid message");
                break;
            }

        }



        } catch ( ... ) {
            spdlog::error("unknown exception in {0}", component_base_->getName());
            result.valid_data = false;
        }
        return result;
    }


}

