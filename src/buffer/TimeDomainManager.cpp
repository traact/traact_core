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

#include "traact/buffer/TimeDomainManager.h"

namespace traact::buffer {

TimeDomainManager::TimeDomainManager(TimeDomainManagerConfig config,
                                     std::set<buffer::GenericFactoryObject::Ptr> generic_factory_objects)
        : config_(config){


    for(const auto& factory : generic_factory_objects){
        generic_factory_objects_.emplace(factory->getTypeName(), factory);
    }

    current_mea_idx = 0;
    next_mea_idx_ = 0;


}

GenericSourceTimeDomainBuffer*  TimeDomainManager::RequestBuffer(const TimestampType ts, const std::string &component_name) {

    if(ts == TimestampType::min()){
        SPDLOG_ERROR("invalid timestamp 0 for component {0}", component_name);
        return nullptr;
    }
    std::size_t source_index = name_to_buffer_source_.at(component_name);
    if(ts <= latest_ts_[source_index]){
        SPDLOG_ERROR("invalid timestamp ts {1} <= latest {2} for component {0}", component_name,ts.time_since_epoch().count(), latest_ts_[source_index].time_since_epoch().count());
        return nullptr;
    }
    latest_ts_[source_index] = ts;

    // is it the init call?
    if(ts.time_since_epoch().count() == 1){
        std::size_t source_index = name_to_buffer_source_.at(component_name);
        // wait for free source buffer buffer
        free_source_buffer_lock_[source_index]->wait();
        GenericSourceTimeDomainBuffer* buffer = nullptr;
        if(free_source_buffer_[source_index].try_pop(buffer)) {
            buffer->Init(ts, 0);
            return buffer;
        }
        spdlog::error("could not pop free source buffer for component {0} at ts {1} even though one should be available", component_name, ts.time_since_epoch().count());
        throw std::invalid_argument("could not pop free source buffer for component");
    }

    if(!index_calc_init_){
        if(component_name == config_.master_source){
            TD_Lock::scoped_lock lock(buffer_mutex_);
            // fist real measurement is at idx 1
            first_master_ts_ = ts;
            current_mea_idx = 1;
            for(int init_index = 0; init_index < td_ringbuffer_list_.size(); ++init_index){
                std::size_t nextMeaIdx = next_mea_idx_++;
                TimestampType nextTs = GetMeaIdxTs(nextMeaIdx);
                //std::size_t ring_index = GetTDBufferIdx(nextMeaIdx);
                auto buffer = td_ringbuffer_list_[init_index];

                if(buffer->isUsed()){
                    spdlog::error("td buffer was already in use");
                }

                buffer->resetForTimestamp(nextTs, nextMeaIdx);
                td_ringbuffer_.emplace(nextMeaIdx, buffer);
            }
            SPDLOG_DEBUG("initialize time domain {0} with first timestamp at {1}", config_.time_domain, first_master_ts_.time_since_epoch().count());
            index_calc_init_ = true;
            wait_for_index_index_lock.SetInit(true);
        } else {
            wait_for_index_index_lock.Wait();
        }
    }

    std::size_t meaIdx;
    if(GetMeaIdx(ts, meaIdx)){
        SPDLOG_TRACE("request buffer {0} at ts {1} : meaIdx {2}", component_name, ts.time_since_epoch().count(), meaIdx);

    } else {
        SPDLOG_ERROR("request buffer {0} at ts {1} : timestamp could not be transformed into valid measurement index, timestamps unstable?", component_name, ts.time_since_epoch().count());
        return nullptr;
    }



    // wait for free source buffer buffer
    // TODO switch sourceMode here?
    free_source_buffer_lock_[source_index]->wait();
    GenericSourceTimeDomainBuffer* buffer = nullptr;
    if(free_source_buffer_[source_index].try_pop(buffer)) {
        buffer->Init(ts, meaIdx);
        return buffer;
    }
    spdlog::error("could not pop free source buffer for component {0} at ts {1} even though one should be available", component_name, ts.time_since_epoch().count());
    throw std::invalid_argument("could not pop free source buffer for component");
}



void TimeDomainManager::init(const ComponentGraphPtr &component_graph) {


    using namespace pattern::instance;

    component_graph_ = component_graph;



    auto components = component_graph_->getPatternsForTimeDomain(config_.time_domain);
    std::set<pattern::instance::PatternInstance::Ptr> pattern_instances;

    for (const auto &component : components) {

        std::shared_ptr<PatternInstance> dataComp = component.first;
        pattern_instances.emplace(dataComp);
        if (!dataComp) {
            spdlog::warn("skipping non dataflow pattern : {0}", component.first->instance_id);
            continue;
        }

        for (auto port : dataComp->getProducerPorts()) {
            //if(port->IsConnected()) {
            buffer_datatype_.emplace_back(port->getDataType());
            std::size_t bufferIndex = buffer_datatype_.size() - 1;

            port_to_bufferIndex_[port->getID()] = bufferIndex;

            for (auto input_port : port->connectedToPtr()) {
                port_to_bufferIndex_[input_port->getID()] = bufferIndex;
            }
            //}

        }

        // if the component is a source component
        switch(component.second->getComponentType()){
            // add other potential sources
            case component::ComponentType::AsyncSource: {

                if(component.first->is_master) {
                    //master_component_ = component;
                } else {

                }
                source_components_.push_back(component);
                name_to_buffer_source_[component.first->instance_id] = source_components_.size()-1;

                break;
            }
            default: // nothing to do
                break;
        }
    }

    buffer_data_.resize(config_.ringbuffer_size);

    for(int ringbuffer_index=0; ringbuffer_index < config_.ringbuffer_size; ++ringbuffer_index){
        // create buffer data
        buffer_data_[ringbuffer_index].resize(buffer_datatype_.size());

        std::vector<void*> td_data;
        std::vector<void*> td_header;
        td_data.resize(buffer_datatype_.size());
        td_header.resize(buffer_datatype_.size());
        for(int buffer_index=0; buffer_index < buffer_datatype_.size(); ++buffer_index){
            buffer_data_[ringbuffer_index][buffer_index] = generic_factory_objects_[buffer_datatype_[buffer_index]]->createObject();
            td_data[buffer_index] = buffer_data_[ringbuffer_index][buffer_index];
            td_header[buffer_index] = nullptr;
        }

        // create time domain buffer, source data pointer are set but not used
        td_ringbuffer_list_.push_back(new GenericTimeDomainBuffer(this, buffer_sources_, td_data, td_header, port_to_bufferIndex_,pattern_instances));
    }

    {
        // create additional td_buffer for configure messages
        // does not need data pointer
        std::vector<void*> td_data;
        std::vector<void*> td_header;
        td_data.resize(buffer_datatype_.size());
        td_header.resize(buffer_datatype_.size());
        configure_message_td_buffer_ = new GenericTimeDomainBuffer(this, buffer_sources_, td_data, td_header, port_to_bufferIndex_,pattern_instances);
        TimestampType init_ts = TimestampType(std::chrono::nanoseconds (1));
        configure_message_td_buffer_->resetForTimestamp(init_ts, next_mea_idx_++);
    }



    // create source time domain buffer
    free_source_buffer_.resize(source_components_.size());
    unassigned_source_buffer_.resize(source_components_.size());
    latest_ts_.resize(source_components_.size(), TimestampType::min());

    for (const auto &component : components) {


        std::shared_ptr<PatternInstance> dataComp = component.first;
        if (!dataComp) {
            spdlog::warn("skipping non dataflow pattern : {0}", component.first->instance_id);
            continue;
        }
        // if the component is a source component
        switch(component.second->getComponentType()){
            // add other potential sources
            case component::ComponentType::AsyncSource: {
                break;
            }
            default: // nothing to do
                continue;
        }

        std::size_t source_buffer_index = 0;
        bool buffer_source_found =false;
        for(int i=0;i<buffer_sources_.size();++i){
            if(buffer_sources_[i]->getComponentName() == component.first->instance_id){
                source_buffer_index = i;
                buffer_source_found = true;
            }
        }
        if(!buffer_source_found){
            throw std::runtime_error("buffer source index not found for component");
        }

        bool isMaster = component.first->is_master;
        std::size_t source_td_buffer_index = name_to_buffer_source_.at(dataComp->instance_id);
        std::vector<size_t> global_buffer_index;

        global_buffer_index.resize(dataComp->getProducerPorts().size());


        for (auto port : dataComp->getProducerPorts()) {
            std::size_t buffer_index = port_to_bufferIndex_[port->getID()];
            global_buffer_index[port->getPortIndex()] = buffer_index;
        }

        for(int ringbuffer_index=0; ringbuffer_index < config_.ringbuffer_size; ++ringbuffer_index){
            std::vector<void*> bufferData;
            bufferData.resize(global_buffer_index.size());
            for (auto port : dataComp->getProducerPorts()) {
                std::size_t buffer_index = port_to_bufferIndex_[port->getID()];
                bufferData[port->getPortIndex()] = buffer_data_[ringbuffer_index][buffer_index];
            }
            GenericSourceTimeDomainBuffer* buffer_ptr = new GenericSourceTimeDomainBuffer(bufferData,this, isMaster, source_buffer_index, source_td_buffer_index, global_buffer_index);
            all_source_buffer_.push_back(buffer_ptr);
            free_source_buffer_[source_buffer_index].push(buffer_ptr);
        }

    }

    // init semaphores
    td_ringbuffer_lock_ = std::make_shared<Semaphore>(config_.ringbuffer_size, config_.ringbuffer_size);
    free_source_buffer_lock_.resize(all_source_buffer_.size());
    for(int i=0;i< free_source_buffer_lock_.size();++i){
        free_source_buffer_lock_[i] = std::make_shared<Semaphore>(config_.ringbuffer_size, config_.ringbuffer_size);
    }

    initialized_ = true;


};
SourceMode TimeDomainManager::getSourceMode() const {
  return config_.source_mode;
}

