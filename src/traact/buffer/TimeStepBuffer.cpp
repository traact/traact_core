/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include <traact/buffer/TimeStepBuffer.h>
#include <fmt/format.h>

namespace traact::buffer {
TimeStepBuffer::TimeStepBuffer(size_t time_step_index,
                               BufferType buffer_data,
                               BufferType header_data,
                               const std::map<int,
                                              BufferConfig> &buffer_config,
                               const int time_domain)
    : time_step_index_(time_step_index),
      buffer_data_(std::move(buffer_data)),
      header_data_(std::move(header_data)),
      current_message_{EventType::INVALID}, time_domain_(time_domain) {

    buffer_timestamp_.resize(buffer_data_.size());
    buffer_valid_.reserve(buffer_data_.size());
    for (size_t i = 0; i < buffer_data_.size(); ++i) {
        buffer_valid_.emplace_back(std::make_unique<PortState>(PortState::INVALID));
        //buffer_valid_.emplace_back(PortState::INVALID);
    }

    int component_count = 0;
    for (const auto &component_config : buffer_config) {
        auto component_index = component_config.first;
        const auto &instance_id = component_config.second.instance_id;

        if (component_count != component_index) {
            throw std::invalid_argument("Component ids of TimeStepBufferConfig must start at 0 and be continuous");
        }

        component_buffers_list_.emplace_back(createComponentBuffer(component_config.second, component_index));

        ++component_count;
        component_buffer_to_index_.emplace(instance_id, component_index);

        switch (component_config.second.component_type) {
            // add source components to source buffer list
            case component::ComponentType::ASYNC_SOURCE:
            case component::ComponentType::INTERNAL_SYNC_SOURCE: {
                // since map is ordered by component index and components are ordered by sources first no need to allocate the source_buffer_list_ for all components as potential sources for "by index" access
                source_buffer_list_.emplace_back(std::make_unique<SourceComponentBuffer>(*component_buffers_list_[component_index]));
                break;
            }
                // nothing to do for other component types
            case component::ComponentType::SYNC_SOURCE:
            case component::ComponentType::SYNC_FUNCTIONAL:
            case component::ComponentType::ASYNC_FUNCTIONAL:
            case component::ComponentType::SYNC_SINK:
            case component::ComponentType::ASYNC_SINK:
            case component::ComponentType::INVALID:break;
            default:assert(!"Invalid ComponentType for TimeStepBuffer");

        }

    }
}
void TimeStepBuffer::createLocalBuffer(const std::vector<std::pair<int, int>> &port_inputs,
                                       LocalDataBuffer &data_buffer,
                                       LocalHeaderBuffer &header_buffer,
                                       LocalValidBuffer &valid_buffer,
                                       LocalTimestampBuffer &timestamp_buffer) {
    data_buffer.resize(port_inputs.size());
    header_buffer.resize(port_inputs.size());
    valid_buffer.resize(port_inputs.size());
    timestamp_buffer.resize(port_inputs.size());
    for (auto port : port_inputs) {
        data_buffer[port.second] = buffer_data_[port.first];
        header_buffer[port.second] = header_data_[port.first];
        timestamp_buffer[port.second] = &buffer_timestamp_[port.first];
        valid_buffer[port.second] = buffer_valid_[port.first].get();
    }
}

size_t TimeStepBuffer::getComponentIndex(const std::string &component_name) {
    auto result = component_buffer_to_index_.find(component_name);
    if (result != component_buffer_to_index_.end()) {
        return result->second;
    }
    throw std::invalid_argument(fmt::format("No Component Buffer for Component {0}", component_name));
}

ComponentBuffer &TimeStepBuffer::getComponentBuffer(size_t component_idx) {
    return *component_buffers_list_[component_idx];
}

void TimeStepBuffer::setEvent(Timestamp timestamp, EventType message_type) {
    current_ts_ = timestamp;
    current_message_ = message_type;
    changed_pattern_instance_id_.clear();
}

traact::Timestamp TimeStepBuffer::getTimestamp() {
    return current_ts_;
}

SourceComponentBuffer *TimeStepBuffer::getSourceComponentBuffer(size_t component_idx) {
    return source_buffer_list_[component_idx].get();
}

ComponentBuffer &TimeStepBuffer::getComponentBuffer(const std::string &component_name) {
    return getComponentBuffer(getComponentIndex(component_name));
}

std::future<bool> TimeStepBuffer::getSourceLock(size_t component_idx) {
    return source_buffer_list_[component_idx]->getSourceLock();
}

void TimeStepBuffer::resetNewEvent() {
    for (auto &comp_buffer : source_buffer_list_) {
        comp_buffer->resetLock();
    }
    for (auto &buffer_valid : buffer_valid_) {
        *buffer_valid = PortState::INVALID;
    }
    changed_pattern_instance_id_.clear();
}

EventType TimeStepBuffer::getEventType() {
    return current_message_;
}
std::unique_ptr<ComponentBuffer> TimeStepBuffer::createComponentBuffer(const BufferConfig &config,
                                                                       int component_index) {
    const auto &port_inputs = config.buffer_to_port_input;
    const auto &port_outputs = config.buffer_to_port_output;

    LocalDataBuffer input_data_buffer;
    LocalHeaderBuffer input_header_buffer;
    LocalValidBuffer input_valid_buffer;
    LocalTimestampBuffer input_timestamp_buffer;
    LocalGroupBuffer input_group_buffer;

    LocalDataBuffer output_data_buffer;
    LocalHeaderBuffer output_header_buffer;
    LocalValidBuffer output_valid_buffer;
    LocalTimestampBuffer output_timestamp_buffer;
    LocalGroupBuffer output_group_buffer;


    createLocalBuffer(port_inputs,
                      input_data_buffer,
                      input_header_buffer,
                      input_valid_buffer,
                      input_timestamp_buffer);
    std::copy(config.port_group_config_output.begin(),
              config.port_group_config_output.end(),
              std::back_inserter(output_group_buffer));

    createLocalBuffer(port_outputs,
                      output_data_buffer,
                      output_header_buffer,
                      output_valid_buffer,
                      output_timestamp_buffer);
    std::copy(config.port_group_config_input.begin(),
              config.port_group_config_input.end(),
              std::back_inserter(input_group_buffer));
    return std::make_unique<ComponentBuffer>(component_index,
                                             input_data_buffer,
                                             input_header_buffer,
                                             input_valid_buffer,
                                             input_timestamp_buffer,
                                             input_group_buffer,
                                             output_data_buffer,
                                             output_header_buffer,
                                             output_valid_buffer,
                                             output_timestamp_buffer,
                                             output_group_buffer,
                                             time_step_index_,
                                             &current_ts_,
                                             &current_message_,
                                             time_domain_);
}
void TimeStepBuffer::setEvent(Timestamp timestamp, EventType message_type, std::string changed_pattern) {
    current_ts_ = timestamp;
    current_message_ = message_type;
    changed_pattern_instance_id_ = changed_pattern;
}
const std::string &TimeStepBuffer::getChangedPattern() const {
    return changed_pattern_instance_id_;
}
bool TimeStepBuffer::hasComponentBuffer(const std::string &component_name) {
    auto result = component_buffer_to_index_.find(component_name);
    return result != component_buffer_to_index_.end();
}
int TimeStepBuffer::getTimeDomain() const {
    return time_domain_;
}
}


