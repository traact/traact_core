/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TraactGpuGraph.cuh"
#include <taskflow/cuda/cudaflow.hpp>
#include "traact/dataflow/graph/task/TraactTaskId.h"


namespace traact {

tf::Task createTraactGpuGraph(const std::string &cuda_graph_name,
                              int time_step_index,
                              tf::Taskflow &taskflow,
                              const std::vector<buffer::ComponentBuffer*> &component_buffer,
                              const std::vector<std::shared_ptr<component::CudaComponent>> &components,
                              const std::vector<std::vector<int>>& component_to_successors) {

    return taskflow.emplace([cuda_graph_name, time_step_index, buffer = component_buffer, components, component_to_successors](tf::cudaFlowCapturer &capturer) {
        std::vector<tf::cudaTask> component_to_task;
        component_to_task.resize(components.size());
        for(auto i=0;i<components.size();++i){
            const auto& component = components[i];
            component_to_task[i] = capturer.on(component->createGpuTask(buffer[i]))
                .name(dataflow::task_util::getTaskId(dataflow::task_util::kCudaComponent, component->getName().c_str(), time_step_index));

        }
        for(auto i=0;i<components.size();++i){
            auto& current_task = component_to_task[i];
            for(auto successor_index : component_to_successors[i]){
                current_task.precede(component_to_task[successor_index]);
            }
        }
    }).name(dataflow::task_util::getTaskId(dataflow::task_util::kCudaFlow, cuda_graph_name.c_str(), time_step_index));
}

} // traact