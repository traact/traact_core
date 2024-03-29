/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_NETWORK_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_NETWORK_H_

#include <memory>
#include <map>
#include <traact/buffer/DataFactory.h>
#include <traact/component/ComponentGraph.h>

#include <traact/traact_core_export.h>
#include <traact/buffer/DataBufferFactory.h>
#include "traact/dataflow/state/DataflowState.h"
namespace traact::dataflow {

class TRAACT_CORE_EXPORT Network {
 public:
    typedef typename std::shared_ptr<Network> Ptr;
    typedef typename component::ComponentGraph::Ptr ComponentGraphPtr;

    Network() = default;
    virtual ~Network() = default;

    void setComponentGraph(ComponentGraphPtr component_graph);

    void setGenericFactoryObjects(buffer::DataBufferFactoryPtr generic_factory_objects);

    void setMasterSourceFinishedCallback(component::Component::SourceFinishedCallback callback);

    virtual bool start() = 0;
    virtual bool stop() = 0;

    virtual void parameterChanged(const std::string &instance_id) = 0;
    virtual DataflowState::SharedPtr getDataflowState() = 0;
 protected:

    ComponentGraphPtr component_graph_;
    buffer::DataBufferFactoryPtr generic_factory_objects_;
    component::Component::SourceFinishedCallback master_source_finished_callback_;

};
}

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_NETWORK_H_
