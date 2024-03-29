/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include <traact/buffer/TimeDomainBuffer.h>

#include <utility>

namespace traact::buffer {

TimeDomainBuffer::TimeDomainBuffer(int time_domain, std::shared_ptr<DataBufferFactory> buffer_factory) : time_domain_{
    time_domain}, buffer_factory_(std::move(buffer_factory)) {

}

void TimeDomainBuffer::init(const component::ComponentGraph &component_graph) {
    using namespace pattern::instance;

    std::vector<std::string> buffer_datatype;
    config_ = component_graph.getTimeDomainConfig(time_domain_);
    auto components = component_graph.getPatternsForTimeDomain(time_domain_);
    // order components as async source first, then sync sources, then the rest with sinks as last, use order of ComponentTypeEnum
    std::vector<component::ComponentGraph::PatternComponentPair>
        ordered_components(components.begin(), components.end());
    std::sort(ordered_components.begin(),
              ordered_components.end(),
              [time_domain = this->time_domain_](const component::ComponentGraph::PatternComponentPair &value_a,
                                                 const component::ComponentGraph::PatternComponentPair &value_b) -> bool {
                  return value_a.first->getComponentType(time_domain) < value_b.first->getComponentType(time_domain);
              });

    // gather all output buffers to allocate memory, input must be connected to an output
    for (const auto &component : ordered_components) {

        std::shared_ptr<PatternInstance> data_comp = component.first;
        if (!data_comp) {
            SPDLOG_WARN("skipping non dataflow pattern : {0}", component.first->instance_id);
            continue;
        }

        if (component.first->getComponentType(time_domain_) == component::ComponentType::ASYNC_SOURCE) {
            count_async_sources_++;
        }
        if (component.first->getComponentType(time_domain_) == component::ComponentType::INTERNAL_SYNC_SOURCE) {
            count_internal_sync_sources_++;
        }

        for (const auto *port : data_comp->getProducerPorts(time_domain_)) {

            buffer_datatype.emplace_back(port->getDataType());
            size_t buffer_index = buffer_datatype.size() - 1;

            port_to_buffer_index_[port->getId()] = buffer_index;

            for (const auto *input_port : port->connectedToPtr()) {
                port_to_buffer_index_[input_port->getId()] = buffer_index;
            }

        }

    }

    // create buffer data
    for (int ringbuffer_index = 0; ringbuffer_index < config_.ringbuffer_size; ++ringbuffer_index) {
        DataBufferPtr time_step_data = buffer_factory_->createBuffer();
        time_step_data->createObjects(buffer_datatype);
        data_buffer_.emplace_back(std::move(time_step_data));
    }

    // create component buffer configs
    int component_index = 0;
    for (const auto &component : ordered_components) {
        std::shared_ptr<PatternInstance> pattern_instance = component.first;

        if (!pattern_instance) {
            continue;
        }

        BufferConfig buffer_config;
        buffer_config.buffer_to_port_output.reserve(pattern_instance->getProducerPorts(time_domain_).size());
        for (const auto *port : pattern_instance->getProducerPorts(time_domain_)) {
            auto global_buffer_index = port_to_buffer_index_[port->getId()];
            buffer_config.buffer_to_port_output.emplace_back(global_buffer_index, port->getPortIndex());
        }
        buffer_config.buffer_to_port_input.reserve(pattern_instance->getConsumerPorts(time_domain_).size());
        for (const auto *port : pattern_instance->getConsumerPorts(time_domain_)) {
            auto global_buffer_index = port_to_buffer_index_[port->getId()];
            buffer_config.buffer_to_port_input.emplace_back(global_buffer_index, port->getPortIndex());
        }

        buffer_config.port_group_config_output.resize(pattern_instance->local_pattern.port_groups.size());
        buffer_config.port_group_config_input.resize(pattern_instance->local_pattern.port_groups.size());
        for (const auto &port_group : pattern_instance->local_pattern.port_groups) {
            auto [input_group_offset, input_group_port_count, input_group_size] = pattern_instance->getPortGroupOffset(
                port_group.group_index, pattern::PortType::CONSUMER, time_domain_);
            buffer_config.port_group_config_input[port_group.group_index].group_offset = input_group_offset;
            buffer_config.port_group_config_input[port_group.group_index].group_port_count = input_group_port_count;
            buffer_config.port_group_config_input[port_group.group_index].size = input_group_size;

            auto [output_group_offset, output_group_port_count, output_group_size] = pattern_instance->getPortGroupOffset(
                port_group.group_index, pattern::PortType::PRODUCER, time_domain_);
            buffer_config.port_group_config_output[port_group.group_index].group_offset = output_group_offset;
            buffer_config.port_group_config_output[port_group.group_index].group_port_count = output_group_port_count;
            buffer_config.port_group_config_output[port_group.group_index].size = output_group_size;
        }
        buffer_config.component_type = component.first->getComponentType(time_domain_);
        buffer_config.instance_id = component.first->instance_id;
        buffer_config_.emplace(component_index, buffer_config);
        component_index++;
    }
    for (int time_step_index = 0; time_step_index < config_.ringbuffer_size; ++time_step_index) {
        // create time domain buffer, source data pointer are set but not used
        time_step_buffer_.emplace_back(std::make_unique<TimeStepBuffer>(time_step_index,
                                                                        data_buffer_[time_step_index]->getData(),
                                                                        data_buffer_[time_step_index]->getHeader(),
                                                                        buffer_config_,
                                                                        time_domain_));
    }

}

TimeStepBuffer &TimeDomainBuffer::getTimeStepBuffer(size_t concurrent_index) {
    return *time_step_buffer_[concurrent_index];
}

int TimeDomainBuffer::getComponentIndex(const std::string &instance_id) {

    for (const auto &index_config_id : buffer_config_) {
        if (index_config_id.second.instance_id == instance_id) {
            return index_config_id.first;
        }
    }

    assert(false);

//        auto result = buffer_config_.find(instance_id);
//        assert(result != buffer_config_.end());
//        return result->second.second;
}

int TimeDomainBuffer::getCountAsyncSources() const {
    return count_async_sources_;
}

int TimeDomainBuffer::getCountInternalSyncSources() const {
    return count_internal_sync_sources_;
}

int TimeDomainBuffer::getCountSources() const {
    return getCountAsyncSources() + getCountInternalSyncSources();
}

void TimeDomainBuffer::cancelAll() {
    for (int ringbuffer_index = 0; ringbuffer_index < config_.ringbuffer_size; ++ringbuffer_index) {
        cancelAllSources(ringbuffer_index, true);
    }
}

void TimeDomainBuffer::cancelAllSources(int ringbuffer_index, bool valid) {
    for (int source_index = 0; source_index < getCountSources(); ++source_index) {
        time_step_buffer_[ringbuffer_index]->getSourceComponentBuffer(source_index)->cancel();
    }
}
}