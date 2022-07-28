/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_CUDA_TASKFLOWCUDAGRAPHBUILDER_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_CUDA_TASKFLOWCUDAGRAPHBUILDER_H_

#include "traact/dataflow/taskflow/builder/TaskFlowGraphBuilder.h"
#include "TraactCudaTask.h"

namespace traact::dataflow {

class TaskFlowCudaGraphBuilder : public TaskFlowGraphBuilder {

 public:
    explicit TaskFlowCudaGraphBuilder(TraactTaskFlowGraph &task_flow_graph);
    void buildGraph(const TraactGraph::SharedPtr &traact_graph) override;
 private:
    void createGpuTimeStepTasks(const TraactGraph::SharedPtr &traact_graph,
                           const int time_step_index);
    tf::Task createCudaFlow(const TraactGraph::SharedPtr &traact_graph,
                            int time_step_index,
                            const std::shared_ptr<TraactCudaTask> &cuda_task);
};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_CUDA_TASKFLOWCUDAGRAPHBUILDER_H_
