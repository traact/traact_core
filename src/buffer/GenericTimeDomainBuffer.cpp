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

#include <spdlog/spdlog.h>
#include "traact/buffer/GenericTimeDomainBuffer.h"
#include "traact/buffer/GenericComponentBuffer.h"
#include "traact/buffer/TimeDomainManager.h"

traact::buffer::GenericTimeDomainBuffer::GenericTimeDomainBuffer(int time_domain,TimeDomainManager* manager, component::ComponentGraph::Ptr component_graph,
                                                                 const std::set<buffer::GenericFactoryObject::Ptr> &generic_factory_objects)
    : time_domain_(time_domain), timedomain_manager_(manager), component_graph_(std::move(component_graph)), current_wait_count_(0), maximum_wait_count(0) {
  using namespace pattern::instance;

  for (const auto &item : generic_factory_objects) {
    std::string name = item->getTypeName();
    generic_factory_objects_.emplace(std::make_pair(name, item));
  }

  auto components = component_graph_->getPatternsForTimeDomain(time_domain_);

  std::vector<std::string> bufferTypes;

  std::map<ComponentID_PortName, int> port_to_bufferIndex;

  for (const auto &component : components) {

    std::shared_ptr<PatternInstance> dataComp = component.first;
    if (!dataComp) {
      spdlog::warn("skipping non dataflow pattern : {0}", component.first->instance_id);
      continue;
    }

    for (auto port : dataComp->getProducerPorts()) {
      //if(port->IsConnected()) {
        bufferTypes.emplace_back(port->getDataType());
        std::size_t bufferIndex = bufferTypes.size() - 1;

        addBuffer(port->getDataType());

        port_to_bufferIndex[port->getID()] = bufferIndex;

        for (auto input_port : port->connectedToPtr()) {
          port_to_bufferIndex[input_port->getID()] = bufferIndex;
        }
      //}

    }




    //if(!dataComp->consumer_ports.empty()){
      maximum_wait_count++;
    //}

  }

  for (const auto &component : components) {

    std::shared_ptr<PatternInstance> dataComp = component.first;
    if (!dataComp) {
      continue;
    }

    std::vector<size_t> output;
    output.resize(dataComp->getProducerPorts().size());
    for (PortInstance::ConstPtr port : dataComp->getProducerPorts()) {
      output[port->getPortIndex()] = port_to_bufferIndex[port->getID()];
    }

    std::vector<size_t> input;
    input.resize(dataComp->getConsumerPorts().size());
    for (PortInstance::ConstPtr port : dataComp->getConsumerPorts()) {
      input[port->getPortIndex()] = port_to_bufferIndex[port->getID()];
    }

    component_buffers_[component.first->instance_id] =
        std::make_shared<GenericComponentBuffer>(component.first->instance_id, *this, input, output);

  }

}
traact::buffer::GenericTimeDomainBuffer::~GenericTimeDomainBuffer() {
  for (int index = 0; index < buffer_data_.size(); ++index) {
    const std::string &type = types_of_buffer_[index];
    void *buffer = buffer_data_[index];
    generic_factory_objects_[type]->deleteObject(buffer);
  }

}
traact::buffer::GenericComponentBuffer &traact::buffer::GenericTimeDomainBuffer::getComponentBuffer(
    const std::string &component_name) {
  return *component_buffers_[component_name];
}
const traact::TimestampType &traact::buffer::GenericTimeDomainBuffer::getTimestamp() const {
  return current_timestamp_;
}
bool traact::buffer::GenericTimeDomainBuffer::isFree() const {
  return current_wait_count_ == 0;
}
void traact::buffer::GenericTimeDomainBuffer::resetForTimestamp(traact::TimestampType ts, size_t measurement_index) {
  SPDLOG_TRACE("resetForTimestamp {0} {1}", ts.time_since_epoch().count(), measurement_index);
  current_timestamp_ = ts;
  current_wait_count_ = maximum_wait_count;
  current_measurement_index_ = measurement_index;

}
void traact::buffer::GenericTimeDomainBuffer::decreaseUse() {
  current_wait_count_ -= 1;
  if (current_wait_count_ < 0) {
    SPDLOG_ERROR("use count of buffer smaller then 0");
    throw std::runtime_error("use count of buffer smaller then 0");
  }
  if(current_wait_count_ == 0) {
      timedomain_manager_->releaseBuffer(current_timestamp_);
  }
}
void traact::buffer::GenericTimeDomainBuffer::increaseUse() {
  current_wait_count_ += 1;
}

int traact::buffer::GenericTimeDomainBuffer::getUseCount() const {
  return current_wait_count_;
}
void traact::buffer::GenericTimeDomainBuffer::addBuffer(const std::string &buffer_type) {
  void *newBuffer;

  //TODO real initialized header
  newBuffer = generic_factory_objects_[buffer_type]->createObject();

  buffer_data_.emplace_back(newBuffer);
  buffer_header_.emplace_back(nullptr);
  types_of_buffer_.emplace_back(buffer_type);

}
size_t traact::buffer::GenericTimeDomainBuffer::GetCurrentMeasurementIndex() const {
  return current_measurement_index_;
}
bool traact::buffer::GenericTimeDomainBuffer::initBuffer(std::string buffer_type, void* header, void* buffer){
    return generic_factory_objects_[buffer_type]->initObject(header, buffer);
}
