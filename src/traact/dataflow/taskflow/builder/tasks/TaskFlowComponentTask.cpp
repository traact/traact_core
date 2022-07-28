/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TaskFlowComponentTask.h"

namespace traact::dataflow::taskflow {
TaskFlowComponentTask::TaskFlowComponentTask(std::shared_ptr<traact::dataflow::ComponentTask> task,
                                             std::shared_ptr<TraactTaskData> data) : pattern_(task->getPattern()), component_(task->getComponent()), data_(std::move(data)) {




}

void TaskFlowComponentTask::process() {

    try {
        data_->task_state.begin();



        bool all_input_valid = true;

        for (auto *valid : data_->predecessors_valid) {
            bool local_valid = std::atomic_load_explicit(valid, std::memory_order_acquire);
            all_input_valid = all_input_valid && local_valid;
        }
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

        auto event_type =data_->time_step_buffer.getEventType();
        data_->task_state.process(event_type);
        bool component_result{false};
        switch (event_type) {
            case EventType::CONFIGURE: {
                component_result = component_->configure(*pattern_, nullptr);
                break;
            }
            case EventType::START: {
                component_result = component_->start();
                break;
            }
            case EventType::DATA: {

                if (data_->buffer.isAllInputValid()) {
                    component_result = component_->processTimePoint(data_->buffer);
                } else {
                    component_result = component_->processTimePointWithInvalid(data_->buffer);
                }

                break;
            }
            case EventType::PARAMETER_CHANGE:{
                if(data_->time_step_buffer.getChangedPattern() == pattern_->instance_id){
                    component_result = component_->configure(*pattern_, nullptr);
                }
                break;
            }
            case EventType::STOP: {
                component_result = component_->stop();
                break;
            }

            case EventType::TEARDOWN: {
                component_result = component_->teardown();
                break;
            }

            case EventType::DATAFLOW_NO_OP:
            case EventType::DATAFLOW_STOP:break;
            case EventType::INVALID:assert(!"Invalid MessageType");
            default:break;
        }

        data_->task_state.end();
        // must be true right now or renderer will stop
        data_->valid_component_call.store(true, std::memory_order_release);
        return;
    } catch (const std::exception& e) {
        data_->task_state.error(e.what());
        SPDLOG_ERROR("{0}, {1}, components must not throw exceptions ", e.what(),pattern_->instance_id);
    } catch (...) {
        data_->task_state.error();
        SPDLOG_ERROR("unknown throw in source component, components must not throw exceptions");
    }

    data_->valid_component_call.store(true, std::memory_order_release);


}
} // traact