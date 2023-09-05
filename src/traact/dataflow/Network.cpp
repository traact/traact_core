/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include <traact/dataflow/Network.h>

#include <utility>

namespace traact::dataflow {

void Network::setComponentGraph(ComponentGraphPtr component_graph) {
    component_graph_ = std::move(component_graph);
}

void Network::setGenericFactoryObjects(buffer::DataBufferFactoryPtr generic_factory_objects) {
    generic_factory_objects_ = std::move(generic_factory_objects);
}

void Network::setMasterSourceFinishedCallback(component::Component::SourceFinishedCallback callback) {
    master_source_finished_callback_ = std::move(callback);
}

}