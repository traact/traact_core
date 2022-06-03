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

#include "TimeStepBuffer.h"
#include <fmt/format.h>

namespace traact::buffer {
    TimeStepBuffer::TimeStepBuffer(BufferType bufferData,
                                   std::map<int, std::pair<BufferConfig, std::string>> buffer_config, const SourceComponentBuffer::CommitCallback& callback) {

        buffer_data_ = std::move(bufferData);
        component_buffers_list_.resize(buffer_config.size());

        for(const auto& component_config : buffer_config) {
            auto component_index = component_config.first;
            auto& component_name = component_config.second.second;

            auto& port_inputs = component_config.second.first.buffer_to_port_inputs;

            ComponentBuffer::LocalDataBufferType local_input_buffers;
            local_input_buffers.resize(port_inputs.size());
            for (auto port : port_inputs) {
                local_input_buffers[port.second] = buffer_data_[port.first];
            }

            auto& port_outputs = component_config.second.first.buffer_to_port_output;
            ComponentBuffer::LocalDataBufferType local_output_buffers;
            local_output_buffers.resize(port_outputs.size());
            for (auto port : port_outputs) {
                local_output_buffers[port.second] = buffer_data_[port.first];
            }

            component_buffers_list_[component_index] = ComponentBuffer(local_input_buffers,local_output_buffers);

            component_buffer_to_index_.emplace(component_name, component_index);

            switch(component_config.second.first.component_type) {
                // add source components to source buffer list
                case component::ComponentType::AsyncSource:
                case component::ComponentType::SyncSource:{
                    // since map is ordered by component index and components are ordered by sources first no need to allocate the source_buffer_list_ for all components as potential sources for "by index" access
                    source_buffer_list_.emplace_back(std::make_shared<SourceComponentBuffer>(component_buffers_list_[component_index],callback));
                    break;
                }
                // nothing to do for other component types
                case component::ComponentType::SyncFunctional:
                case component::ComponentType::AsyncFunctional:
                case component::ComponentType::SyncSink:
                case component::ComponentType::AsyncSink:
                case component::ComponentType::Invalid:
                default:
                    break;

            }

        }
    }

    std::size_t TimeStepBuffer::GetComponentIndex(const std::string &component_name) {
        auto result = component_buffer_to_index_.find(component_name);
        if(result != component_buffer_to_index_.end())
            return result->second;
        throw std::invalid_argument(fmt::format("No Component Buffer for Component {0}", component_name));
    }

    ComponentBuffer &TimeStepBuffer::GetComponentBuffer(std::size_t component_idx) {
        return component_buffers_list_[component_idx];
    }

    void TimeStepBuffer::SetEvent(TimestampType ts, MessageType message_type) {
        current_ts_ = ts;
        message_type_ = message_type;
        for (auto& comp_buffer : component_buffers_list_) {
            comp_buffer.SetTimestamp(ts);
        }

    }

    traact::TimestampType TimeStepBuffer::GetTimestamp() {
        return current_ts_;
    }

    SourceComponentBuffer *TimeStepBuffer::GetSourceComponentBuffer(std::size_t component_idx) {
        return source_buffer_list_[component_idx].get();
    }

    ComponentBuffer &TimeStepBuffer::GetComponentBuffer(const std::string &component_name) {
        return GetComponentBuffer(GetComponentIndex(component_name));
    }

    std::future<bool> TimeStepBuffer::GetSourceLock(std::size_t component_idx) {
        return source_buffer_list_[component_idx]->GetSourceLock();
    }

    void TimeStepBuffer::ResetLock() {
        for (auto& comp_buffer : source_buffer_list_) {
            if(comp_buffer)
                comp_buffer->ResetLock();
        }
    }

    MessageType TimeStepBuffer::GetEventType() {
        return message_type_;
    }
}


