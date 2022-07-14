/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_COMPONENT_GPUCOMPONENT_H_
#define TRAACT_CORE_SRC_TRAACT_COMPONENT_GPUCOMPONENT_H_

#include "Component.h"

#include <cuda_runtime.h>

namespace traact::component {

using GpuTask = std::function<void(cudaStream_t)>;

class TRAACT_CORE_EXPORT GpuComponent : public Component{
 public:
    static traact::pattern::Pattern::Ptr GetPattern(const std::string& pattern_name,Concurrency t_concurrency,
                                                    component::ComponentType component_type) {

        traact::pattern::Pattern::Ptr
            pattern =
            std::make_shared<traact::pattern::Pattern>(pattern_name, t_concurrency, component_type);

        pattern->addStringParameter("cuda_graph", "default_cuda_graph");


        return pattern;
    }

    explicit GpuComponent(const std::string &name);
    virtual void configureInstance(const pattern::instance::PatternInstance &pattern_instance) override;


    virtual GpuTask createGpuTask(buffer::ComponentBuffer *data)=0;
    const std::string& getCudaGraphName() const;
 protected:
    std::string cuda_graph_name_;

};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_COMPONENT_GPUCOMPONENT_H_
