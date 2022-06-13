/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

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
    std::vector<bool *> successors_valid{};
    bool valid_component_call{false};

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
        local_data.valid_component_call = false;
        return;
    }

    switch (local_data.time_step_buffer.getEventType()) {

        case EventType::CONFIGURE: {
            local_data.valid_component_call = local_data.component.configure(local_data.component_parameter, nullptr);
            break;
        }

        case EventType::START: {
            local_data.valid_component_call = local_data.component.start();
            break;
        }

        case EventType::DATA: {
            local_data.valid_component_call = local_data.valid_component_call = lock.get();
            break;
        }
        case EventType::STOP: {
            local_data.valid_component_call = local_data.component.stop();
            break;
        }

        case EventType::TEARDOWN: {
            local_data.valid_component_call = local_data.component.teardown();
            break;
        }

        case EventType::DATAFLOW_NO_OP:
        case EventType::DATAFLOW_STOP: {
            break;
        }
        case EventType::INVALID:
        default: {
            assert(!"Invalid MessageType");
        }

    }

    SPDLOG_TRACE("SourceComponent: {0} ts: {1} done", local_data.component.getName(),
                 local_data.time_step_buffer.getTimestamp());
}

inline void taskGenericComponent(ComponentData &local_data) {
    SPDLOG_TRACE("Component {0}: start ts: {1}",
                 local_data.component.getName(),
                 local_data.buffer.getTimestamp());

    for (auto *valid : local_data.successors_valid) {
        if (*valid == false) {
            SPDLOG_TRACE("{0}: abort ts {1}, message type {2}, successors call to component returned false",
                         local_data.component.getName(),
                         local_data.buffer.getTimestamp(),
                         local_data.time_step_buffer.getEventType());
            local_data.valid_component_call = false;
            return;
        }
    }

    switch (local_data.time_step_buffer.getEventType()) {
        case EventType::CONFIGURE: {
            local_data.valid_component_call = local_data.component.configure(local_data.component_parameter, nullptr);
            break;
        }
        case EventType::START: {
            local_data.valid_component_call = local_data.component.start();
            break;
        }
        case EventType::DATA: {

            if (local_data.buffer.isAllInputValid()) {
                local_data.valid_component_call = local_data.component.processTimePoint(local_data.buffer);
            } else {
                local_data.valid_component_call = local_data.component.processTimePointWithInvalid(local_data.buffer);
            }

            break;
        }
        case EventType::STOP: {
            local_data.valid_component_call = local_data.component.stop();
            break;
        }

        case EventType::TEARDOWN: {
            local_data.valid_component_call = local_data.component.teardown();
            break;
        }

        case EventType::DATAFLOW_NO_OP:
        case EventType::DATAFLOW_STOP:break;
        case EventType::INVALID:assert(!"Invalid MessageType");
        default:break;
    }

    SPDLOG_TRACE("Component {0}: finished ts {1}",
                 local_data.component.getName(),
                 local_data.buffer.getTimestamp());
}
}

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWTASKFUNCTIONS_H_
