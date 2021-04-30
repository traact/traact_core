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

#include "TBBTimeDomainManager.h"
#include <dataflow/tbb/NetworkGraph.h>

using namespace traact::dataflow;

traact::buffer::TBBTimeDomainManager::TBBTimeDomainManager(const traact::buffer::TimeDomainManagerConfig &config,
                                                           const std::set<buffer::BufferFactory::Ptr> &genericFactoryObjects,
                                                           dataflow::NetworkGraph *graph) : TimeDomainManager(config,
                                                                                                                      genericFactoryObjects),
                                                                                                    graph_(graph) {}

void traact::buffer::TBBTimeDomainManager::Init(
        const traact::buffer::TimeDomainManager::ComponentGraphPtr &component_graph) {

    using namespace tbb::flow;

    TimeDomainManager::Init(component_graph);

    std::size_t source_count = all_source_buffer_.size();

    all_source_nodes_.resize(source_count);
    latest_source_event_idx_.resize(source_count, 0);


    free_source_buffer_lock_.resize(source_count);
    free_source_buffer_.resize(source_count);
    latest_ts_.resize(source_count, TimestampType::min());
    for(int i=0;i< free_source_buffer_lock_.size();++i){
        free_source_buffer_lock_[i] = std::make_shared<Semaphore>(config_.ringbuffer_size, config_.ringbuffer_size);
        for(auto tmp : all_source_buffer_[i]){
            free_source_buffer_[i].push(tmp);
        }
    }


    for(int i=0; i < source_count; ++i){
        auto source_merge_node =std::make_shared<SourceMergeNode>(graph_->getTBBGraph(), i);
        all_source_nodes_[i] = source_merge_node;

        const auto& component_name = buffer_sources_[i]->getComponentName();
        auto& source_receiver = graph_->getSourceReceiver(component_name);
        make_edge(source_merge_node->getSender(), source_receiver);

        bool component_found =false;

        for(const auto& pattern_component : source_components_){
            if(pattern_component.second->getName() == component_name) {
                switch (pattern_component.second->getComponentType()) {
                    case component::ComponentType::AsyncSource:  {
                        async_source_nodes_.push_back(source_merge_node);
                        break;
                    }
                    case component::ComponentType::SyncSource:{
                        sync_source_nodes_.push_back(source_merge_node);
                        break;
                    }
                    default:{
                        spdlog::error("unsupported component type in source components: {0} for component {1}, dataflow will not work", (int)pattern_component.second->getComponentType(), component_name);
                    }
                }
                component_found = true;
                break;
            }
        }

        if(!component_found){
            spdlog::error("component not found for source component: {0}", component_name);
        }







    }

    std::vector<std::string> endpoints;

    for(const auto& pattern_component : component_graph_->getPatterns()) {

        bool is_endpoint = true;
        for(const auto& port : pattern_component.first->getProducerPorts()) {
            if(port->IsConnected())
                is_endpoint = false;
        }
        if(is_endpoint)
            endpoints.push_back(pattern_component.second->getName());
    }

    emit_td_node_ = std::make_shared<FunctionNode>(graph_->getTBBGraph(),
                                                   unlimited,
                                                   std::bind(&TBBTimeDomainManager::FinishEvent,
                                                             this,
                                                             std::placeholders::_1));

    if(endpoints.empty()) {
        spdlog::error("no endpoints in dataflow");
    } else if(endpoints.size() == 1){
        auto& end_sender = graph_->getSender(endpoints[0]);
        make_edge(end_sender, *emit_td_node_);

    } else {
        sink_join_node_ = std::make_shared<DynamicJoinNode>(graph_->getTBBGraph(), endpoints.size());

        make_edge(sink_join_node_->getSender(), *emit_td_node_);
        for(std::size_t endpoint_idx = 0; endpoint_idx < endpoints.size(); ++endpoint_idx) {
            auto& end_sender = graph_->getSender(endpoints[endpoint_idx]);
            make_edge(end_sender, sink_join_node_->getReceiver(endpoint_idx));
        }
    }



    for(int i=0;i<td_ringbuffer_list_.size();++i){
        EmitEvent(td_ringbuffer_list_[i]);

    }






    running_ = true;

}

void traact::buffer::TBBTimeDomainManager::Configure() {
    spdlog::info("Send Configure message to all components");



    EmitNonDataEvent(MessageType::Configure);
}

