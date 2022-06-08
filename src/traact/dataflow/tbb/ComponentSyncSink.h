/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACTTEST_SRC_TRAACT_NETWORK_TRAACTCOMPONENTSINK_H_
#define TRAACTTEST_SRC_TRAACT_NETWORK_TRAACTCOMPONENTSINK_H_

#include <tbb/flow_graph.h>

#include "ComponentBase.h"
#include "DynamicJoinNode.h"

namespace traact::dataflow {

class ComponentSyncSink : public ComponentBase {
 public:

    ComponentSyncSink(DefaultPatternPtr pattern_base,
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

 protected:

    tbb::flow::function_node<TraactMessage, TraactMessage> *node_;
    DynamicJoinNode *join_node_;
    tbb::flow::sequencer_node<TraactMessage> *sequencer_node_;

};

}

#endif //TRAACTTEST_SRC_TRAACT_NETWORK_TRAACTCOMPONENTSINK_H_
