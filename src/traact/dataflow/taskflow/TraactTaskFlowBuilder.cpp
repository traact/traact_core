/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TraactTaskFlowBuilder.h"

#include "builder/TaskFlowStandardTasks.h"
#include "builder/TaskFlowTimeStep.h"
#include "builder/TaskFlowConnections.h"
#include "traact/dataflow/graph/cuda/TaskFlowCudaGraphBuilder.h"

namespace traact {
dataflow::TraactTaskFlowBuilder::TraactTaskFlowBuilder(tf::Taskflow *taskflow,
                                                       std::shared_ptr<buffer::TimeDomainBuffer> time_domain_buffer,
                                                       int time_domain,
                                                       TaskFlowInFlowScheduler *scheduler) {
    graph_.taskflow = taskflow;
    graph_.time_domain_buffer = time_domain_buffer;
    graph_.time_domain = time_domain;
    graph_.scheduler = scheduler;

    builder_.emplace_back(std::make_unique<TaskFlowStandardTasks>(graph_));
    builder_.emplace_back(std::make_unique<TaskFlowCudaGraphBuilder>(graph_));
    builder_.emplace_back(std::make_unique<TaskFlowTimeStep>(graph_));
    builder_.emplace_back(std::make_unique<TaskFlowConnections>(graph_));

}

void dataflow::TraactTaskFlowBuilder::build(const TraactGraph::SharedPtr& traact_graph) {
    graph_.dataflow_state = traact_graph->dataflow_state;
    for(auto& builder : builder_){
        builder->buildGraph(traact_graph);
    }
}

} // traact