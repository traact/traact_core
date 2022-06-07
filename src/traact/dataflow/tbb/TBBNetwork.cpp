/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#include "TBBNetwork.h"

#include "NetworkGraph.h"

traact::dataflow::TBBNetwork::TBBNetwork()
    : Network() {}

bool traact::dataflow::TBBNetwork::start() {
    network_graphs_.clear();
    bool result = true;

    for (const ComponentGraphPtr &component_graph : component_graphs_) {
        auto newGraph = std::make_shared<NetworkGraph>(component_graph, generic_factory_objects_);
        newGraph->setMasterSourceFinishedCallback(std::bind(&TBBNetwork::MasterSourceFinished, this));
        network_graphs_.emplace(newGraph);
    }

    for (const auto &network_graph : network_graphs_) {
        //result = result && network_graph->init();
        network_graph->init();
    }

    for (const auto &network_graph : network_graphs_) {
        //result = result && network_graph->start();
        network_graph->start();
    }
    return result;
}

bool traact::dataflow::TBBNetwork::stop() {
    bool result = true;
    for (const auto &network_graph : network_graphs_) {
        //result = result && network_graph->stop();
        network_graph->stop();
    }

    for (const auto &network_graph : network_graphs_) {
        //result = result && network_graph->teardown();
        network_graph->teardown();
    }
    network_graphs_.clear();
    return result;
}

void traact::dataflow::TBBNetwork::MasterSourceFinished() {
    finished_count_++;
    if (finished_count_ == network_graphs_.size()) {
        stop_signal_thread_ = std::thread(master_source_finished_callback_);
    }
}

traact::dataflow::TBBNetwork::~TBBNetwork() {
    if (stop_signal_thread_.joinable())
        stop_signal_thread_.join();

}
