/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include <traact/component/CudaComponent.h>

namespace traact::component {

CudaComponent::CudaComponent(const std::string &name) : Component(name){


}
void CudaComponent::configureInstance(const pattern::instance::PatternInstance &pattern_instance) {
    pattern_instance.setValueFromParameter("cuda_graph", cuda_graph_name_);
}
const std::string &CudaComponent::getCudaGraphName() const {
    return cuda_graph_name_;
}
} // traact