/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#include "Network.h"

#include <utility>

namespace traact::dataflow {

void Network::addComponentGraph(ComponentGraphPtr component_graph) {
    component_graphs_.emplace(std::move(component_graph));
}

void Network::setGenericFactoryObjects(buffer::DataBufferFactoryPtr generic_factory_objects) {
    generic_factory_objects_ = std::move(generic_factory_objects);
}

void Network::setMasterSourceFinishedCallback(component::Component::SourceFinishedCallback callback) {
    master_source_finished_callback_ = std::move(callback);
}

}