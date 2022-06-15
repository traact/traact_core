/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_PATTERN_INSTANCE_GRAPHINSTANCE_H_
#define TRAACT_CORE_SRC_TRAACT_PATTERN_INSTANCE_GRAPHINSTANCE_H_
#include <map>
#include <memory>
#include <traact/pattern/instance/PatternInstance.h>
#include <traact/traact_core_export.h>
namespace traact::pattern::instance {
struct TRAACT_CORE_EXPORT GraphInstance {
 public:
    using Ptr = std::shared_ptr<GraphInstance>;
    using WeakPtr = std::weak_ptr<GraphInstance>;
    GraphInstance();
    GraphInstance(const std::string &name);

    PatternInstance::Ptr addPattern(std::string pattern_id, const Pattern::Ptr& pattern, int time_domain = 0);

    PatternInstance::Ptr getPattern(const std::string &pattern_id) const;

    std::set<PatternInstance::Ptr> getAll() const;

    void connect(const std::string& source_component,
                 const std::string& producer_port,
                 const std::string& sink_component,
                 const std::string& consumer_port);
    bool disconnect(std::string sink_component, std::string consumer_port);

    traact::pattern::instance::PortInstance::ConstPtr getPort(const ComponentID_PortName &id) const;
    std::set<traact::pattern::instance::PortInstance::ConstPtr> connectedToPtr(const ComponentID_PortName &id) const;

    std::string name;
    std::map<std::string, PatternInstance::Ptr> pattern_instances;
    std::map<size_t, traact::buffer::TimeDomainManagerConfig> timedomain_configs;

    void initializeGraphConnections();

    std::optional<std::string> checkSourceAndSinkConnectionError(const std::string &source_component,
                                                                 const std::string &producer_port,
                                                                 const std::string &sink_component,
                                                                 const std::string &consumer_port);
    std::optional<std::string> checkSourceAndSinkConnectionError(const traact::pattern::instance::ComponentID_PortName &source,
                                                                 const traact::pattern::instance::ComponentID_PortName &sink);
    void initializeGraphPortConnections(PortGroupInstance &port_group_instance);
 private:
    void forAllPatternInstances(const std::function<void (const PortGroupInstance&)>& func) const;
};
}

#endif //TRAACT_CORE_SRC_TRAACT_PATTERN_INSTANCE_GRAPHINSTANCE_H_
