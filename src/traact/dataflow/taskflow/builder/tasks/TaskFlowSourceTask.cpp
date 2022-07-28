/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TaskFlowSourceTask.h"

namespace traact::dataflow::taskflow {
TaskFlowSourceTask::TaskFlowSourceTask(std::shared_ptr<traact::dataflow::SourceTask> task,
                                             std::shared_ptr<TraactTaskData> data) : pattern_(task->getPattern()), component_(task->getComponent()), data_(std::move(data)) {




}

void TaskFlowSourceTask::process() {

    try {
        data_->task_state.beginWaiting();


        auto lock = data_->time_step_buffer.getSourceLock(data_->component_index);
        auto status = lock.wait_for(kDefaultTimeout);
        while (status != std::future_status::ready) {
            status = lock.wait_for(kDefaultTimeout);
        }
        data_->task_state.begin();

        if (status != std::future_status::ready) {
            data_->valid_component_call.store(true, std::memory_order_release);
            SPDLOG_TRACE("SourceComponent: {0} invalid waiting or not running", component_->getName());
            return;
        }

        SPDLOG_TRACE("SourceComponent: {0} ts: {1} {2} start", component_->getName(),
                     data_->time_step_buffer.getTimestamp(), data_->time_step_buffer.getEventType());

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
                component_result = lock.get();
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