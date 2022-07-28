/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_BUILDER_TASKFLOWGRAPHBUILDER_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_BUILDER_TASKFLOWGRAPHBUILDER_H_

#include "traact/dataflow/graph/TraactGraph.h"
#include "TraactTaskFlowGraph.h"
namespace traact::dataflow {

class TaskFlowGraphBuilder {
 public:
    TaskFlowGraphBuilder(TraactTaskFlowGraph &task_flow_graph);
    virtual ~TaskFlowGraphBuilder() = default;

    virtual void buildGraph(const TraactGraph::SharedPtr &traact_graph) = 0;

 protected:
    TraactTaskFlowGraph& task_flow_graph_;

};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_BUILDER_TASKFLOWGRAPHBUILDER_H_
