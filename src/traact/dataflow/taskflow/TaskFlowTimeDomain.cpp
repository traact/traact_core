/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TaskFlowTimeDomain.h"
#include <utility>

#include "traact/dataflow/graph/cuda/TraactGpuGraph.cuh"

namespace traact::dataflow {

TaskFlowTimeDomain::TaskFlowTimeDomain(int time_domain,
                                       component::ComponentGraph::Ptr component_graph,
                                       buffer::DataBufferFactoryPtr buffer_factory,
                                       component::Component::SourceFinishedCallback callback,
                                       DataflowState::SharedPtr dataflow_state)
    : generic_factory_objects_(std::move(buffer_factory)),
      source_finished_callback_(std::move(callback)),
      component_graph_(std::move(component_graph)),
      time_domain_(time_domain) {



    createBuffer();

    scheduler_ = std::make_unique<DefaultScheduler>(time_domain_config_,
                                                    time_domain_buffer_,
                                                    component_graph_->getName(),
                                                    time_domain_,
                                                    &taskflow_, [&]() {
            printState();
        }, dataflow_state->getState(time_domain_));

    auto create_buffer_callback = [scheduler = scheduler_.get()](int component_index) {
        return [scheduler, component_index](Timestamp timestamp) {
            return scheduler->requestSourceBuffer(timestamp, component_index);
        };
    };

    GraphBuilderConfig
        graph_builder_config(time_domain_config_,
                             component_graph_,
                             time_domain_buffer_,
                             create_buffer_callback,
                             source_finished_callback_, dataflow_state);

    graph_builder_ = std::make_unique<TraactGraphBuilder>(graph_builder_config);
    task_flow_builder_ = std::make_unique<TraactTaskFlowBuilder>(&taskflow_, time_domain_buffer_, time_domain_config_.time_domain, scheduler_.get());
}

void TaskFlowTimeDomain::init() {



    taskflow_.name(fmt::format("{0}_TD:{1}", component_graph_->getName(), time_domain_));

    auto traact_graph = graph_builder_->build();

    task_flow_builder_->build(traact_graph);



    SPDLOG_TRACE("dump of task flow \n{0}", taskflow_.dump());

}

void TaskFlowTimeDomain::createBuffer() {
    time_domain_config_ = component_graph_->getTimeDomainConfig(time_domain_);
    time_domain_buffer_ = std::make_shared<buffer::TimeDomainBuffer>(time_domain_, generic_factory_objects_);

    time_step_count_ = time_domain_config_.ringbuffer_size;
    time_domain_buffer_->init(*component_graph_);

    for (int time_step_index = 0; time_step_index < time_step_count_; ++time_step_index) {
        time_domain_buffer_->getTimeStepBuffer(time_step_index).setEvent(kTimestampZero, EventType::INVALID);
    }

}

void TaskFlowTimeDomain::stop() {

    if (stop_called_ || !running_) {
        return;
    }

    stop_called_ = true;
    scheduler_->stop();

    running_ = false;

}

void TaskFlowTimeDomain::start() {

    running_ = true;
    scheduler_->start();

}

void TaskFlowTimeDomain::printState() {
    std::stringstream ss;
    ss << "State of dataflow network\n";
    int time_step_buffer = 0;
//    for (const auto &data : time_step_data_) {
//        ss << "time step buffer " << time_step_buffer << ":\n";
//        for (const auto &component_data : data.component_data) {
//            ss << fmt::format("{0} last call: {1} position: {2} event type: {3} error: {4}\n",
//                              component_data.first,
//                              component_data.second.state_last_call,
//                              component_data.second.state_last_event_position,
//                              component_data.second.state_last_event_type,
//                              component_data.second.state_had_error);
//        }
//        ++time_step_buffer;
//    }

    SPDLOG_WARN(ss.str());
}

void TaskFlowTimeDomain::parameterChanged(const std::string &instance_id) {
    scheduler_->parameterChanged(instance_id);

}

}