    void TimeDomainManager::RegisterBufferSource(BufferSource::Ptr buffer_source){
    buffer_sources_.push_back(buffer_source);
    name_to_buffer_source_.emplace(buffer_source->getComponentName(),buffer_sources_.size()-1);
}

    void TimeDomainManager::stop(){

        TD_Lock::scoped_lock lock(buffer_mutex_);
        if(!initialized_)
            return;
        // cancel all running buffers
        for(auto& buffer : td_ringbuffer_){
            buffer.second->invalidateBuffer();
        }


        initialized_ = false;


}

    bool TimeDomainManager::CommitSourceBuffer(GenericSourceTimeDomainBuffer *buffer) {
        //SPDLOG_TRACE("commit source buffer {0} at meaIdx {1}, ts {2}",buffer->GetSourceTDBufferIndex(), buffer->GetMeaIdx(), buffer->GetTs().time_since_epoch().count());
        TD_Lock::scoped_lock lock(buffer_mutex_);
        return InternalCommitBuffer(buffer);
    }

    bool TimeDomainManager::IsTimestampMatch(TimestampType master, TimestampType subordiante) {
    //TODO need check since ts 1ns is used for init ts, don't like this
    if(master == subordiante)
        return true;
    if(master.time_since_epoch().count() > 1)
        return ((master-config_.max_offset) < subordiante && (master+config_.max_offset) > subordiante);

    return false;
    }

