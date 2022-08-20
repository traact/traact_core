/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_BUILDER_TRAACTTASKFLOWGRAPH_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_BUILDER_TRAACTTASKFLOWGRAPH_H_

#include <taskflow/taskflow.hpp>
#include "traact/component/ComponentGraph.h"
#include "traact/dataflow/graph/task/TraactTask.h"
#include "traact/buffer/TimeDomainBuffer.h"
#include "traact/dataflow/taskflow/TaskFlowInFlowScheduler.h"
#include "traact/dataflow/state/DataflowState.h"

namespace traact::dataflow {


struct TraactTaskFlowGraph {
    tf::Taskflow* taskflow;
    TaskFlowInFlowScheduler* scheduler;
    int time_domain;
    std::shared_ptr<buffer::TimeDomainBuffer> time_domain_buffer;
    std::vector<std::map<std::string, tf::Task>> task_flow_tasks;
    DataflowState::SharedPtr dataflow_state;
};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_BUILDER_TRAACTTASKFLOWGRAPH_H_
