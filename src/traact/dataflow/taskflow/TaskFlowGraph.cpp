/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TaskFlowGraph.h"
#include "TaskFlowTaskFunctions.h"
#include <utility>

namespace traact::dataflow {

void TaskFlowGraph::init() {

    for (const auto &time_domain : component_graph_->getTimeDomains()) {
        task_flow_time_domains_.emplace_back(std::make_unique<TaskFlowTimeDomain>(time_domain,
                                                                                  component_graph_,
                                                                                  generic_factory_objects_,
                                                                                  source_finished_callback_));

    }

    for (auto &task_flow_time_domain : task_flow_time_domains_) {
        task_flow_time_domain->init();
    }

}

void TaskFlowGraph::start() {
    for (auto &task_flow_time_domain : task_flow_time_domains_) {
        task_flow_time_domain->start();
    }
}

void TaskFlowGraph::stop() {
    for (auto &task_flow_time_domain : task_flow_time_domains_) {
        task_flow_time_domain->stop();
    }

}

void TaskFlowGraph::teardown() {

}

void TaskFlowGraph::masterSourceFinished() {
    // first finished call ends playback
    if (source_finished_.test_and_set()) {
        return;
    }

    source_finished_callback_();
}

TaskFlowGraph::TaskFlowGraph(buffer::DataBufferFactoryPtr generic_factory_objects,
                             component::ComponentGraph::Ptr  component_graph,
                             component::Component::SourceFinishedCallback callback) : generic_factory_objects_(
    std::move(
        generic_factory_objects)), component_graph_(std::move(component_graph)), source_finished_callback_(std::move(callback)) {

}
void TaskFlowGraph::parameterChanged(const std::string &instance_id) {
    for (auto &task_flow_time_domain : task_flow_time_domains_) {
        task_flow_time_domain->parameterChanged(instance_id);
    }

}

} // traact