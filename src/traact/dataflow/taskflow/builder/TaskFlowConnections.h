/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_BUILDER_TASKFLOWCONNECTIONS_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_BUILDER_TASKFLOWCONNECTIONS_H_

#include "TaskFlowGraphBuilder.h"

namespace traact::dataflow {

class TaskFlowConnections : public TaskFlowGraphBuilder {
 public:
    TaskFlowConnections(TraactTaskFlowGraph &task_flow_graph);
    virtual void buildGraph(const TraactGraph::SharedPtr &traact_graph) override;
 protected:
    tf::Task createSeamEntryTask(int time_step_index, const std::string &seam_entry_name);

    tf::Task createGlobalStartTask();

    tf::Task createGlobalEndTask();
    void createInterTimeStepDependencies(const TraactGraph::SharedPtr &traact_graph);

    tf::SmallVector<int, kStartEntries> start_entries_;

    void connectTimeStepComponents(int time_step, const TraactGraph::SharedPtr &traact_graph);
};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_BUILDER_TASKFLOWCONNECTIONS_H_