void traact::buffer::TBBTimeDomainManager::Start() {
    spdlog::info("Send Start message to all components");



    spdlog::info("send start message");
    EmitNonDataEvent(MessageType::Start);

    while(running_) {
        if(wait_for_start_finished_lock.Wait()) {
            break;
        } else {
            spdlog::info("waiting for start message to finish");
        }
    }


    spdlog::info("start sources");
    for(const auto& pattern_component : source_components_){
        pattern_component.second->start();
    }



}

void traact::buffer::TBBTimeDomainManager::Stop() {
    spdlog::info("Send Stop message to all components");
    EmitNonDataEvent(MessageType::Stop);

    while(running_) {
        if(wait_for_stop_finished_lock.Wait()) {
            break;
        } else {
            spdlog::info("waiting for start message to finish");
        }
    }




}


void traact::buffer::TBBTimeDomainManager::Teardown() {
    spdlog::info("Send Teardown message to all components");
    EmitNonDataEvent(MessageType::Teardown);
    while(running_) {
        if(wait_for_teardown_finished_lock.Wait()) {
            break;
        } else {
            spdlog::info("waiting for teardown message to finish");
        }
    }
    running_ =false;
}

traact::buffer::SourceTimeDomainBuffer *
traact::buffer::TBBTimeDomainManager::RequestSourceBuffer(const traact::TimestampType ts,
                                                          const std::string &component_name) {

    SPDLOG_TRACE("request buffer {0} at ts {1} : find mea idx", component_name, ts.time_since_epoch().count());

    if(!running_)
        return nullptr;

    std::size_t source_index = name_to_buffer_source_.at(component_name);

    if(ts <= latest_ts_[source_index]){
        SPDLOG_ERROR("invalid timestamp ts {1} <= latest {2} for source {0}", component_name,ts.time_since_epoch().count(), latest_ts_[source_index].time_since_epoch().count());
        return nullptr;
    }
    latest_ts_[source_index] = ts;

    if(!index_calc_init_){
        if(component_name == config_.master_source){
            first_master_ts_ = ts;
            SPDLOG_INFO("initialize time domain {0} with first timestamp at {1}", config_.time_domain, first_master_ts_.time_since_epoch().count());
            index_calc_init_ = true;
            wait_for_init_lock.SetInit(true);
        } else {
            while(running_) {
                if(wait_for_init_lock.Wait()) {
                    break;
                } else {
                    SPDLOG_INFO("wait for master source to initialize");
                }
            }

        }
    }

    if(ts < first_master_ts_)
        return nullptr;



    std::size_t event_idx;
    if(GetMeaIdx(ts, event_idx)){
        SPDLOG_TRACE("request buffer {0} at ts {1} : meaIdx {2}", component_name, ts.time_since_epoch().count(), source_index);

    } else {
        SPDLOG_ERROR("request buffer {0} at ts {1} : timestamp could not be transformed into valid measurement index, timestamps unstable?", component_name, ts.time_since_epoch().count());
        return nullptr;
    }


    auto buffer_result =RequestSourceBuffer(ts, event_idx, source_index );

    if(buffer_result && component_name == config_.master_source){
        EmitSyncSourceEvent(event_idx, ts);
    }


    return buffer_result;
}

traact::buffer::SourceTimeDomainBuffer *
traact::buffer::TBBTimeDomainManager::RequestSourceBuffer(const TimestampType ts,
                                                          const std::size_t event_index,
                                                          const std::size_t source_index) {
    SPDLOG_TRACE("request buffer, source idx {0} at ts {1} : event idx {2}", source_index,ts.time_since_epoch().count(), event_index);

    if(!running_)
        return nullptr;





    switch(config_.source_mode){
        case SourceMode::ImmediateReturn: {
            SourceTimeDomainBuffer* buffer = nullptr;
            if(free_source_buffer_[source_index].try_pop(buffer)) {
                buffer->Init(ts, event_index);
                return buffer;
            }
            return nullptr;
        }
        case SourceMode::WaitForBuffer: {
            while(graph_->IsRunning()) {
                if(free_source_buffer_lock_[source_index]->wait()) {
                    break;
                }
                // no problem here, we wait forever or until program stops
//                else {
//                    SPDLOG_ERROR("timeout when trying to get free source buffer");
//                };
            }

            if(!graph_->IsRunning())
                return nullptr;

            SourceTimeDomainBuffer* buffer = nullptr;
            if(free_source_buffer_[source_index].try_pop(buffer)) {
                buffer->Init(ts, event_index);
                return buffer;
            }
            spdlog::error("could not pop free source buffer for source idx {0} at ts {1} even though one should be available", source_index, ts.time_since_epoch().count());
            throw std::invalid_argument("could not pop free source buffer for component");
        }
    }
}

