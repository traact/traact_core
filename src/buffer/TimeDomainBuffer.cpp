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

#include <traact/util/Logging.h>
#include "traact/buffer/TimeDomainBuffer.h"
#include "traact/buffer/ComponentBuffer.h"
#include "traact/buffer/TimeDomainManager.h"




traact::buffer::TimeDomainBuffer::~TimeDomainBuffer() {



}
traact::buffer::ComponentBuffer &traact::buffer::TimeDomainBuffer::getComponentBuffer(
    const std::string &component_name) {
  return *component_buffers_[component_name];
}
const traact::TimestampType &traact::buffer::TimeDomainBuffer::getTimestamp() const {
  return current_timestamp_;
}
bool traact::buffer::TimeDomainBuffer::isFree() const {
  return current_wait_count_ == 0;
}
void traact::buffer::TimeDomainBuffer::resetForEvent(size_t event_idx, TimestampType ts) {

    //LockType::scoped_lock lock(source_mutex_);
    SPDLOG_TRACE("resetForEvent: before event idx: {0}, now event idx {1}", current_measurement_index_, event_idx);

    current_measurement_index_ = event_idx;
    current_timestamp_ = ts;
    current_wait_count_ = maximum_wait_count_;
    for(int i=0;i<td_buffer_sources_valid_.size();++i){
        //td_buffer_sources_valid_[i] = false;
        td_buffer_sources_[i] = nullptr;
        //td_buffer_sources_send_[i] = false;
    }
    is_used_ = true;
}
void traact::buffer::TimeDomainBuffer::decreaseUse() {
    //LockType::scoped_lock lock(source_mutex_);
    SPDLOG_TRACE("decrease use, count {0}, event idx {1}", current_wait_count_, current_measurement_index_);
  //current_wait_count_--;
  if (current_wait_count_ < 0) {
    SPDLOG_ERROR("use count of buffer ts {1} smaller then 0 : {0}", current_wait_count_, current_timestamp_.time_since_epoch().count());
    //throw std::runtime_error("use count of buffer smaller then 0");
  }
  if(current_wait_count_ == 0) {
      SPDLOG_TRACE("release time domain buffer: event idx {0}", current_measurement_index_);
      is_used_ = false;
      //timedomain_manager_->ReleaseTimeDomainBuffer(this);
  }
}
void traact::buffer::TimeDomainBuffer::increaseUse() {
    //LockType::scoped_lock lock(source_mutex_);
    current_wait_count_ ++;
}

int traact::buffer::TimeDomainBuffer::getUseCount() const {
  return current_wait_count_;
}

size_t traact::buffer::TimeDomainBuffer::GetCurrentMeasurementIndex() const {
  return current_measurement_index_;
}


bool traact::buffer::TimeDomainBuffer::isSourcesSet()  {
    bool result = true;
    for(bool is_set : td_buffer_sources_valid_){
        result = result && is_set;
    }
    return result;
}

bool traact::buffer::TimeDomainBuffer::isUsed() const {
    return is_used_;
}
void traact::buffer::TimeDomainBuffer::invalidateBuffer() {

    for(int index = 0; index < td_buffer_sources_valid_.size();++ index){
        SetInvalidSourceBuffer(index);
    }


}

bool traact::buffer::TimeDomainBuffer::SetInvalidSourceBuffer(std::size_t source_buffer) {

    if(!td_buffer_sources_send_[source_buffer]){
        td_buffer_sources_send_[source_buffer] = true;
        //buffer_sources_[source_buffer]->SendMessage(this, false, MessageType::Data);
        decreaseUse();
        return true;
    }

    return false;
}

