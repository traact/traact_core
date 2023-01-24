/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TASK_TRAACTTASKDATA_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TASK_TRAACTTASKDATA_H_

#include <memory>
#include "traact/buffer/TimeStepBuffer.h"

namespace traact::dataflow {

struct TraactTaskData {
     using SharedPtr = std::shared_ptr<TraactTaskData>;

    TraactTaskData(buffer::TimeStepBuffer &time_step_buffer,
                   buffer::ComponentBuffer &buffer,
                   int component_index);

    buffer::TimeStepBuffer &time_step_buffer;
    buffer::ComponentBuffer &buffer;
    int component_index;

    std::vector<std::atomic_bool *> predecessors_valid{};
    std::atomic_bool valid_component_call{false};
};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TASK_TRAACTTASKDATA_H_
