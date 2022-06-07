/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACTMULTI_COMPONENTSYNCSOURCE_H
#define TRAACTMULTI_COMPONENTSYNCSOURCE_H

#include <tbb/flow_graph.h>

#include "ComponentBase.h"
#include "DynamicJoinNode.h"

namespace traact::dataflow {

class ComponentSyncSource : public ComponentBase {

 public:
    ComponentSyncSource(DefaultPatternPtr pattern_base,
                        DefaultComponentPtr component_base,
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

 private:
    std::string getComponentName();

 protected:
    tbb::flow::function_node<TraactMessage, TraactMessage> *node_;
    tbb::flow::broadcast_node<TraactMessage> *broadcast_node_;
    DynamicJoinNode *join_node_;

};
}

#endif //TRAACTMULTI_COMPONENTSYNCSOURCE_H
