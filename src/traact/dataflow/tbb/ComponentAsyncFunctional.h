/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACTTEST_TRAACTCOMPONENTASYNCFUNCTIONAL_H
#define TRAACTTEST_TRAACTCOMPONENTASYNCFUNCTIONAL_H

#include <tbb/flow_graph.h>
#include <tbb/concurrent_hash_map.h>

#include "ComponentBase.h"
#include "DynamicJoinNode.h"

namespace traact::dataflow {

class ComponentAsyncFunctional : public ComponentBase {

 public:
    ComponentAsyncFunctional(DefaultPatternPtr pattern_base,
                             DefaultComponentPtr component_base,
                             TBBTimeDomainManager *buffer_manager,
                             NetworkGraph *network_graph);

    bool init() override;
    bool teardown() override;

    void connect() override;

    void disconnect() override;

    component::ComponentType getComponentType() override;

    tbb::flow::receiver<TraactMessage> &getReceiver(int index) override;
    tbb::flow::sender<TraactMessage> &getSender(int index) override;

    void ReleaseGateway(TimestampType ts, bool valid);

 protected:

    typedef tbb::flow::async_node<TraactMessage, TraactMessage> AsyncNodeType;

    struct work_type {
        TraactMessage input;
        AsyncNodeType::gateway_type *gateway;
    };

    typedef tbb::concurrent_hash_map<TimestampType, work_type, TimestampHashCompare> MapDataType;

    void submit(TraactMessage in, AsyncNodeType::gateway_type &gateway);
    AsyncNodeType *node_;
    DynamicJoinNode *join_node_;
    tbb::flow::broadcast_node<TraactMessage> *broadcast_node_;
    //tbb::flow::sequencer_node<TraactMessage> *sequencer_node_;

    MapDataType async_messages_;

};

}

#endif //TRAACTTEST_TRAACTCOMPONENTASYNCFUNCTIONAL_H
