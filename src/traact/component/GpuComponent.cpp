/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "GpuComponent.h"

namespace traact::component {

GpuComponent::GpuComponent(const std::string &name) : Component(name){


}
void GpuComponent::configureInstance(const pattern::instance::PatternInstance &pattern_instance) {
    pattern_instance.setValueFromParameter("cuda_graph", cuda_graph_name_);
}
const std::string &GpuComponent::getCudaGraphName() const {
    return cuda_graph_name_;
}
} // traact