bool
traact::buffer::TBBTimeDomainManager::CommitSourceBuffer(traact::buffer::SourceTimeDomainBuffer *buffer,
                                                         bool valid) {
    buffer->GetComponentIndex();

    SourceEventMessage msg;
    msg.event_idx = buffer->GetMeaIdx();
    msg.valid_data = valid;
    msg.message_type = MessageType::Data;
    msg.buffer = buffer;

    EmitEvent(msg);

    return true;
}

void traact::buffer::TBBTimeDomainManager::ReleaseTimeDomainBuffer(
        traact::buffer::TimeDomainBuffer *td_buffer) {

    SPDLOG_TRACE("release time domain buffer, event idx {0}, ts {1}", td_buffer->GetCurrentMeasurementIndex(), td_buffer->getTimestamp().time_since_epoch().count());

    // return source buffers
    const auto& source_buffer = td_buffer->GetSourceTimeDomainBuffer();
    for(int source_idx=0;source_idx<source_buffer.size();++source_idx){
        SourceTimeDomainBuffer* buffer = source_buffer[source_idx];
        if(buffer){
            free_source_buffer_[source_idx].push(buffer);
            free_source_buffer_lock_[source_idx]->notify();
        }
    }


    // reset time domain buffer for next event

    if(running_){
        EmitEvent(td_buffer);
    } else {
        free_time_domain_buffer_.push(td_buffer);
    }



}

bool traact::buffer::TBBTimeDomainManager::GetMeaIdx(traact::TimestampType ts, size_t &meaIdx) {
    TimeDurationType timeDiff = ts - first_master_ts_;
    double tmp = static_cast<double>(timeDiff.count()) / static_cast<double>(config_.measurement_delta.count());

    meaIdx = std::round(tmp)+non_data_events_;

    TimestampType meaTs = GetMeaIdxTs(meaIdx);

    if(IsTimestampMatch(meaTs, ts)){
        int64_t tmp = static_cast<int64_t>(ts.time_since_epoch().count()) - static_cast<int64_t>(meaTs.time_since_epoch().count());
        SPDLOG_TRACE("timestamp match with offset {0}", tmp);
        return true;
    } else {
        return false;
    }

}

traact::TimestampType traact::buffer::TBBTimeDomainManager::GetMeaIdxTs(std::size_t meaIdx) {
    return first_master_ts_ + config_.measurement_delta*(meaIdx-non_data_events_);
}

std::size_t traact::buffer::TBBTimeDomainManager::GetNextEventForNonData() {
    std::size_t result;
    {
        TD_Lock::scoped_lock lock(event_idx_lock_);
        result = next_event_idx_++;
        non_data_events_++;
    }
    return result;
}

bool traact::buffer::TBBTimeDomainManager::IsTimestampMatch(traact::TimestampType master, traact::TimestampType subordinate) {
    //TODO need check since ts 1ns is used for Init ts, don't like this
    if(master == subordinate)
        return true;
    if(master.time_since_epoch().count() > 1)
        return ((master-config_.max_offset) < subordinate && (master+config_.max_offset) > subordinate);

    return false;
}

void traact::buffer::TBBTimeDomainManager::EmitEvent(const SourceEventMessage &msg) {
    SPDLOG_TRACE("emit source event, event idx {0}, message {1}", msg.event_idx, (int)msg.message_type );
    // send data messages to specific source node, all other events should be send to all nodes
    switch (msg.message_type) {
        case MessageType::Data: {
            auto source_index =msg.buffer->GetSourceTDBufferIndex();

            // cancel older source events we will not receive anymore
            CancelOlderEvents(source_index, msg.event_idx);


            SPDLOG_TRACE("emit source event data message, event idx {0}, source idx {1}", msg.event_idx, source_index );
            if(!all_source_nodes_[source_index]->getSourceEventMessageReceiver().try_put(msg)) {
                SPDLOG_ERROR("unable to put source event message into graph for source index {0}, event idx {1}", source_index, msg.event_idx);
            }
            latest_source_event_idx_[source_index] = msg.event_idx;



            break;
        }
        default: {
            for(std::size_t source_index=0; source_index < all_source_nodes_.size(); ++source_index ){
                CancelOlderEvents(source_index, msg.event_idx);
                if(!all_source_nodes_[source_index]->getSourceEventMessageReceiver().try_put(msg)) {
                    SPDLOG_ERROR("unable to put source event message into graph for source index {0}", source_index);
                }
                latest_source_event_idx_[source_index] = msg.event_idx;
            }
            break;
        }

    }

    next_event_idx_ = std::max(next_event_idx_, msg.event_idx+1);

}

