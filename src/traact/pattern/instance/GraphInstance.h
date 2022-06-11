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
    typedef typename std::shared_ptr<GraphInstance> Ptr;
    GraphInstance();
    GraphInstance(const std::string &name);

    PatternInstance::Ptr addPattern(std::string pattern_id, Pattern::Ptr pattern);

    PatternInstance::Ptr getPattern(const std::string &pattern_id) const;

    std::set<PatternInstance::Ptr> getAll() const;

    bool connect(std::string source_component,
                 std::string producer_port,
                 std::string sink_component,
                 std::string consumer_port);
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
};
}

#endif //TRAACT_CORE_SRC_TRAACT_PATTERN_INSTANCE_GRAPHINSTANCE_H_
