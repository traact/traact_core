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

#include "TimeDomainBuffer.h"

namespace traact::buffer {

    TimeDomainBuffer::TimeDomainBuffer(int time_domain, std::set<buffer::BufferFactory::Ptr> factory_objects) : time_domain_{time_domain}{
        for(const auto& factory : factory_objects){
            factory_objects_.emplace(factory->getTypeName(), factory);
        }
    }

    TimeDomainBuffer::~TimeDomainBuffer() {
        for(auto& td_buffer_data : buffer_data_){
            for(int i=0;i< td_buffer_data.size();++i){
                factory_objects_[buffer_datatype_[i]]->deleteObject(td_buffer_data[i]);
            }
        }
    }

    void TimeDomainBuffer::Init(const component::ComponentGraph &component_graph, const SourceComponentBuffer::CommitCallback& callback) {
        using namespace pattern::instance;

        config_ = component_graph.GetTimeDomainConfig(time_domain_);
        auto components = component_graph.getPatternsForTimeDomain(time_domain_);
        // order components as async source first, then sync sources, then the rest with sinks as last, use order of ComponentTypeEnum
        std::vector<component::ComponentGraph::PatternComponentPair> ordered_components(components.begin(), components.end());
        std::sort(ordered_components.begin(),ordered_components.end(), [](const component::ComponentGraph::PatternComponentPair & a, const component::ComponentGraph::PatternComponentPair & b) -> bool {
            return a.second->getComponentType() < b.second->getComponentType();
        });

        // gather all output buffers to allocate memory, input must be connected to an output
        for (const auto &component : ordered_components) {

            std::shared_ptr<PatternInstance> dataComp = component.first;
            if (!dataComp) {
                spdlog::warn("skipping non dataflow pattern : {0}", component.first->instance_id);
                continue;
            }

            if(component.second->getComponentType() == component::ComponentType::AsyncSource)
                count_async_sources_++;
            if(component.second->getComponentType() == component::ComponentType::SyncSource)
                count_sync_sources_++;

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


        }

        // create buffer data
        buffer_data_.resize(config_.ringbuffer_size);
        for(int ringbuffer_index=0; ringbuffer_index < config_.ringbuffer_size; ++ringbuffer_index){
            buffer_data_[ringbuffer_index].resize(buffer_datatype_.size());
            for(int buffer_index=0; buffer_index < buffer_datatype_.size(); ++buffer_index){
                buffer_data_[ringbuffer_index][buffer_index] = factory_objects_[buffer_datatype_[buffer_index]]->createObject();
            }
        }

        // create component buffer configs
        int component_index = 0;
        for (const auto &component : ordered_components) {
            std::shared_ptr<PatternInstance> dataComp = component.first;

            BufferConfig buffer_config;

            if (!dataComp) {
                continue;
            }
            buffer_config.buffer_to_port_output.reserve(dataComp->getProducerPorts().size());
            for (auto port : dataComp->getProducerPorts()) {
                auto global_buffer_index = port_to_bufferIndex_[port->getID()];
                buffer_config.buffer_to_port_output.emplace_back(global_buffer_index, port->getPortIndex());
            }
            buffer_config.buffer_to_port_inputs.reserve(dataComp->getConsumerPorts().size());
            for (auto port : dataComp->getConsumerPorts()) {
                auto global_buffer_index = port_to_bufferIndex_[port->getID()];
                buffer_config.buffer_to_port_inputs.emplace_back(global_buffer_index, port->getPortIndex());
            }
            buffer_config.component_type = component.second->getComponentType();
            buffer_config_.emplace(component_index, std::make_pair(buffer_config,component.first->instance_id));
            component_index++;
        }
        for(int ringbuffer_index=0; ringbuffer_index < config_.ringbuffer_size; ++ringbuffer_index){
            // create time domain buffer, source data pointer are set but not used
            time_step_buffer_.emplace_back(buffer_data_[ringbuffer_index], buffer_config_, callback);
        }


    }

    TimeStepBuffer &TimeDomainBuffer::GetTimeStepBuffer(size_t concurrent_index) {
        return time_step_buffer_[concurrent_index];
    }

    int TimeDomainBuffer::GetComponentIndex(const std::string &instance_id) {

        for (const auto& index_config_id : buffer_config_) {
            if(index_config_id.second.second == instance_id)
                return index_config_id.first;
        }

        assert(false);

//        auto result = buffer_config_.find(instance_id);
//        assert(result != buffer_config_.end());
//        return result->second.second;
    }

    int TimeDomainBuffer::GetCountAsyncSources() const {
        return count_async_sources_;
    }

    int TimeDomainBuffer::GetCountSyncSources() const {
        return count_sync_sources_;
    }

    int TimeDomainBuffer::GetCountSources() const {
        return GetCountAsyncSources()+GetCountSyncSources();
    }

    void TimeDomainBuffer::CancelAll() {
        for(int ringbuffer_index=0; ringbuffer_index < config_.ringbuffer_size; ++ringbuffer_index){
            CancelAllSources(ringbuffer_index, true);
        }
    }

    void TimeDomainBuffer::CancelAllSources(int ringbuffer_index, bool valid) {
        for (int source_index = 0; source_index < GetCountSources(); ++source_index) {
            time_step_buffer_[ringbuffer_index].GetSourceComponentBuffer(source_index)->Cancel();
        }
    }
}