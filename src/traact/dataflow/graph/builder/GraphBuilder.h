/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_BUILDER_GRAPHBUILDER_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_BUILDER_GRAPHBUILDER_H_

#include <memory>
#include "traact/dataflow/graph/TraactGraph.h"

namespace traact::dataflow {

class GraphBuilder {
 public:
    using UniquePtr = std::unique_ptr<GraphBuilder>;
    GraphBuilder(GraphBuilderConfig &config);
    virtual ~GraphBuilder() = default;
    virtual void buildGraph(TraactGraph& traact_graph) = 0;
 protected:
    GraphBuilderConfig& config_;
    int time_domain_;
    std::set<component::ComponentGraph::PatternComponentPair> components_;



};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_BUILDER_GRAPHBUILDER_H_