    void TimeDomainManager::ReleaseTimeDomainBuffer(GenericTimeDomainBuffer *td_buffer) {
        SPDLOG_INFO("release td buffer: ts {0} MeaIndex {1}", td_buffer->getTimestamp().time_since_epoch().count(), td_buffer->GetCurrentMeasurementIndex());





        {
            TD_Lock::scoped_lock lock(buffer_mutex_);

            const auto& source_td_buffer = td_buffer->GetSourceTimeDomainBuffer();
            for(const auto& source_buffer : source_td_buffer){
                if(source_buffer != nullptr){
                    std::size_t buffer_index = source_buffer->GetSourceTDBufferIndex();
                    free_source_buffer_[buffer_index].push(source_buffer);
                    free_source_buffer_lock_[buffer_index]->notify();
                }
            }

            if(td_buffer == configure_message_td_buffer_){
                SPDLOG_INFO("finished with configure message run");
                return;
            }


            std::size_t finished_mea_idx = td_buffer->GetCurrentMeasurementIndex();
            if(finished_mea_idx > current_mea_idx+1){
                spdlog::error("received finished td buffer that is further ahead then expected");
                throw std::runtime_error("received finished td buffer that is further ahead then expected");
            }
            std::size_t nextMeaIdx = next_mea_idx_++;
            TimestampType nextTs = GetMeaIdxTs(nextMeaIdx);
            td_ringbuffer_.erase(finished_mea_idx);
            td_buffer->resetForTimestamp(nextTs, nextMeaIdx);
            td_ringbuffer_.emplace(nextMeaIdx, td_buffer);

            AdvanceBuffers();

            // go through all waiting messages
            for(auto& buffer_list : unassigned_source_buffer_){
                while(!buffer_list.empty()){
                    auto buffer = buffer_list.front();
                    std::size_t mea_idx = buffer->GetMeaIdx();
                    if(mea_idx > GetMaxMeaIdx()){
                        InvalidateOlderTD(buffer->GetMeaIdx(), buffer->GetSourceTDBufferIndex());
                        break;
                    }

                    buffer_list.pop_front();
                    InternalCommitBuffer(buffer);



                }


            }
        }

    }

