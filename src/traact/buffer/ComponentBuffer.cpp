/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#include "ComponentBuffer.h"

#include <utility>

namespace traact::buffer {
ComponentBuffer::ComponentBuffer(size_t component_index,
                                 LocalDataBuffer input_buffer,
                                 LocalValidBuffer input_valid,
                                 LocalTimestampBuffer input_timestamp,
                                 LocalDataBuffer output_buffer,
                                 LocalValidBuffer output_valid,
                                 LocalTimestampBuffer output_timestamp,
                                 size_t time_step_index,
                                 const Timestamp *time_step_ts,
                                 const EventType *message_type)
    : time_step_index_(time_step_index),
      input_buffer_(std::move(input_buffer)),
      input_valid_(std::move(input_valid)),
      input_timestamp_(std::move(input_timestamp)),
      output_buffer_(std::move(output_buffer)),
      output_valid_(std::move(output_valid)),
      output_timestamp_(std::move(output_timestamp)),
      component_index_(component_index), timestamp_(time_step_ts), event_type_(message_type) {

}

Timestamp ComponentBuffer::getTimestamp() const noexcept {
    return *timestamp_;
}

size_t ComponentBuffer::getInputCount() const noexcept {
    return input_buffer_.size();
}

size_t ComponentBuffer::getOutputCount() const noexcept {
    return output_buffer_.size();
}
size_t ComponentBuffer::getComponentIndex() const noexcept {
    return component_index_;
}
size_t ComponentBuffer::getTimeStepIndex() const noexcept {
    return time_step_index_;
}
bool ComponentBuffer::isInputValid(size_t index) const noexcept {
    return *input_valid_[index] == PortState::VALID;
}
void ComponentBuffer::setOutputInvalid(size_t index) const noexcept {
    *output_valid_[index] = PortState::INVALID;
}
EventType ComponentBuffer::getEventType() const noexcept {
    return *event_type_;
}
Timestamp ComponentBuffer::getInputTimestamp(size_t index) const noexcept {
    return *input_timestamp_[index];
}
bool ComponentBuffer::isOutputValid(size_t index) const noexcept {
    return *output_valid_[index] == PortState::VALID;
}
Timestamp ComponentBuffer::getOutputTimestamp(size_t index) const noexcept {
    return *output_timestamp_[index];
}
}