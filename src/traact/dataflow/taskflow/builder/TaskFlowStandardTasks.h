/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_BUILDER_TASKFLOWSTANDARDTASKS_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_BUILDER_TASKFLOWSTANDARDTASKS_H_

#include "TaskFlowGraphBuilder.h"
#include "traact/dataflow/graph/task/ComponentTask.h"
#include "traact/dataflow/graph/task/SourceTask.h"
#include "traact/dataflow/graph/task/ModuleTask.h"
#include "traact/dataflow/graph/task/ControlFlowTask.h"

namespace traact::dataflow {
class TaskFlowStandardTasks : public  TaskFlowGraphBuilder{
 public:
    explicit TaskFlowStandardTasks(TraactTaskFlowGraph &task_flow_graph);
    void buildGraph(const TraactGraph::SharedPtr &traact_graph) override;
 private:
    void createTimeStep(int time_step, const TraactGraph::SharedPtr &traact_graph);
    tf::Task createTask(std::shared_ptr<ComponentTask> task, std::shared_ptr<TraactTaskData> data);
    tf::Task createTask(std::shared_ptr<SourceTask> task, std::shared_ptr<TraactTaskData> data);
    tf::Task createTask(std::shared_ptr<ModuleTask> task, std::shared_ptr<TraactTaskData> data);
    tf::Task createTask(std::shared_ptr<ControlFlowTask> task, std::shared_ptr<TraactTaskData> data);
};
}


#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_BUILDER_TASKFLOWSTANDARDTASKS_H_
