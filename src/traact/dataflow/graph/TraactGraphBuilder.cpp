/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TraactGraphBuilder.h"

#include <utility>

#include "builder/ComponentGraphBuilder.h"
#include "builder/ModuleGraphBuilder.h"
#include "traact/dataflow/taskflow/builder/TaskFlowTimeStep.h"
#include "traact/dataflow/taskflow/builder/TaskFlowConnections.h"

#include "traact/dataflow/graph/cuda/CudaGraphBuilder.h"

namespace traact::dataflow {
TraactGraphBuilder::TraactGraphBuilder(GraphBuilderConfig config)
    : config_(std::move(config)) {

    graph_builder_.emplace_back(std::make_unique<ComponentGraphBuilder>(config_));
    graph_builder_.emplace_back(std::make_unique<ModuleGraphBuilder>(config_));
    graph_builder_.emplace_back(std::make_unique<CudaGraphBuilder>(config_));




}
TraactGraph::SharedPtr TraactGraphBuilder::build() {

    traact_graph_ = std::make_shared<TraactGraph>();

    for(auto& builder : graph_builder_){
        builder->buildGraph(*traact_graph_);
    }

    createTimeStepTraactTaskData();

    return traact_graph_;

}
void TraactGraphBuilder::createTimeStepTraactTaskData() {
    auto time_step_count = config_.config.ringbuffer_size;
    traact_graph_->task_data.resize(time_step_count);

    for(int i=0;i< time_step_count;++i){
        auto& time_step_data = traact_graph_->task_data[i];
        auto& time_step_buffer = config_.time_domain_buffer->getTimeStepBuffer(i);

        for(const auto&[id, task] : traact_graph_->tasks){
            auto component_index = time_step_buffer.getComponentIndex(id);
            auto& component_buffer = time_step_buffer.getComponentBuffer(component_index);
            TaskState& task_state = traact_graph_->dataflow_state->getTaskState(id);
            auto task_data = std::make_shared<TraactTaskData>(time_step_buffer, component_buffer, task_state, component_index );
            time_step_data.emplace(id, task_data);
        }
    }
}
} // traact