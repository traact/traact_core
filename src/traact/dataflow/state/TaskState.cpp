/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TaskState.h"

namespace traact::dataflow {
void TaskState::beginWaiting() {
    //SPDLOG_TRACE("SourceComponent: {0} start waiting", local_data.component.getName());

}
void TaskState::begin() {
//    SPDLOG_TRACE("Component {0}: start ts {1} {2}",
//                 local_data.component.getName(),
//                 local_data.buffer.getTimestamp(),
//                 local_data.time_step_buffer.getEventType());
}
void TaskState::process(EventType event_type) {

}
void TaskState::end() {
//    SPDLOG_TRACE("Component {0}: finished ts {1} {2}",
//                 local_data.component.getName(),
//                 local_data.buffer.getTimestamp(),
//                 local_data.time_step_buffer.getEventType());
}
void TaskState::error() {

}
void TaskState::error(std::string error) {

}
} // traact