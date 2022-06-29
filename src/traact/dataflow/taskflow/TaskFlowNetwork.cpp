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

    is_running_ = result;

    return result;
}

bool traact::dataflow::TaskFlowNetwork::stop() {
    if(is_running_){
        for (auto &graph : task_graphs_) {
            graph->stop();
        }
    }
    is_running_ = false;

    return true;
}

traact::dataflow::TaskFlowNetwork::~TaskFlowNetwork() {
    for (auto &graph : task_graphs_) {
        graph->teardown();
    }
}
void traact::dataflow::TaskFlowNetwork::parameterChanged(const std::string &instance_id) {
    for (auto &graph : task_graphs_) {
        graph->parameterChanged(instance_id);
    }

}
