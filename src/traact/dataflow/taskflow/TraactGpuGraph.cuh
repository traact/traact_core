/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TRAACTGPUGRAPH_CUH_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TRAACTGPUGRAPH_CUH_

#include "traact/component/GpuComponent.h"
#include "traact/buffer/TimeStepBuffer.h"
#include <taskflow/taskflow.hpp>
#include "traact/dataflow/taskflow/TaskFlowTaskFunctions.h"

namespace traact {

tf::Task createTraactGpuGraph(const std::string &cuda_graph_name,
                              int time_step_index,
                              tf::Taskflow &taskflow,
                              dataflow::TimeStepData &time_step_data,
                              std::map<std::string, std::shared_ptr<component::GpuComponent>> &components, std::map<std::string, std::set<std::string>>& component_to_successors);



} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TRAACTGPUGRAPH_CUH_
