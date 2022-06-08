/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#ifndef TRAACTTEST_SRC_TRAACT_NETWORK_TRAACTCOMPONENTBASE_H_
#define TRAACTTEST_SRC_TRAACT_NETWORK_TRAACTCOMPONENTBASE_H_

#include <tbb/flow_graph.h>

#include <traact/datatypes.h>
#include <traact/component/Component.h>
#include <traact/pattern/instance/PatternInstance.h>
#include <traact/buffer/TimeStepBuffer.h>
#include <traact/buffer/ComponentBuffer.h>
#include "DataflowMessage.h"

namespace traact::dataflow {
class NetworkGraph;
class TBBTimeDomainManager;
}

namespace traact::dataflow {

class ComponentBase {

 public:
    typedef typename std::shared_ptr<ComponentBase> Ptr;
    typedef typename pattern::instance::PatternInstance::Ptr DefaultPatternPtr;
    typedef typename component::Component::Ptr DefaultComponentPtr;
    typedef typename buffer::ComponentBuffer DefaultComponentBuffer;

    ComponentBase(DefaultPatternPtr pattern_base,
                  DefaultComponentPtr component_base,
                  TBBTimeDomainManager *buffer_manager,
                  NetworkGraph *network_graph);

    virtual ~ComponentBase() = default;;

    virtual bool init() {
        //component_base_->Init();
        return true;
    }

    virtual void connect() = 0;

    bool init_component(buffer::ComponentBufferConfig *buffer_config) {
        SPDLOG_DEBUG("Init component");
        return component_base_->configure(pattern_base_->local_pattern.parameter, buffer_config);
    }

    virtual bool start() {
        component_base_->start();
        return true;
    }

    virtual bool stop() {
        component_base_->stop();
        return true;
    }

    virtual void disconnect() = 0;

    virtual bool teardown() {
        component_base_->teardown();
        return true;
    }

    virtual tbb::flow::sender<TraactMessage> &getSender(int index) {
        throw std::runtime_error("trying to get sender port of component with no senders");
    }
    virtual tbb::flow::receiver<TraactMessage> &getReceiver(int index) {
        throw std::runtime_error("trying to get receiever port of component with no receivers");
    }

    virtual component::ComponentType getComponentType() = 0;

    std::string GetName() const {
        return component_base_->getName();
    }

    void setSourceFinishedCallback(const component::Component::SourceFinishedCallback callback) {
        component_base_->setSourceFinishedCallback(callback);
    }

 protected:
    TBBTimeDomainManager *buffer_manager_;
    DefaultComponentPtr component_base_;
    DefaultPatternPtr pattern_base_;
    NetworkGraph *network_graph_;
    int component_index_;

};
}
#endif //TRAACTTEST_SRC_TRAACT_NETWORK_TRAACTCOMPONENTBASE_H_

#pragma clang diagnostic pop