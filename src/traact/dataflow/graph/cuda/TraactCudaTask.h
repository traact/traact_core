/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_CUDA_TRAACTCUDATASK_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_CUDA_TRAACTCUDATASK_H_

#include "traact/dataflow/graph/task/TraactTask.h"
#include "traact/component/CudaComponent.h"

namespace traact::dataflow {

class TraactCudaTask : public TraactTask {
 public:
    TraactCudaTask(const std::string &cuda_graph_name,
                   std::map<std::string,
                            std::shared_ptr<component::CudaComponent>> cuda_components);

    virtual const std::string &getId() override;

    void addInternalSuccessor(const std::string& predecessor, const std::string& successor);
    const std::string &getCudaGraphName() const;
    const std::map<std::string, std::shared_ptr<component::CudaComponent>> &getCudaComponents() const;
    const std::map<std::string, std::set<std::string>> &getComponentToSuccessors() const;
 private:
    std::string cuda_graph_name_;
    std::map<std::string, std::shared_ptr<component::CudaComponent > > cuda_components_;
    std::map<std::string, std::set<std::string>> component_to_successors_;

};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_CUDA_TRAACTCUDATASK_H_
