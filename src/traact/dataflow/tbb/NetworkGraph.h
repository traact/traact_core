/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACTTEST_SRC_TRAACT_NETWORK_NETWORKGRAPH_H_
#define TRAACTTEST_SRC_TRAACT_NETWORK_NETWORKGRAPH_H_

#include <memory>
#include <tbb/flow_graph.h>

#include "ComponentBase.h"
#include <traact/component/ComponentGraph.h>
#include <traact/pattern/instance/PortInstance.h>

#include <traact/buffer/DataFactory.h>

#include <traact/buffer/TimeStepBuffer.h>
#include "TBBTimeDomainManager.h"
namespace traact::dataflow {

class NetworkGraph {

 public:
    typedef typename std::shared_ptr<NetworkGraph> Ptr;

    typedef typename ComponentBase::Ptr TraactComponentBasePtr;
    typedef typename component::ComponentGraph::Ptr DefaultComponentGraphPtr;

    typedef typename component::Component DefaultComponent;
    typedef typename component::Component::Ptr DefaultComponentPtr;

    typedef typename pattern::instance::PatternInstance DefaultPattern;
    typedef typename DefaultPattern::Ptr DefaultPatternPtr;
    typedef typename pattern::instance::PortInstance::ConstPtr PortPtr;

    NetworkGraph(DefaultComponentGraphPtr component_graph,
                 std::set<buffer::BufferFactory::Ptr> generic_factory_objects);
    ~NetworkGraph() = default;

    void init();
    void start();
    void stop();
    void teardown();

    bool IsRunning();

    tbb::flow::graph &getTBBGraph();

    tbb::flow::sender<TraactMessage> &getSender(PortPtr port);
    tbb::flow::receiver<TraactMessage> &getReceiver(PortPtr port);

    tbb::flow::receiver<TraactMessage> &getSourceReceiver(const std::string &component_name);
    tbb::flow::sender<TraactMessage> &getSender(const std::string &component_name);

    void setMasterSourceFinishedCallback(component::Component::SourceFinishedCallback callback);

 private:
    bool running_{false};
    DefaultComponentGraphPtr component_graph_;
    std::map<int, std::shared_ptr<TBBTimeDomainManager>> time_domain_manager_;
    tbb::flow::graph graph_;

    std::vector<TraactComponentBasePtr> network_components_;
    std::map<PortPtr, TraactComponentBasePtr> port_to_network_component;

    std::set<buffer::BufferFactory::Ptr> generic_factory_objects_;

    std::atomic_flag source_finished_{ATOMIC_FLAG_INIT};
    component::Component::SourceFinishedCallback source_finished_callback;
    void MasterSourceFinished();
};

}
#endif //TRAACTTEST_SRC_TRAACT_NETWORK_NETWORKGRAPH_H_
