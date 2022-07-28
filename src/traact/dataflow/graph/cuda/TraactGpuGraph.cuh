/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_CUDA_TRAACTGPUGRAPH_CUH_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_CUDA_TRAACTGPUGRAPH_CUH_

#include "traact/component/CudaComponent.h"
#include "traact/buffer/TimeStepBuffer.h"
#include <taskflow/taskflow.hpp>
#include "traact/dataflow/taskflow/TaskFlowTaskFunctions.h"

namespace traact {

tf::Task createTraactGpuGraph(const std::string &cuda_graph_name,
                              int time_step_index,
                              tf::Taskflow &taskflow,
                              const std::vector<buffer::ComponentBuffer*> &component_buffer,
                              const std::vector<std::shared_ptr<component::CudaComponent>> &components,
                              const std::vector<std::vector<int>>& component_to_successors);



} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_CUDA_TRAACTGPUGRAPH_CUH_
