/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TaskFlowNetwork.h"

bool traact::dataflow::TaskFlowNetwork::start() {

    bool result = true;

    for (const ComponentGraphPtr &component_graph : component_graphs_) {
        task_graphs_.emplace_back(std::make_shared<TaskFlowGraph>(generic_factory_objects_,
                                                                  component_graph,
                                                                  master_source_finished_callback_));
    }
    for (auto &graph : task_graphs_) {
        graph->init();
    }

    for (auto &graph : task_graphs_) {
        graph->start();
    }

    return result;
}

bool traact::dataflow::TaskFlowNetwork::stop() {
    for (auto &graph : task_graphs_) {
        graph->stop();
    }
    return true;
}

traact::dataflow::TaskFlowNetwork::~TaskFlowNetwork() {
    for (auto &graph : task_graphs_) {
        graph->teardown();
    }
}
