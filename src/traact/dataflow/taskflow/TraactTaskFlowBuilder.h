/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TRAACTTASKFLOWBUILDER_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TRAACTTASKFLOWBUILDER_H_

#include "traact/dataflow/graph/TraactGraph.h"
#include "builder/TaskFlowGraphBuilder.h"

namespace traact::dataflow {

class TraactTaskFlowBuilder {
 public:
    TraactTaskFlowBuilder(tf::Taskflow *taskflow,
                          std::shared_ptr<buffer::TimeDomainBuffer> time_domain_buffer,
                          int time_domain,
                          TaskFlowInFlowScheduler *scheduler);
    void build(const TraactGraph::SharedPtr& traact_graph);
 private:
    TraactTaskFlowGraph graph_;
    std::vector<std::unique_ptr<TaskFlowGraphBuilder> > builder_;
};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TRAACTTASKFLOWBUILDER_H_
