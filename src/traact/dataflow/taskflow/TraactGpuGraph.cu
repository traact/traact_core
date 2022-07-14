/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TraactGpuGraph.cuh"
#include <taskflow/cuda/cudaflow.hpp>
#include <fmt/format.h>



namespace traact {

tf::Task createTraactGpuGraph(const std::string &cuda_graph_name,
                              int time_step_index,
                              tf::Taskflow &taskflow,
                              dataflow::TimeStepData &time_step_data,
                              std::map<std::string, std::shared_ptr<component::GpuComponent>> &components,
                              std::map<std::string, std::set<std::string>>& component_to_successors) {

    return taskflow.emplace([cuda_graph_name, time_step_index, &step_data = time_step_data, &components, &component_to_successors](tf::cudaFlowCapturer &capturer) {
        std::map<std::string, tf::cudaTask> component_to_task;
        for (auto &[instance_id, component] : components) {
            auto& component_data = step_data.component_data.at(instance_id);


            auto task = capturer.on(component->createGpuTask(&component_data.buffer))
                .name(fmt::format("{0}_task{1}", instance_id, time_step_index));

            component_to_task.emplace(instance_id, task);
        }
        for (auto &[instance_id, task] : component_to_task) {
            const auto& successors = component_to_successors.at(instance_id);
            for(const auto& successor : successors){
                task.precede(component_to_task.at(successor));
            }

        }

    }).name(fmt::format("{0}_task{1}", cuda_graph_name, time_step_index)
    );
}
} // traact