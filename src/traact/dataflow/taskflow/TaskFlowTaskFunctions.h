/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWTASKFUNCTIONS_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWTASKFUNCTIONS_H_

#include "traact/buffer/ComponentBuffer.h"
#include "traact/component/Component.h"

namespace traact::dataflow {

struct ComponentData {

    ComponentData(buffer::TimeStepBuffer &t_time_step_buffer,
                  buffer::ComponentBuffer &t_buffer,
                  component::Component &t_component,
                  int t_component_index,
                  pattern::instance::PatternInstance &t_pattern_instance,
                  std::atomic<bool> &t_running) : time_step_buffer(t_time_step_buffer), buffer(t_buffer),
                                                  component(t_component), component_index(t_component_index),
                                                  pattern_instance(t_pattern_instance), running(t_running) {}
    ComponentData(ComponentData &&other) : time_step_buffer(other.time_step_buffer),
                                           buffer(other.buffer),
                                           component(other.component),
                                           component_index(other.component_index),
                                           pattern_instance(other.pattern_instance),
                                           running(other.running),
                                           successors_valid(std::move(other.successors_valid)),
                                           valid_component_call(other.valid_component_call.load()) {

    }

    buffer::TimeStepBuffer &time_step_buffer;
    buffer::ComponentBuffer &buffer;
    component::Component &component;
    int component_index;
    pattern::instance::PatternInstance &pattern_instance;
    std::atomic<bool> &running;

    //tf::SmallVector<bool*> valid_input{};
    std::vector<std::atomic_bool *> successors_valid{};
    std::atomic_bool valid_component_call{false};

    TimestampSteady state_last_call{kTimestampSteadyZero};
    EventType state_last_event_type{EventType::INVALID};
    int state_last_event_position;
    bool state_had_error{false};

};

struct TimeStepData {
    std::map<std::string, ComponentData> component_data;
    std::map<std::string, tf::Task> component_id_to_task;
};

inline void taskSource(ComponentData &local_data) {

    try {
        local_data.state_last_call = nowSteady();
        local_data.state_last_event_position = 0;
        SPDLOG_TRACE("SourceComponent: {0} start waiting", local_data.component.getName());
        auto lock = local_data.time_step_buffer.getSourceLock(local_data.component_index);
        auto status = lock.wait_for(kDefaultTimeout);
        while (status != std::future_status::ready && local_data.running) {
            status = lock.wait_for(kDefaultTimeout);
        }
        local_data.state_last_event_position = 1;
        if (status != std::future_status::ready || !local_data.running) {
            //std::atomic_thread_fence(std::memory_order_release);
            local_data.valid_component_call.store(true, std::memory_order_release);
            SPDLOG_TRACE("SourceComponent: {0} invalid waiting or not running", local_data.component.getName());
            return;
        }
        local_data.state_last_event_position = 2;
        SPDLOG_TRACE("SourceComponent: {0} ts: {1} {2} start", local_data.component.getName(),
                     local_data.time_step_buffer.getTimestamp(), local_data.time_step_buffer.getEventType());

        bool component_result{false};

        local_data.state_last_event_type = local_data.time_step_buffer.getEventType();
        switch (local_data.time_step_buffer.getEventType()) {

            case EventType::CONFIGURE: {
                component_result = local_data.component.configure(local_data.pattern_instance, nullptr);
                break;
            }

            case EventType::START: {
                component_result = local_data.component.start();
                break;
            }

            case EventType::DATA: {
                component_result = local_data.valid_component_call = lock.get();
                break;
            }
            case EventType::STOP: {
                component_result = local_data.component.stop();
                break;
            }

            case EventType::TEARDOWN: {
                component_result = local_data.component.teardown();
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

        SPDLOG_TRACE("SourceComponent: {0} ts: {1} {2} done", local_data.component.getName(),
                     local_data.time_step_buffer.getTimestamp(), local_data.time_step_buffer.getEventType());

        //std::atomic_thread_fence(std::memory_order_release);
        //local_data.valid_component_call.store(component_result, std::memory_order_release);
        // must be true right now or renderer will stop
        local_data.valid_component_call.store(true, std::memory_order_release);
        local_data.state_last_event_position = 3;
        return;
    } catch(std::exception &e) {
        SPDLOG_ERROR("{0}, components must not throw exceptions ", e.what());
    } catch (...) {
        SPDLOG_ERROR("unknown throw in source component, components must not throw exceptions");
    }
    local_data.valid_component_call.store(true, std::memory_order_release);
    local_data.state_had_error = true;
    local_data.state_last_event_position = 4;
}

inline void taskGenericComponent(ComponentData &local_data) {
    try {
        local_data.state_last_call = nowSteady();
        local_data.state_last_event_position = 0;
        SPDLOG_TRACE("Component {0}: start ts {1} {2}",
                     local_data.component.getName(),
                     local_data.buffer.getTimestamp(),
                     local_data.time_step_buffer.getEventType());

        bool all_input_valid = true;

//        for (auto *valid : local_data.successors_valid) {
//            bool local_valid = std::atomic_load_explicit(valid, std::memory_order_acquire);
//            //std::atomic_thread_fence(std::memory_order_acquire);
//            all_input_valid = all_input_valid && local_valid;
//        }
//
//        if (!all_input_valid) {
//            SPDLOG_TRACE("{0}: abort ts {1}, message type {2}, successors call to component returned false",
//                         local_data.component.getName(),
//                         local_data.buffer.getTimestamp(),
//                         local_data.time_step_buffer.getEventType());
//            //std::atomic_thread_fence(std::memory_order_release);
//            local_data.valid_component_call.store(true, std::memory_order_release);
//            return;
//        }

        local_data.state_last_event_type =local_data.time_step_buffer.getEventType();
        bool component_result{false};
        switch (local_data.time_step_buffer.getEventType()) {
            case EventType::CONFIGURE: {
                component_result = local_data.component.configure(local_data.pattern_instance, nullptr);
                break;
            }
            case EventType::START: {
                component_result = local_data.component.start();
                break;
            }
            case EventType::DATA: {

                if (local_data.buffer.isAllInputValid()) {
                    component_result = local_data.component.processTimePoint(local_data.buffer);
                } else {
                    component_result = local_data.component.processTimePointWithInvalid(local_data.buffer);
                }

                break;
            }
            case EventType::STOP: {
                component_result = local_data.component.stop();
                break;
            }

            case EventType::TEARDOWN: {
                component_result = local_data.component.teardown();
                break;
            }

            case EventType::DATAFLOW_NO_OP:
            case EventType::DATAFLOW_STOP:break;
            case EventType::INVALID:assert(!"Invalid MessageType");
            default:break;
        }

        SPDLOG_TRACE("Component {0}: finished ts {1} {2}",
                     local_data.component.getName(),
                     local_data.buffer.getTimestamp(),
                     local_data.time_step_buffer.getEventType());

        //std::atomic_thread_fence(std::memory_order_release);
        //local_data.valid_component_call.store(component_result, std::memory_order_release);
        // must be true right now or renderer will stop
        local_data.valid_component_call.store(true, std::memory_order_release);
        local_data.state_last_event_position = 3;
        return;
    } catch (const std::exception& e) {
        SPDLOG_ERROR("{0}, {1}, components must not throw exceptions ", e.what(),local_data.component.getName());
    } catch (...) {
        SPDLOG_ERROR("unknown throw in source component, components must not throw exceptions");
    }
    local_data.valid_component_call.store(true, std::memory_order_release);
    local_data.state_had_error = true;
    local_data.state_last_event_position = 4;
}
}

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWTASKFUNCTIONS_H_
