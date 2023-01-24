/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_BUILDER_TASKFLOWTIMESTEP_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_BUILDER_TASKFLOWTIMESTEP_H_

#include "TaskFlowGraphBuilder.h"

namespace traact::dataflow {

class TaskFlowTimeStep : public TaskFlowGraphBuilder {
 public:
    TaskFlowTimeStep(TraactTaskFlowGraph &task_flow_graph);
    virtual void buildGraph(const TraactGraph::SharedPtr &traact_graph) override;
 protected:
    void createStartStopTasks(const TraactGraph &traact_graph, int time_step_index);

    tf::Task createLocalStartTask(int time_step_index, const std::string &name);

    tf::Task createLocalEndTask(int time_step_index, const std::string &name);
};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_BUILDER_TASKFLOWTIMESTEP_H_
