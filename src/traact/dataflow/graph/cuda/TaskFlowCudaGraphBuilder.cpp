/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TaskFlowCudaGraphBuilder.h"
#include "TraactGpuGraph.cuh"

namespace traact::dataflow {
TaskFlowCudaGraphBuilder::TaskFlowCudaGraphBuilder(TraactTaskFlowGraph &task_flow_graph) : TaskFlowGraphBuilder(
    task_flow_graph) {}
void TaskFlowCudaGraphBuilder::buildGraph(const TraactGraph::SharedPtr &traact_graph) {
    for(auto time_step = 0; time_step < traact_graph->getTimeStepCount(); ++time_step){
        createGpuTimeStepTasks(traact_graph, time_step);
    }

}

void TaskFlowCudaGraphBuilder::createGpuTimeStepTasks(const TraactGraph::SharedPtr &traact_graph,
                                                      const int time_step_index) {
    auto &task_flow_tasks = task_flow_graph_.task_flow_tasks.at(time_step_index);
    for (const auto &[id, traact_task] : traact_graph->tasks) {
        const auto traact_cuda_task = std::dynamic_pointer_cast<TraactCudaTask>(traact_task);
        if(traact_cuda_task){
            tf::Task cuda_flow = createCudaFlow(traact_graph, time_step_index, traact_cuda_task);
            task_flow_tasks.emplace(id, cuda_flow);
        }
    }
}
tf::Task TaskFlowCudaGraphBuilder::createCudaFlow(const TraactGraph::SharedPtr &traact_graph,
                                                  int time_step_index,
                                                  const std::shared_ptr<TraactCudaTask> &cuda_task) {

    std::vector<buffer::ComponentBuffer*> component_buffer;
    std::vector<std::shared_ptr<component::CudaComponent>> components;
    std::vector<std::vector<int>> component_to_successors;

    auto component_count = cuda_task->getCudaComponents().size();
    component_buffer.resize(component_count);
    components.resize(component_count);
    component_to_successors.resize(component_count);

    size_t component_index = 0;
    std::map<std::string, size_t > id_to_index;
    for(const auto& [id, component] : cuda_task->getCudaComponents()){
        components[component_index] = component;

        auto data = traact_graph->task_data.at(time_step_index)[id];
        component_buffer[component_index] = &data->buffer;
        id_to_index.emplace(id, component_index);
        component_index++;
    }
    for(const auto& [id, successors] : cuda_task->getComponentToSuccessors()){
        auto& task_successors = component_to_successors[id_to_index.at(id)];
        task_successors.reserve(successors.size());
        for(const auto& successor : successors) {
            task_successors.emplace_back(id_to_index.at(successor));
        }
    }

    return createTraactGpuGraph(cuda_task->getCudaGraphName(),
                                time_step_index,
                                *task_flow_graph_.taskflow,
                                component_buffer,
                                components,
                                component_to_successors);
}

} // traact