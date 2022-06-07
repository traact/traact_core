/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACT_CORE_SRC_TRAACT_COMPONENT_COMPONENTGRAPH_H_
#define TRAACT_CORE_SRC_TRAACT_COMPONENT_COMPONENTGRAPH_H_

#include <memory>
#include <set>

#include <traact/component/Component.h>
#include <traact/component/ModuleComponent.h>
#include <traact/pattern/instance/PatternInstance.h>
#include <traact/pattern/instance/GraphInstance.h>
#include <traact/traact_core_export.h>
namespace traact::component {

class TRAACT_CORE_EXPORT ComponentGraph {
 public:
    typedef typename std::shared_ptr<ComponentGraph> Ptr;
    typedef typename std::shared_ptr<pattern::instance::PatternInstance> PatternPtr;
    typedef typename std::shared_ptr<Component> ComponentPtr;
    typedef typename std::pair<PatternPtr, ComponentPtr> PatternComponentPair;

    explicit ComponentGraph(pattern::instance::GraphInstance::Ptr graph_instance);
    std::string getName() const;

    void addPattern(const std::string &pattern_id, ComponentPtr component);
    const std::set<PatternComponentPair> &getPatterns() const;
    std::set<PatternComponentPair> getPatternsForTimeDomain(int time_domain) const;
    std::set<int> getTimeDomains() const;
    traact::buffer::TimeDomainManagerConfig getTimeDomainConfig(size_t time_domain) const;

    ComponentPtr getComponent(const std::string &instance_id);

 private:
    pattern::instance::GraphInstance::Ptr graph_instance_;
    std::set<PatternComponentPair> patterns_;
    std::map<std::string, Module::Ptr> module_map_;
};
}

#endif //TRAACT_CORE_SRC_TRAACT_COMPONENT_COMPONENTGRAPH_H_
