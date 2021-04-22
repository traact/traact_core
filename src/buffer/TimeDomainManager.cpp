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
                                     std::set<BufferFactory::Ptr> factory_objects)
        : config_(config){


    for(const auto& factory : factory_objects){
        factory_objects_.emplace(factory->getTypeName(), factory);
    }



}



void TimeDomainManager::Init(const ComponentGraphPtr &component_graph) {


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
            case component::ComponentType::SyncSource:
            case component::ComponentType::AsyncSource: {

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
            buffer_data_[ringbuffer_index][buffer_index] = factory_objects_[buffer_datatype_[buffer_index]]->createObject();
            td_data[buffer_index] = buffer_data_[ringbuffer_index][buffer_index];
            td_header[buffer_index] = nullptr;
        }

        // create time domain buffer, source data pointer are set but not used
        td_ringbuffer_list_.push_back(new TimeDomainBuffer(this, buffer_sources_, td_data, td_header, port_to_bufferIndex_,pattern_instances));
    }


    // create source time domain buffer
    all_source_buffer_.resize(source_components_.size());

    for (const auto &component : components) {


        std::shared_ptr<PatternInstance> dataComp = component.first;
        if (!dataComp) {
            spdlog::warn("skipping non dataflow pattern : {0}", component.first->instance_id);
            continue;
        }
        // if the component is a source component
        switch(component.second->getComponentType()){
                // add other potential sources
                case component::ComponentType::AsyncSource:
                case component::ComponentType::SyncSource: {
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

        bool isMaster = component.second->getName() == config_.master_source;
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
            SourceTimeDomainBuffer* buffer_ptr = new SourceTimeDomainBuffer(bufferData,this, isMaster, source_buffer_index, source_td_buffer_index, global_buffer_index);
            all_source_buffer_[source_buffer_index].push_back(buffer_ptr);
        }

    }



};
SourceMode TimeDomainManager::GetSourceMode() const {
  return config_.source_mode;
}

    void TimeDomainManager::RegisterBufferSource(BufferSource::Ptr buffer_source){
    buffer_sources_.push_back(buffer_source);
    name_to_buffer_source_.emplace(buffer_source->getComponentName(),buffer_sources_.size()-1);
}


    TimeDomainManager::~TimeDomainManager() {


        for(auto ptr : td_ringbuffer_list_)
            delete ptr;

        for(auto source_ptr : all_source_buffer_)
            for(auto ptr : source_ptr)
                delete ptr;


        for(auto& td_buffer_data : buffer_data_){
            for(int i=0;i< td_buffer_data.size();++i){
                factory_objects_[buffer_datatype_[i]]->deleteObject(td_buffer_data[i]);
                td_buffer_data[i] = nullptr;
            }
        }
    }

    void TimeDomainManager::SetOutputHeader(std::size_t buffer_index, void *header) {
    // TODO works for initial configure but not after start, integrate into next data message, mark buffer with timestamp for configure time, update when mea idx < config mea idx
    const auto& type_name = buffer_datatype_.at(buffer_index);
    for(int ring_idx = 0; ring_idx < buffer_header_.size();++ring_idx){
        void* header_ptr = buffer_header_[ring_idx][buffer_index];
        void* data_ptr = buffer_data_[ring_idx][buffer_index];
        if(header_ptr != nullptr){
            // TODO delete old header
            //delete
        }

        buffer_header_[ring_idx][buffer_index] = header;
        factory_objects_.at(type_name)->initObject(header, data_ptr);
        //void* header_ptr =
    }



    }




}