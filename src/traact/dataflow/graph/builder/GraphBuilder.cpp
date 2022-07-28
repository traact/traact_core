/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "GraphBuilder.h"

namespace traact::dataflow {
GraphBuilder::GraphBuilder(GraphBuilderConfig &config) : config_(config), time_domain_(config_.config.time_domain) {

    components_ = config_.component_graph->getPatternsForTimeDomain(time_domain_);
}


} // traact