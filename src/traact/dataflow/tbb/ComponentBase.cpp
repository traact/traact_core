/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#include "ComponentBase.h"
#include "TBBTimeDomainManager.h"
namespace traact::dataflow {
ComponentBase::ComponentBase(DefaultPatternPtr pattern_base,
                             DefaultComponentPtr component_base,
                             TBBTimeDomainManager *buffer_manager,
                             NetworkGraph *network_graph) : pattern_base_(std::move(pattern_base)),
                                                            component_base_(std::move(component_base)),
                                                            buffer_manager_(std::move(buffer_manager)),
                                                            network_graph_(network_graph) {
    component_index_ = buffer_manager_->GetComponentIndex(pattern_base_->instance_id);
}
}