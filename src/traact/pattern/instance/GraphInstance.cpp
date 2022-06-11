/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "traact/util/Utils.h"
#include "traact/pattern/instance/GraphInstance.h"

bool would_connect_to_self_recursive(const traact::pattern::instance::PatternInstance *prod_comp,
                                     const traact::pattern::instance::PatternInstance *test_comp) {
    if (test_comp == prod_comp)
        return true;
    if (test_comp->producer_ports.empty())
        return false;

    for (const auto &tmp : test_comp->producer_ports) {
        if (!tmp.IsConnected())
            continue;
        for (const auto &connected_port : tmp.connectedToPtr()) {
            if (would_connect_to_self_recursive(prod_comp, connected_port->pattern_instance))
                return true;
        }
    }
    return false;

};

traact::pattern::instance::GraphInstance::GraphInstance(const std::string &name) : name(name) {}
traact::pattern::instance::GraphInstance::GraphInstance() : name("Invalid") {

}

traact::pattern::instance::PatternInstance::Ptr traact::pattern::instance::GraphInstance::addPattern(std::string pattern_id,
                                                                                                     traact::pattern::Pattern::Ptr pattern) {
    PatternInstance::Ptr newPattern = std::make_shared<PatternInstance>(pattern_id, *pattern, this);
    pattern_instances[pattern_id] = newPattern;
    return newPattern;
}
traact::pattern::instance::PatternInstance::Ptr traact::pattern::instance::GraphInstance::getPattern(const std::string &pattern_id) const {
    return pattern_instances.at(pattern_id);
}
std::set<traact::pattern::instance::PatternInstance::Ptr> traact::pattern::instance::GraphInstance::getAll() const {
    std::set<PatternInstance::Ptr> result;
    for (const auto &instances : pattern_instances) {
        result.emplace(instances.second);
    }
    return result;
}
traact::pattern::instance::PortInstance::ConstPtr traact::pattern::instance::GraphInstance::getPort(const traact::pattern::instance::ComponentID_PortName &id) const {
    return pattern_instances.at(id.first)->getPort(id.second);
}
void traact::pattern::instance::GraphInstance::initializeGraphConnections() {
    for (auto &pattern_instance : pattern_instances) {
        pattern_instance.second->parent_graph = this;
        for (auto &producer_port : pattern_instance.second->producer_ports) {
            producer_port.pattern_instance = pattern_instance.second.get();
        }
        for (auto &consumer_port : pattern_instance.second->consumer_ports) {
            consumer_port.pattern_instance = pattern_instance.second.get();
        }
    }

}
bool traact::pattern::instance::GraphInstance::connect(std::string source_component,
                                                       std::string producer_port,
                                                       std::string sink_component,
                                                       std::string consumer_port) {

    if (checkSourceAndSinkConnectionError(source_component, producer_port, sink_component, consumer_port).has_value())
        return false;

    PortInstance::Ptr
        source = util::vectorGetForName(pattern_instances[source_component]->producer_ports, producer_port);
    PortInstance::Ptr sink = util::vectorGetForName(pattern_instances[sink_component]->consumer_ports, consumer_port);

    sink->connected_to = source->getID();

    return true;
}
std::set<traact::pattern::instance::PortInstance::ConstPtr> traact::pattern::instance::GraphInstance::connectedToPtr(
    const traact::pattern::instance::ComponentID_PortName &id) const {

    std::set<traact::pattern::instance::PortInstance::ConstPtr> result;

    PortInstance::ConstPtr port = getPort(id);
    if (port->port.port_type == PortType::CONSUMER) {
        for (const auto &pattern_instance : pattern_instances) {
            for (const auto &producer_port : pattern_instance.second->producer_ports) {
                if (producer_port.getID() == port->connected_to)
                    result.emplace(&producer_port);
            }
        }
    } else {
        for (const auto &pattern_instance : pattern_instances) {
            for (const auto &consumer_port : pattern_instance.second->consumer_ports) {
                if (consumer_port.connected_to == id)
                    result.emplace(&consumer_port);
            }
        }
    }

    return result;
}

std::optional<std::string>
traact::pattern::instance::GraphInstance::checkSourceAndSinkConnectionError(const std::string &source_component,
                                                                            const std::string &producer_port,
                                                                            const std::string &sink_component,
                                                                            const std::string &consumer_port) {

    if (source_component == sink_component)
        return fmt::format("Cannot connect component to itself {0}", source_component);

    auto find_source = pattern_instances.find(source_component);
    if (find_source == pattern_instances.end()) {
        return fmt::format("Unknown Source {0} when trying to connect {0}:{1} to {2}:{3}",
                           source_component,
                           producer_port,
                           sink_component,
                           consumer_port);
    } else {
        if (!util::vectorContainsName(find_source->second->producer_ports, producer_port)) {
            return fmt::format("Unknown Port {1} in Source {0} when trying to connect {0}:{1} to {2}:{3}",
                               source_component,
                               producer_port,
                               sink_component,
                               consumer_port);;
        }

    }

    auto find_sink = pattern_instances.find(sink_component);
    if (find_sink == pattern_instances.end()) {
        return fmt::format("Unknown Sink {2} when trying to connect {0}:{1} to {2}:{3}",
                           source_component,
                           producer_port,
                           sink_component,
                           consumer_port);
    } else {
        if (!util::vectorContainsName(find_sink->second->consumer_ports, consumer_port)) {
            return fmt::format("Unknown Port {3} in Sink {2} when trying to connect {0}:{1} to {2}:{3}",
                               source_component,
                               producer_port,
                               sink_component,
                               consumer_port);
        }
    }

    auto prod_comp = find_source->second;
    auto cons_comp = find_sink->second;
    auto prod_port = util::vectorGetForName(prod_comp->producer_ports, producer_port);
    auto cons_port = util::vectorGetForName(cons_comp->consumer_ports, consumer_port);

    if (prod_port->port.datatype != cons_port->port.datatype)
        return fmt::format("Incompatible port datatype {0} -> {1}",
                           prod_port->port.datatype,
                           cons_port->port.datatype);;

    if (would_connect_to_self_recursive(prod_comp.get(), cons_comp.get()))
        return fmt::format("Connection would create loop in dataflow");

    //cons_port->




    return {};
}

bool traact::pattern::instance::GraphInstance::disconnect(std::string sink_component, std::string consumer_port) {

    PortInstance::Ptr sink = util::vectorGetForName(pattern_instances[sink_component]->consumer_ports, consumer_port);

    sink->connected_to.first = "";
    sink->connected_to.second = "";

    return true;
}

std::optional<std::string> traact::pattern::instance::GraphInstance::checkSourceAndSinkConnectionError(
    const traact::pattern::instance::ComponentID_PortName &source,
    const traact::pattern::instance::ComponentID_PortName &sink) {
    return checkSourceAndSinkConnectionError(source.first, source.second, sink.first, sink.second);
}