    bool TimeDomainManager::IsAfterTimestamp(TimestampType master, TimestampType subordinate) {
        //TODO need check since ts 1ns is used for init ts, don't like this
        if(master == subordinate)
            return false;
        if(master.time_since_epoch().count() > 1)
            return (subordinate > (master - config_.max_offset));

        return false;
    }

    bool TimeDomainManager::GetMeaIdx(TimestampType ts, std::size_t &meaIdx) {
        TimeDurationType timeDiff = ts - first_master_ts_;
        double tmp = static_cast<double>(timeDiff.count()) / static_cast<double>(config_.measurement_delta.count());
        // +1 for 0 is initial init
        meaIdx = std::round(tmp)+1;

        TimestampType meaTs = GetMeaIdxTs(meaIdx);

        if(IsTimestampMatch(meaTs, ts)){
            int64_t tmp = static_cast<int64_t>(ts.time_since_epoch().count()) - static_cast<int64_t>(meaTs.time_since_epoch().count());
            SPDLOG_TRACE("timestamp match with offset {0}", tmp);
            return true;
        } else {
            return false;
        }
        //return IsTimestampMatch(meaTs, ts);
    }

    std::size_t TimeDomainManager::GetMaxMeaIdx() {
        return next_mea_idx_ - 1;
    }

//    int TimeDomainManager::GetTDBufferIdx(std::size_t meaIdx) {
//        return (meaIdx-1) % config_.ringbuffer_size;
//    }

    void TimeDomainManager::AdvanceBuffers() {
        meaIdx_offset_++;
        current_mea_idx++;
    }

    TimestampType TimeDomainManager::GetMeaIdxTs(std::size_t meaIdx) {
        return first_master_ts_ + config_.measurement_delta*(meaIdx-1);
    }

