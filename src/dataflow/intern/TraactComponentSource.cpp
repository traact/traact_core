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

#include "TraactComponentSource.h"

#include <dataflow/intern/NetworkGraph.h>

namespace traact::dataflow::intern {
    TraactComponentSource::TraactComponentSource(DefaultPatternPtr pattern_base,
                                                 DefaultComponentPtr component_base,
                                                 DefaultTimeDomainManagerPtr buffer_manager,
                                                 NetworkGraph *network_graph) : TraactComponentBase(
            std::move(pattern_base),
            std::move(component_base),
            std::move(buffer_manager),
            network_graph), node_(
            nullptr) {

        this->component_base_->setRequestCallback(std::bind(&TraactComponentSource::RequestBuffer,
                                                            this,
                                                            std::placeholders::_1));


        buffer_manager_->RegisterBufferSource(this);

    }

    bool TraactComponentSource::init() {
        TraactComponentBase::init();
        using namespace tbb::flow;

        node_ = new async_node<TraactMessage, TraactMessage>(this->network_graph_->getTBBGraph(),
                                                                       tbb::flow::unlimited,
                                                                       [&](const TraactMessage &msg,
                                                                           async_node<TraactMessage,
                                                                                   TraactMessage>::gateway_type &gateway) {
                                                                           //asyncNodeActivity.submitRead(gateway);
                                                                           //submitRead(gateway);
                                                                           gateway.try_put(msg);

                                                                       });

        broadcast_node_ = new broadcast_node<TraactMessage>(this->network_graph_->getTBBGraph());
        make_edge(*node_, *broadcast_node_);
        node_->gateway().reserve_wait();

        return true;
    }

    bool TraactComponentSource::start() {
        // TODO need to reserve again if dataflow was stopped and started again
        //node_->gateway().reserve_wait();


        return TraactComponentBase::start();
    }

    bool TraactComponentSource::stop() {
        TraactComponentBase::stop();
        node_->gateway().release_wait();

        return true;
    }

    bool TraactComponentSource::teardown() {
        TraactComponentBase::teardown();
        //node_->gateway().release_wait();

        delete broadcast_node_;
        delete node_;


        return true;
    }

    tbb::flow::sender<TraactMessage> &TraactComponentSource::getSender(int index) {
        return *broadcast_node_;
        //return *node_;
    }

    void TraactComponentSource::connect() {

    }

    void TraactComponentSource::disconnect() {

    }

    int TraactComponentSource::configure_component(TimestampType ts) {
        auto source_buffer = buffer_manager_->RequestBuffer(ts, component_base_->getName());
        if(source_buffer == nullptr)
            return -1;
        source_buffer->SetMessageType(MessageType::Parameter);

        init_component(nullptr);
        //componentBuffer.commit();
        source_buffer->Commit();


    }


    int TraactComponentSource::SendMessage(buffer::GenericTimeDomainBuffer *buffer, bool valid, MessageType msg_type) {
        TraactMessage message;
        TimestampType ts = buffer->getTimestamp();
        message.timestamp = ts;
        message.valid = valid;
        message.message_type = msg_type;

        message.domain_buffer = buffer;
        message.domain_measurement_index = message.domain_buffer->GetCurrentMeasurementIndex();


        //DefaultComponentBuffer& componentBuffer = message.domain_buffer->getComponentBuffer(component_base_->getName());

        SPDLOG_TRACE("Try send message into network Component {0}; ts {1}; {2}", component_base_->getName(), ts.time_since_epoch().count(), message.toString());
        //node_->gateway().reserve_wait();
        if (node_->gateway().try_put(message)) {
            //if (node_->try_put(message)) {
            SPDLOG_TRACE("Component {0}; ts {1}; {2}", component_base_->getName(), ts.time_since_epoch().count(), "try put succeeded");
            //componentBuffer.commit();
            return 0;
        }
        //node_->gateway().release_wait();

        SPDLOG_ERROR("Component {0}; ts {1}; {2}", component_base_->getName(), ts.time_since_epoch().count(), "try put failed");
        //componentBuffer.commit();

        throw std::runtime_error("try_put failed, dataflow network broken");
    }

    std::string TraactComponentSource::getComponentName(){
        return component_base_->getName();
    }

    component::ComponentType TraactComponentSource::getComponentType(){
        return component::ComponentType::AsyncSource;
    }

    bool TraactComponentSource::isMaster(){
        return pattern_base_->is_master;
    }
}

