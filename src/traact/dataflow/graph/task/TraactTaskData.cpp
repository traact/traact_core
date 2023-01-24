/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TraactTaskData.h"

namespace traact {
dataflow::TraactTaskData::TraactTaskData(buffer::TimeStepBuffer &time_step_buffer,
                                         buffer::ComponentBuffer &buffer,
                                         int component_index)
    : time_step_buffer(time_step_buffer), buffer(buffer), component_index(component_index) {}
} // traact