/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACTMULTI_COMPONENTASYNCSINK_H
#define TRAACTMULTI_COMPONENTASYNCSINK_H

#include <tbb/flow_graph.h>
#include <tbb/concurrent_hash_map.h>

#include "ComponentBase.h"
#include "DynamicJoinNode.h"

namespace traact::dataflow {

class ComponentAsyncSink : public ComponentBase {
 public:

    ComponentAsyncSink(DefaultPatternPtr pattern_base,
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
    tbb::flow::sequencer_node<TraactMessage> *sequencer_node_;

    MapDataType async_messages_;

};

}

#endif //TRAACTMULTI_COMPONENTASYNCSINK_H