    void TimeDomainManager::InvalidateOlderTD(std::size_t mea_idx, std::size_t source_idx) {

        // the data is far in the future, cancel all older td
        if(mea_idx > GetMaxMeaIdx()) {
            SPDLOG_WARN("cancel all td buffer for source, meaIdx {0} in far future, current {1}", mea_idx, GetMaxMeaIdx());
            for(auto td_buffer : td_ringbuffer_){
                if(td_buffer.second->SetInvalidSourceBuffer(source_idx)) {
                    SPDLOG_WARN("cancel source idx {0} at meaIdx {1}, event missing meaIdx {2}", source_idx, mea_idx, td_buffer.second->GetCurrentMeasurementIndex());
                }
            }

        } // cancel only older
        else {
            //for(std::size_t idx = current_mea_idx; idx < mea_idx;++idx){
            for(auto td_buffer : td_ringbuffer_){
                //auto older_td_ring_idx = GetTDBufferIdx(idx);
                if(td_buffer.second->GetCurrentMeasurementIndex() < mea_idx)
                    if(td_buffer.second->SetInvalidSourceBuffer(source_idx)) {
                        SPDLOG_WARN("cancel source idx {0} at meaIdx {1}, event missing meaIdx {2}", source_idx, mea_idx, td_buffer.second->GetCurrentMeasurementIndex());
                    }
            }

        }

    }

    TimeDomainManager::~TimeDomainManager() {
        stop();
        for(auto ptr : td_ringbuffer_list_)
            delete ptr;

        for(auto ptr : all_source_buffer_)
            delete ptr;
        delete configure_message_td_buffer_;

        for(auto& td_buffer_data : buffer_data_){
            for(int i=0;i< td_buffer_data.size();++i){
                generic_factory_objects_[buffer_datatype_[i]]->deleteObject(td_buffer_data[i]);
                td_buffer_data[i] = nullptr;
            }
        }
    }

    bool TimeDomainManager::InternalCommitBuffer(GenericSourceTimeDomainBuffer *buffer) {
        SPDLOG_TRACE("commit source buffer {0} at meaIdx {1}, ts {2}",buffer->GetSourceTDBufferIndex(), buffer->GetMeaIdx(), buffer->GetTs().time_since_epoch().count());

        if(buffer->GetMessageType() == MessageType::Parameter){
            configure_message_td_buffer_->SetSourceBuffer(buffer);
            return true;
        }

        std::size_t mea_idx = buffer->GetMeaIdx();
        std::size_t source_idx = buffer->GetSourceTDBufferIndex();



        {
            if(mea_idx < current_mea_idx){
                spdlog::error("received source buffer for a meaIdx that already finished, current {0}, received {1}", current_mea_idx, mea_idx);
                free_source_buffer_[source_idx].push(buffer);
                free_source_buffer_lock_[source_idx]->notify();
                return false;
            }

            if(mea_idx > GetMaxMeaIdx()){
                SPDLOG_TRACE("source buffer {0} at meaIdx {1}, in future meaIdx, push into waiting queue",buffer->GetSourceTDBufferIndex(), buffer->GetMeaIdx(), buffer->GetTs().time_since_epoch().count());
                //TD_Lock::scoped_lock lock(buffer_mutex_);
                unassigned_source_buffer_[source_idx].push_back(buffer);
                auto oldest_in_list = unassigned_source_buffer_[source_idx].front();
                InvalidateOlderTD(oldest_in_list->GetMeaIdx(), source_idx);
                return true;
            }

            auto result_buffer = td_ringbuffer_.find(mea_idx);
            if(result_buffer == td_ringbuffer_.end()){
                throw std::runtime_error("could not find time domain buffer");
            }
            InvalidateOlderTD(mea_idx, source_idx);
            result_buffer->second->SetSourceBuffer(buffer);


//            auto mea_idx_dist = mea_idx - current_mea_idx;
//            if(mea_idx_dist >= config_.ringbuffer_size-1){
//                SPDLOG_WARN("all time domain buffers used, try cancel source idx {0} ", source_idx);
//                InvalidateOlderTD(mea_idx, source_idx);
//            }

        }


        return true;


    }

}