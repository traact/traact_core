/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include <traact/buffer/ComponentBuffer.h>

#include <utility>

namespace traact::buffer {
ComponentBuffer::ComponentBuffer(size_t component_index,
                                 LocalDataBuffer input_buffer,
                                 LocalHeaderBuffer input_header,
                                 LocalValidBuffer input_valid,
                                 LocalTimestampBuffer input_timestamp,
                                 LocalGroupBuffer input_groups,
                                 LocalDataBuffer output_buffer,
                                 LocalHeaderBuffer output_header,
                                 LocalValidBuffer output_valid,
                                 LocalTimestampBuffer output_timestamp,
                                 LocalGroupBuffer output_groups,
                                 size_t time_step_index,
                                 const Timestamp *time_step_ts,
                                 const EventType *message_type,
                                 const int time_domain)
    : time_step_index_(time_step_index),
      input_buffer_(std::move(input_buffer)),
      input_valid_(std::move(input_valid)),
      input_timestamp_(std::move(input_timestamp)),
      output_buffer_(std::move(output_buffer)),
      output_valid_(std::move(output_valid)),
      output_timestamp_(std::move(output_timestamp)),
      component_index_(component_index), timestamp_(time_step_ts), event_type_(message_type),
      input_header_(input_header), output_header_(output_header), input_groups_(input_groups),
      output_groups_(output_groups), time_domain_(time_domain) {

}

Timestamp ComponentBuffer::getTimestamp() const  {
    return *timestamp_;
}

size_t ComponentBuffer::getInputCount() const  {
    return input_buffer_.size();
}

size_t ComponentBuffer::getOutputCount() const  {
    return output_buffer_.size();
}
size_t ComponentBuffer::getComponentIndex() const  {
    return component_index_;
}
size_t ComponentBuffer::getTimeStepIndex() const  {
    return time_step_index_;
}
bool ComponentBuffer::isInputValid(size_t index) const  {
    return *input_valid_[index] == PortState::VALID;
}
void ComponentBuffer::setOutputInvalid(size_t index) const  {
    *output_valid_[index] = PortState::INVALID;
}
EventType ComponentBuffer::getEventType() const  {
    return *event_type_;
}
Timestamp ComponentBuffer::getInputTimestamp(size_t index) const  {
    return *input_timestamp_[index];
}
bool ComponentBuffer::isOutputValid(size_t index) const  {
    return *output_valid_[index] == PortState::VALID;
}
Timestamp ComponentBuffer::getOutputTimestamp(size_t index) const  {
    return *output_timestamp_[index];
}
bool ComponentBuffer::isAllInputValid() const  {

    for (auto *valid : input_valid_) {
        if(*valid != PortState::VALID)
            return false;
    }
    return true;
}
bool ComponentBuffer::isInputGroupValid(int port_group_index, int port_group_instance_index) const  {
    bool result{true};
    for (int port_index = 0; port_index < input_groups_[port_group_index].size; ++port_index) {
        const size_t kIndex = output_groups_[port_group_index].group_offset
            + output_groups_[port_group_index].group_port_count * port_group_instance_index + port_index;

        result = result && isInputValid(kIndex);
    }
    return result;
}
int ComponentBuffer::getTimeDomain() const {
    return time_domain_;
}

}