/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TraactCudaTask.h"

namespace traact::dataflow {

TraactCudaTask::TraactCudaTask(const std::string &cuda_graph_name,
                               std::map<std::string,
                                        std::shared_ptr<component::CudaComponent>> cuda_components)
    : cuda_graph_name_(cuda_graph_name), cuda_components_(std::move(cuda_components)) {}


const std::string &TraactCudaTask::getId() {
    return cuda_graph_name_;
}

void TraactCudaTask::addInternalSuccessor(const std::string &predecessor, const std::string &successor) {
    component_to_successors_[predecessor].emplace(successor);

}
const std::string &TraactCudaTask::getCudaGraphName() const {
    return cuda_graph_name_;
}
const std::map<std::string, std::shared_ptr<component::CudaComponent>> &TraactCudaTask::getCudaComponents() const {
    return cuda_components_;
}
const std::map<std::string, std::set<std::string>> &TraactCudaTask::getComponentToSuccessors() const {
    return component_to_successors_;
}
} // traact