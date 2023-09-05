/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TaskFlowNetwork.h"

namespace traact::dataflow {
TaskFlowNetwork::~TaskFlowNetwork() {

}

void TaskFlowNetwork::init() {
    auto time_domain_count = component_graph_->getTimeDomains().size();
    dataflow_state_ = std::make_shared<DataflowState>();
    dataflow_state_->init(time_domain_count);
    for (const auto &time_domain : component_graph_->getTimeDomains()) {
        auto task_flow_time_domain = std::make_unique<TaskFlowTimeDomain>(time_domain, component_graph_, generic_factory_objects_, master_source_finished_callback_, dataflow_state_);
        task_flow_time_domains_.emplace_back(std::move(task_flow_time_domain));
    }

    for (auto &task_flow_time_domain : task_flow_time_domains_) {
        task_flow_time_domain->init();
    }

}

bool TaskFlowNetwork::start() {
    init();

    for (auto &task_flow_time_domain : task_flow_time_domains_) {
        task_flow_time_domain->start();
    }
    return true;
}

bool TaskFlowNetwork::stop() {
    for (auto &task_flow_time_domain : task_flow_time_domains_) {
        task_flow_time_domain->stop();
    }
    return true;

}
void TaskFlowNetwork::parameterChanged(const std::string &instance_id) {
    for (auto &task_flow_time_domain : task_flow_time_domains_) {
        task_flow_time_domain->parameterChanged(instance_id);
    }

}
DataflowState::SharedPtr TaskFlowNetwork::getDataflowState() {
    return dataflow_state_;
}
}
