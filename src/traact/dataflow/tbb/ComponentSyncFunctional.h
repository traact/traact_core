/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACTTEST_TRAACTCOMPONENTFUNCTIONAL_H
#define TRAACTTEST_TRAACTCOMPONENTFUNCTIONAL_H

#include <tbb/flow_graph.h>

#include "ComponentBase.h"
#include "DynamicJoinNode.h"

namespace traact::dataflow {

class ComponentSyncFunctional : public ComponentBase {

 public:
    ComponentSyncFunctional(DefaultPatternPtr pattern_base,
                            component::Component::Ptr component_base,
                            TBBTimeDomainManager *buffer_manager,
                            NetworkGraph *network_graph);

    bool init() override;
    bool teardown() override;

    TraactMessage operator()(const TraactMessage &in);

    void connect() override;

    void disconnect() override;

    component::ComponentType getComponentType() override;

    tbb::flow::receiver<TraactMessage> &getReceiver(int index) override;
    tbb::flow::sender<TraactMessage> &getSender(int index) override;

 protected:
    tbb::flow::function_node<TraactMessage, TraactMessage> *node_;
    tbb::flow::broadcast_node<TraactMessage> *broadcast_node_;
    DynamicJoinNode *join_node_;

};

}

#endif //TRAACTTEST_TRAACTCOMPONENTFUNCTIONAL_H
