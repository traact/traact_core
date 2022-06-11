/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACTTEST_SRC_TRAACT_NETWORK_TRAACTCOMPONENTSOURCE_H_
#define TRAACTTEST_SRC_TRAACT_NETWORK_TRAACTCOMPONENTSOURCE_H_

#include <thread>
#include <tbb/flow_graph.h>

#include "ComponentBase.h"

namespace traact::dataflow {

class ComponentAsyncSource : public ComponentBase {
 public:
    ComponentAsyncSource(DefaultPatternPtr pattern_base,
                         component::Component::Ptr component_base,
                         TBBTimeDomainManager *buffer_manager,
                         NetworkGraph *network_graph);

    bool init() override;
    bool start() override;
    bool stop() override;
    bool teardown() override;
    component::ComponentType getComponentType() override;

    tbb::flow::receiver<TraactMessage> &getReceiver(int index) override;

    tbb::flow::sender<TraactMessage> &getSender(int index) override;

    void connect() override;
    void disconnect() override;

    std::string getComponentName();

    TraactMessage operator()(const TraactMessage &in);
 protected:

    tbb::flow::function_node<TraactMessage, TraactMessage> *node_;
    tbb::flow::broadcast_node<TraactMessage> *broadcast_node_;

    std::future<buffer::SourceComponentBuffer *> RequestSourceBuffer(TimestampType ts, int component_index) {
        std::promise<buffer::SourceComponentBuffer *> foo;
        foo.set_value(nullptr);
        return foo.get_future();

    }

//  buffer::SourceTimeDomainBuffer* RequestBuffer(TimestampType ts) {
//      auto result =buffer_manager_->RequestSourceBuffer(ts, component_base_->getName());
//      if(result)
//        result->SetMessageType(MessageType::Data);
//    return result;
//  }


};

}

#endif //TRAACTTEST_SRC_TRAACT_NETWORK_TRAACTCOMPONENTSOURCE_H_