traact::buffer::TimeDomainBuffer::TimeDomainBuffer(traact::buffer::TimeDomainManager *timedomainManager,
                                                                 std::vector<BufferSource*> bufferSources,
                                                                 traact::buffer::TimeDomainBuffer::BufferType bufferData,
                                                                 traact::buffer::TimeDomainBuffer::BufferType bufferHeader,
                                                                 const std::map<pattern::instance::ComponentID_PortName, int> &port_to_bufferIndex,
                                                                 const std::set<pattern::instance::PatternInstance::Ptr> &components)
                                                                 : timedomain_manager_(timedomainManager),
                                                                 buffer_sources_(std::move(bufferSources)),
                                                                 buffer_data_(std::move(bufferData)),
                                                                 buffer_header_(std::move(bufferHeader)) {

    using namespace traact::pattern::instance;

    for (const auto &dataComp : components) {

        if (!dataComp) {
            continue;
        }

        std::vector<size_t> output;
        output.resize(dataComp->getProducerPorts().size());
        for (PortInstance::ConstPtr port : dataComp->getProducerPorts()) {
            output[port->getPortIndex()] = port_to_bufferIndex.at(port->getID());
        }

        std::vector<size_t> input;
        input.resize(dataComp->getConsumerPorts().size());
        for (PortInstance::ConstPtr port : dataComp->getConsumerPorts()) {
            auto port_result = port_to_bufferIndex.find(port->getID());
            if(port_result != port_to_bufferIndex.end()){
                input[port->getPortIndex()] = port_result->second;
            } else {
                std::stringstream ss;
                for(const auto& port_buffer : port_to_bufferIndex){

                    ss << port_buffer.first.first << ":" << port_buffer.first.second << "\n";
                }
                spdlog::error("could not find buffer index for port: {0}:{1} \n in ports: \n {2}", port->getID().first, port->getID().second, ss.str());
                throw std::runtime_error("unknown buffer index");
            }

        }

        component_buffers_[dataComp->instance_id] =
                std::make_shared<ComponentBuffer>(dataComp->instance_id, *this, input, output);
        component_buffers_config_[dataComp->instance_id] =
                std::make_shared<ComponentBufferConfig>(dataComp->instance_id, *this, input, output);

    }

    maximum_wait_count_ = components.size();
    td_buffer_sources_valid_.resize(buffer_sources_.size(), false);
    td_buffer_sources_.resize(buffer_sources_.size(), nullptr);
    td_buffer_sources_send_.resize(buffer_sources_.size(), false);
    is_used_ = false;
    is_master_set_ = false;
    current_measurement_index_ = 0;

}

void
traact::buffer::TimeDomainBuffer::SetSourceBuffer(traact::buffer::SourceTimeDomainBuffer *source_buffer) {

    SPDLOG_TRACE("set source buffer: event idx {0}, source idx {1}, event idx domain buffer {2}", source_buffer->GetMeaIdx(),source_buffer->GetSourceTDBufferIndex(), current_measurement_index_);

    if(!is_used_){
        spdlog::error("trying to set source buffer for invalid time domain buffer: event idx {0}, source idx {1}", source_buffer->GetMeaIdx(),source_buffer->GetSourceTDBufferIndex());
        throw std::runtime_error("trying to set source buffer for invalid time domain buffer");
        return;
    }



    const std::size_t source_index = source_buffer->GetSourceTDBufferIndex();
    //MessageType msg_type = source_buffer->GetMessageType();
//    if(td_buffer_sources_valid_[source_index]){
//        spdlog::error("source {0} is already set for buffer idx: {2}", source_index, current_measurement_index_);
//        throw std::runtime_error("trying to set already existing source");
//    }
    // decreaseUse after all messages are send
    {
       // LockType::scoped_lock lock(source_mutex_);
        //td_buffer_sources_valid_[source_index] = true;
        td_buffer_sources_[source_index] = source_buffer;
        const auto& global_buffer_index = source_buffer->GetGlobalBufferIndex();
        void** data = source_buffer->GetBufferData();

        for(int index = 0; index < global_buffer_index.size();++ index){
            buffer_data_[global_buffer_index[index]] = data[index];
        }

//        if(source_buffer->IsMaster()){
//            is_master_set_ = true;
//            current_timestamp_ = source_buffer->GetTs();
//        }

        //td_buffer_sources_send_[source_index] = true;
        //buffer_sources_[source_index]->SendMessage(this, true, msg_type);

    }

    decreaseUse();

}








const std::vector<traact::buffer::SourceTimeDomainBuffer *> &
traact::buffer::TimeDomainBuffer::GetSourceTimeDomainBuffer() const {
    return td_buffer_sources_;
}

traact::buffer::ComponentBufferConfig *
traact::buffer::TimeDomainBuffer::getComponentBufferConfig(const std::string &component_name) {
    return component_buffers_config_.at(component_name).get();
}

const void traact::buffer::TimeDomainBuffer::setOutputHeader(size_t index, void *header) {
    timedomain_manager_->SetOutputHeader(index, header);
    return;
}