void traact::buffer::TBBTimeDomainManager::EmitEvent(traact::buffer::TimeDomainBuffer *td_buffer) {
    TD_Lock::scoped_lock lock(event_idx_lock_);
    std::size_t next_idx = next_td_buffer_event_idx_++;

    SPDLOG_TRACE("emit time domain event, event idx {0}", next_idx);

    td_buffer->resetForEvent(next_idx);


    TimeDomainEventMessage msg;
    msg.event_idx = td_buffer->GetCurrentMeasurementIndex();
    msg.buffer = td_buffer;
    for(std::size_t source_index=0; source_index < all_source_nodes_.size(); ++source_index ){
        if(!all_source_nodes_[source_index]->getTimeDomainEventMessage().try_put(msg)) {
            SPDLOG_ERROR("unable to put time domain event message into graph for source index {0}", source_index);
        }
    }


}



void traact::buffer::TBBTimeDomainManager::EmitNonDataEvent(traact::MessageType message_type) {

    SourceEventMessage msg;

    msg.message_type = message_type;
    msg.event_idx = GetNextEventForNonData();
    msg.valid_data = false;

    SPDLOG_TRACE("emit non data source event to all sources, event idx {0}", msg.event_idx);

    EmitEvent(msg);
}

void traact::buffer::TBBTimeDomainManager::CancelOlderEvents(std::size_t source_index, std::size_t current_event) {
    SourceEventMessage cancel_msg;

    cancel_msg.message_type = MessageType::Data;
    cancel_msg.valid_data = false;

    for(std::size_t next_idx = latest_source_event_idx_[source_index]+1; next_idx < current_event;++next_idx){
        cancel_msg.event_idx = next_idx;
        SPDLOG_TRACE("emit source event abort message, event idx {0}, source idx {1}, aborted idx {2}", current_event, source_index, next_idx);

        if(!all_source_nodes_[source_index]->getSourceEventMessageReceiver().try_put(cancel_msg)) {
            SPDLOG_ERROR("unable to put abort ts source event message into graph for source index {0}, event idx {1}", source_index, cancel_msg.event_idx);
        }
    }
}

void traact::buffer::TBBTimeDomainManager::EmitSyncSourceEvent(std::size_t event_idx, traact::TimestampType ts) {
    SPDLOG_TRACE("emit data event to all sync sources, event idx {0}", event_idx);
    for(std::size_t i = 0; i < sync_source_nodes_.size();++i){

        std::size_t source_idx = sync_source_nodes_[i]->GetSourceIndex();

        SourceTimeDomainBuffer* buffer = RequestSourceBuffer(ts, event_idx, source_idx);

        SourceEventMessage msg;
        msg.message_type = MessageType::Data;
        msg.event_idx = event_idx;
        msg.valid_data = true;
        msg.buffer = buffer;

        EmitEvent(msg);
    }
}

void traact::buffer::TBBTimeDomainManager::FinishEvent(const traact::dataflow::TraactMessage &in) {
    TimeDomainBuffer* td_buffer = in.domain_buffer;
    SPDLOG_INFO("finish event, event idx {0}, ts {1}, {2}", td_buffer->GetCurrentMeasurementIndex(), td_buffer->getTimestamp().time_since_epoch().count(), reinterpret_cast<uint64_t>(td_buffer));

    switch (in.message_type) {
        case MessageType::Start:{
            spdlog::info("start message finished");
            wait_for_start_finished_lock.SetInit(true);
            break;
        }
        case MessageType::Stop:{
            spdlog::info("stop message finished");
            wait_for_stop_finished_lock.SetInit(true);
            break;
        }
        case MessageType::Teardown:{
            spdlog::info("teardown message finished");
            wait_for_teardown_finished_lock.SetInit(true);
            break;
        }
        default: {
            break;
        }
    }

    // return source buffers
    const auto& source_buffer = td_buffer->GetSourceTimeDomainBuffer();
    for(int source_idx=0;source_idx<source_buffer.size();++source_idx){
        SourceTimeDomainBuffer* buffer = source_buffer[source_idx];
        if(buffer){
            free_source_buffer_[source_idx].push(buffer);
            free_source_buffer_lock_[source_idx]->notify();
        }
    }


    // reset time domain buffer for next event
    EmitEvent(td_buffer);
//    if(running_){
//        EmitEvent(td_buffer);
//    } else {
//        free_time_domain_buffer_.push(td_buffer);
//    }
}










