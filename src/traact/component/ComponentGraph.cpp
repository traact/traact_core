/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "ComponentGraph.h"
#include "traact/util/Logging.h"

#include <exception>

namespace traact::component {

ComponentGraph::ComponentGraph(pattern::instance::GraphInstance::Ptr graph_instance) : graph_instance_(std::move(
    graph_instance)) {

}

std::string ComponentGraph::getName() const {
    return graph_instance_->name;
}

void ComponentGraph::addPattern(const std::string &pattern_id, ComponentPtr component) {
    SPDLOG_INFO("Graph add: {0} Component: {1}", getName(), component->getName());

    ModuleComponent::Ptr module_comp_tmp = std::dynamic_pointer_cast<ModuleComponent, Component>(component);

    if (module_comp_tmp) {
        SPDLOG_INFO("Graph: {0} Component: {1} is a module component", getName(), component->getName());
        std::string module_key = module_comp_tmp->getModuleKey();
        if (module_map_[module_key]) {
            SPDLOG_INFO("Graph: {0} Component: {1} module exists", getName(), component->getName());
            module_comp_tmp->setModule(module_map_[module_key]);
        } else {
            SPDLOG_INFO("Graph: {0} Component: {1} create new module", getName(), component->getName());
            Module::Ptr new_module = module_comp_tmp->instantiateModule();
            module_comp_tmp->setModule(new_module);
            module_map_[module_key] = new_module;
        }
    }

    patterns_.emplace(std::make_pair(graph_instance_->getPattern(pattern_id), std::move(component)));

}
const std::set<ComponentGraph::PatternComponentPair> &ComponentGraph::getPatterns() const {
    return patterns_;
}

ComponentGraph::ComponentPtr ComponentGraph::getComponent(const std::string &instance_id) {
    for (const auto &item : patterns_) {
        if (item.first->instance_id == instance_id) {
            return item.second;
        }
    }
    return nullptr;
}

std::set<ComponentGraph::PatternComponentPair> ComponentGraph::getPatternsForTimeDomain(int time_domain) const {
    std::set<ComponentGraph::PatternComponentPair> result;
    for (const auto &pattern : patterns_) {
        switch (pattern.first->getComponentType(time_domain)) {
            case ComponentType::ASYNC_SOURCE:
            case ComponentType::INTERNAL_SYNC_SOURCE:
            case ComponentType::ASYNC_SINK:
            case ComponentType::SYNC_SOURCE:
            case ComponentType::SYNC_FUNCTIONAL:
            case ComponentType::ASYNC_FUNCTIONAL:
            case ComponentType::SYNC_SINK: {

                if (pattern.first->isInTimeDomain(time_domain)) {
                    result.emplace(pattern);
                }
                break;
            }
            case ComponentType::INVALID: {
                throw std::runtime_error("getPatternsForTimeDomain for invalid ComponentType");
            }
            default: {

                throw std::runtime_error("getPatternsForTimeDomain not implemented for ComponentType : "
                                             + std::to_string((int) pattern.first->getComponentType(time_domain)));
            }

        }
    }
    return std::move(result);
}
std::set<int> ComponentGraph::getTimeDomains() const {
    std::set<int> all_domains;

    for (const auto &pattern : patterns_) {
        for(int global_time_domain : pattern.first->local_to_global_time_domain){
            all_domains.emplace(global_time_domain);
        }

    }

    return all_domains;
}

traact::buffer::TimeDomainManagerConfig ComponentGraph::getTimeDomainConfig(size_t time_domain) const {
    auto result = graph_instance_->timedomain_configs.find(time_domain);
    if (result == graph_instance_->timedomain_configs.end()) {
        traact::buffer::TimeDomainManagerConfig config;
        SPDLOG_ERROR("no valid time domain manager config for time domain {0}", time_domain);
        return config;
    } else {
        return result->second;
    }

}

}
