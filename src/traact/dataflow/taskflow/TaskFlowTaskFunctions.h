/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWTASKFUNCTIONS_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWTASKFUNCTIONS_H_

#include "traact/buffer/ComponentBuffer.h"
#include "traact/component/Component.h"

namespace traact::dataflow {

struct ComponentData {

    ComponentData(buffer::TimeStepBuffer &time_step_buffer, buffer::ComponentBuffer &buffer,
                  component::Component &component, int component_index, nlohmann::json &component_parameter,
                  std::atomic<bool> &running) : time_step_buffer(time_step_buffer), buffer(buffer),
                                                component(component), component_index(component_index),
                                                component_parameter(component_parameter), running(running) {}

    buffer::TimeStepBuffer &time_step_buffer;
    buffer::ComponentBuffer &buffer;
    component::Component &component;
    int component_index;
    nlohmann::json &component_parameter;
    std::atomic<bool> &running;

    //tf::SmallVector<bool*> valid_input{};
    std::vector<bool *> valid_input{};
    bool valid_output{false};

};

struct TimeStepData {
    std::map<std::string, ComponentData> component_data;
    std::map<std::string, tf::Task> component_id_to_task;
};

inline void taskSource(ComponentData &local_data) {

    SPDLOG_TRACE("SourceComponent: {0} start waiting", local_data.component.getName());
    auto lock = local_data.time_step_buffer.getSourceLock(local_data.component_index);
    auto status = lock.wait_for(kDefaultTimeout);
    while (status != std::future_status::ready && local_data.running) {
        status = lock.wait_for(kDefaultTimeout);
    }
    if (status != std::future_status::ready || !local_data.running) {
        local_data.valid_output = false;
        return;
    }

    local_data.valid_output = lock.get();

    switch (local_data.time_step_buffer.getEventType()) {

        case EventType::CONFIGURE:local_data.component.configure(local_data.component_parameter, nullptr);
            break;
        case EventType::START:local_data.component.start();
            break;
        case EventType::DATA:break;
        case EventType::STOP:local_data.component.stop();
            break;
        case EventType::TEARDOWN:local_data.component.teardown();
            break;
        case EventType::DATAFLOW_NO_OP:
        case EventType::DATAFLOW_STOP:break;
        case EventType::INVALID:assert(!"Invalid MessageType");
        default:break;
    }

    SPDLOG_TRACE("SourceComponent: {0} ts: {1}", local_data.component.getName(),
                 local_data.time_step_buffer.getTimestamp());
}

inline void taskGenericComponent(ComponentData &local_data) {
    SPDLOG_TRACE("Component {0}: start ts: {1}",
                 local_data.component.getName(),
                 local_data.buffer.getTimestamp());

    switch (local_data.time_step_buffer.getEventType()) {
        case EventType::CONFIGURE:local_data.component.configure(local_data.component_parameter, nullptr);
            break;
        case EventType::START:local_data.component.start();
            break;
        case EventType::DATA: {
            bool valid_input = true;
            for (auto *valid : local_data.valid_input) {
                valid_input &= *valid;
            }

            if (valid_input) {
                local_data.valid_output = local_data.component.processTimePoint(local_data.buffer);
            } else {
                local_data.component.invalidTimePoint(local_data.buffer.getTimestamp(), 0);
                local_data.valid_output = false;
            }

            break;
        }
        case EventType::STOP:local_data.component.stop();
            break;
        case EventType::TEARDOWN:local_data.component.teardown();
            break;
        case EventType::DATAFLOW_NO_OP:
        case EventType::DATAFLOW_STOP:break;
        case EventType::INVALID:assert(!"Invalid MessageType");
        default:
            break;
    }

    SPDLOG_TRACE("{0}: finished ts {1}",
                 local_data.component.getName(),
                 local_data.buffer.getTimestamp());
}
}

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWTASKFUNCTIONS_H_
