/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "ComponentGraphBuilder.h"

#include <utility>
#include "traact/dataflow/graph/task/ComponentTask.h"
#include "traact/dataflow/graph/task/SourceTask.h"

namespace traact::dataflow {

ComponentGraphBuilder::ComponentGraphBuilder(GraphBuilderConfig &config)
    : GraphBuilder(config) {}

void ComponentGraphBuilder::buildGraph(TraactGraph &traact_task_flow) {

    prepareComponents(traact_task_flow);

    createTasks(traact_task_flow);

    createDependencies(traact_task_flow);
}

void ComponentGraphBuilder::prepareComponents(TraactGraph &traact_task_flow) {

    for (const auto &[pattern, component] : components_) {
        const auto &instance_id = pattern->instance_id;

        component->setRequestExitCallback(config_.request_exit_callback);

        switch (pattern->getComponentType(time_domain_)) {
            case component::ComponentType::ASYNC_SOURCE:
            case component::ComponentType::INTERNAL_SYNC_SOURCE: {
                setRequestCallback(component, instance_id);
                break;
            }
            case component::ComponentType::SYNC_SOURCE:
            case component::ComponentType::ASYNC_SINK:
            case component::ComponentType::SYNC_FUNCTIONAL:
            case component::ComponentType::ASYNC_FUNCTIONAL:
            case component::ComponentType::SYNC_SINK: {
                break;
            }
            case component::ComponentType::INVALID:
            default: {
                throw std::invalid_argument("invalid component type");
            }
        }

    }
}

void ComponentGraphBuilder::setRequestCallback(const std::shared_ptr<component::Component> &component,
                                              const std::string &instance_id) const {
    auto component_index = config_.time_domain_buffer->getComponentIndex(instance_id);
    component->setRequestCallback(config_.create_buffer_callback(component_index));
}
void ComponentGraphBuilder::createTasks(TraactGraph &traact_task_flow) const {

    for (const auto &[pattern, component] : components_) {
        auto id = pattern->instance_id;
        auto task = createTask(time_domain_, pattern, component);
        traact_task_flow.tasks.emplace(id, task);
    }
}

TraactTask::SharedPtr ComponentGraphBuilder::createTask(int time_domain,
                                                       const std::shared_ptr<pattern::instance::PatternInstance> &pattern,
                                                       const std::shared_ptr<component::Component> &component) const {

    switch (pattern->getComponentType(time_domain)) {
        case component::ComponentType::ASYNC_SOURCE:
        case component::ComponentType::INTERNAL_SYNC_SOURCE: {
            return std::make_shared<SourceTask>(pattern, component);
        }
        case component::ComponentType::SYNC_SOURCE:
        case component::ComponentType::ASYNC_SINK:
        case component::ComponentType::SYNC_FUNCTIONAL:
        case component::ComponentType::ASYNC_FUNCTIONAL:
        case component::ComponentType::SYNC_SINK: {
            return std::make_shared<ComponentTask>(pattern, component);
        }
        case component::ComponentType::INVALID:
        default: {
            throw std::invalid_argument("invalid component type");
        }
    }
}
void ComponentGraphBuilder::createDependencies(TraactGraph &traact_task_flow) {
    

    for (const auto &component : components_) {

        std::shared_ptr<pattern::instance::PatternInstance> pattern_instance = component.first;
        auto instance_id = component.first->instance_id;

        auto &current_task = traact_task_flow.tasks.at(instance_id);

        for (const auto *port : pattern_instance->getProducerPorts(time_domain_)) {
            for (const auto *input_port : port->connectedToPtr()) {
                auto &consumer_task = traact_task_flow.tasks.at(input_port->getId().first);
                current_task->precede(consumer_task);
            }
        }

        switch (component.first->getComponentType(time_domain_)) {
            case component::ComponentType::SYNC_SOURCE: {
                auto sync_source_start = findSyncSourceStartPoint(traact_task_flow, component);
                current_task->succeed(sync_source_start);
                break;
            }
            case component::ComponentType::ASYNC_SOURCE:
            case component::ComponentType::INTERNAL_SYNC_SOURCE:
            case component::ComponentType::ASYNC_SINK:
            case component::ComponentType::SYNC_FUNCTIONAL:
            case component::ComponentType::ASYNC_FUNCTIONAL:
            case component::ComponentType::SYNC_SINK:
            case component::ComponentType::INVALID:
            default:break;
        }

        if(component.first->getConcurrency(time_domain_) == Concurrency::SERIAL){
            current_task->succeedInterTimeStep(current_task);
        }
    }


}
TraactTask::SharedPtr ComponentGraphBuilder::findSyncSourceStartPoint(TraactGraph &traact_task_flow,
                                                                     const std::pair<component::ComponentGraph::PatternPtr,
                                                                                     component::ComponentGraph::ComponentPtr> &component) {
    // find the latest point in dataflow where the sync source needs to be triggered to allow maximum wait for
    // time domain syncing components
    // for now use first async source

    for (const auto &pattern : components_) {
        if (pattern.first->getComponentType(time_domain_) == component::ComponentType::ASYNC_SOURCE) {
            return traact_task_flow.tasks.at(pattern.first->instance_id);
        }
    }
    auto error = fmt::format(
        "sync source {0} has no start point (other non sync source, there should be at least one async source)",
        component.first->instance_id);
    SPDLOG_ERROR(error);
    throw std::invalid_argument(error);
}

} // traact