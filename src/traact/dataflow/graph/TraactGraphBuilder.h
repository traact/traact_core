/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_GRAPH_TRAACTGRAPHBUILDER_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_GRAPH_TRAACTGRAPHBUILDER_H_


#include "traact/dataflow/state/DataflowState.h"
#include "builder/GraphBuilder.h"
#include "TraactGraph.h"

namespace traact::dataflow {

class TraactGraphBuilder {
 public:
    TraactGraphBuilder(GraphBuilderConfig config);

    [[nodiscard]] TraactGraph::SharedPtr build();
 private:
    GraphBuilderConfig config_;
    std::vector<GraphBuilder::UniquePtr> graph_builder_;

    TraactGraph::SharedPtr traact_graph_;
    std::shared_ptr<DataflowState> dataflow_state_;
    void createTimeStepTraactTaskData();
};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_GRAPH_TRAACTGRAPHBUILDER_H_
