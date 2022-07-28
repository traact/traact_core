/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TraactTaskData.h"

namespace traact {
dataflow::TraactTaskData::TraactTaskData(buffer::TimeStepBuffer &time_step_buffer,
                                         buffer::ComponentBuffer &buffer,
                                         dataflow::TaskState &task_state,
                                         int component_index)
    : time_step_buffer(time_step_buffer), buffer(buffer), task_state(task_state), component_index(component_index) {}
} // traact