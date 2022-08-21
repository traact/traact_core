/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "traact/util/Utils.h"
#include "traact/pattern/instance/GraphInstance.h"

namespace traact::pattern::instance {

bool would_connect_to_self_recursive(const traact::pattern::instance::PatternInstance *prod_comp,
                                     const traact::pattern::instance::PatternInstance *test_comp) {
    if (test_comp == prod_comp)
        return true;

    for (const auto &port_groups : test_comp->port_groups) {
        for (const auto &port_group_instance : port_groups) {
            for (const auto &tmp : port_group_instance->producer_ports) {
                if (!tmp.isConnected())
                    continue;
                for (const auto &connected_port : tmp.connectedToPtr()) {
                    if (would_connect_to_self_recursive(prod_comp,
                                                        connected_port->port_group_instance->pattern_instance))
                        return true;
                }
            }

        }

    }

    return false;

};

traact::pattern::instance::GraphInstance::GraphInstance(const std::string &name) : name(name) {}
traact::pattern::instance::GraphInstance::GraphInstance() : name("Invalid") {

}

traact::pattern::instance::PatternInstance::Ptr traact::pattern::instance::GraphInstance::addPattern(std::string pattern_id,
                                                                                                     const traact::pattern::Pattern::Ptr &pattern,
                                                                                                     int time_domain) {
    auto pattern_exists = pattern_instances.find(pattern_id);
    if(pattern_exists != pattern_instances.end()){
        auto error = fmt::format("pattern id {0} already exists", pattern_id);
        SPDLOG_ERROR(error);
        throw std::invalid_argument(error);
    }
    PatternInstance::Ptr newPattern = std::make_shared<PatternInstance>(pattern_id, *pattern, this);
    newPattern->local_to_global_time_domain.resize(pattern->concurrency.size(), -1);
    newPattern->local_to_global_time_domain[kDefaultTimeDomain] = time_domain;
    pattern_instances.emplace(pattern_id, newPattern);
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
        for (auto &group_port_instances : pattern_instance.second->port_groups) {
            for (auto &group_port_instance : group_port_instances) {
                group_port_instance->pattern_instance = pattern_instance.second.get();
                initializeGraphPortConnections(*group_port_instance);
            }
        }
    }
}
void traact::pattern::instance::GraphInstance::connect(const std::string &source_component,
                                                       const std::string &producer_port,
                                                       const std::string &sink_component,
                                                       const std::string &consumer_port) {

    auto check_result =
        checkSourceAndSinkConnectionError(source_component, producer_port, sink_component, consumer_port);
    if (check_result.has_value()) {
        SPDLOG_ERROR(check_result.value());
        throw std::invalid_argument(check_result.value());
    }

    PortInstance::ConstPtr source = pattern_instances[source_component]->getProducerPort(producer_port);
    PortInstance::Ptr sink = pattern_instances[sink_component]->getConsumerPort(consumer_port);
    sink->connected_to = source->getId();

}
std::set<traact::pattern::instance::PortInstance::ConstPtr> traact::pattern::instance::GraphInstance::connectedToPtr(
    const traact::pattern::instance::ComponentID_PortName &id) const {

    std::set<traact::pattern::instance::PortInstance::ConstPtr> result;

    PortInstance::ConstPtr port = getPort(id);
    if (port == nullptr) {
        throw std::invalid_argument(fmt::format("unknown port component: {0} port: {1}, check graph connections",
                                                id.first,
                                                id.second));
    }



    forAllPatternInstances([&result, port](const PortGroupInstance &port_group_instance) {
        PortType search_type = port->getPortType() == PortType::PRODUCER ? PortType::CONSUMER : PortType::PRODUCER;
        for (const auto &other_port : port_group_instance.getPortList(search_type)) {
            if(port->getPortType() == PortType::PRODUCER){
                if (other_port.connected_to == port->getId()) {
                    result.emplace(&other_port);
                }
            } else {
                if (other_port.getId() == port->connected_to) {
                    result.emplace(&other_port);
                }
            }

        }
    });

    return result;
}

std::optional<std::string>
traact::pattern::instance::GraphInstance::checkSourceAndSinkConnectionError(const std::string &source_component,
                                                                            const std::string &producer_port,
                                                                            const std::string &sink_component,
                                                                            const std::string &consumer_port) {

    if (source_component == sink_component) {
        return fmt::format("Cannot connect component to itself {0}", source_component);
    }

    auto find_source = pattern_instances.find(source_component);
    PatternInstance::Ptr prod_comp;
    PortInstance::ConstPtr prod_port;
    if (find_source == pattern_instances.end()) {
        return fmt::format("Unknown Source {0} when trying to connect {0}:{1} to {2}:{3}",
                           source_component,
                           producer_port,
                           sink_component,
                           consumer_port);
    } else {
        prod_comp = find_source->second;
        prod_port = prod_comp->getProducerPort(producer_port);
        if (prod_port == nullptr) {
            return fmt::format("Unknown Port {1} in Source {0} when trying to connect {0}:{1} to {2}:{3}",
                               source_component,
                               producer_port,
                               sink_component,
                               consumer_port);
        }
    }

    auto find_sink = pattern_instances.find(sink_component);
    PatternInstance::Ptr cons_comp;
    PortInstance::ConstPtr cons_port;

    if (find_sink == pattern_instances.end()) {
        return fmt::format("Unknown Sink {2} when trying to connect {0}:{1} to {2}:{3}",
                           source_component,
                           producer_port,
                           sink_component,
                           consumer_port);
    } else {

        cons_comp = find_sink->second;
        cons_port = cons_comp->getConsumerPort(consumer_port);
        if (cons_port == nullptr) {
            return fmt::format("Unknown Port {3} in Sink {2} when trying to connect {0}:{1} to {2}:{3}",
                               source_component,
                               producer_port,
                               sink_component,
                               consumer_port);
        }

    }

    if (prod_port->getTimeDomain() != cons_port->getTimeDomain()) {
        return fmt::format("Different time domains when trying to connect {0}:{1} to {2}:{3}, {4} -> {5}",
                           source_component,
                           producer_port,
                           sink_component,
                           consumer_port,
                           prod_port->getTimeDomain(),
                           cons_port->getTimeDomain());
    }

    if (prod_port->port.datatype != cons_port->port.datatype) {
        return fmt::format("Incompatible port datatype {0}:{1} -> {2}:{3}",
                           source_component,
                           prod_port->port.datatype,
                           sink_component,
                           cons_port->port.datatype);
    }

    if (would_connect_to_self_recursive(prod_comp.get(), cons_comp.get())) {
        return fmt::format("Connection would create loop in dataflow");
    }

    return {};
}

bool traact::pattern::instance::GraphInstance::disconnect(std::string sink_component, std::string consumer_port) {

    throw std::logic_error("not implemented");
//    PortInstance::Ptr
//        sink = util::vectorGetForName(pattern_instances[sink_component]->default_ports->consumer_ports, consumer_port);
//
//    sink->connected_to.first = "";
//    sink->connected_to.second = "";

//    return true;
}

std::optional<std::string> traact::pattern::instance::GraphInstance::checkSourceAndSinkConnectionError(
    const traact::pattern::instance::ComponentID_PortName &source,
    const traact::pattern::instance::ComponentID_PortName &sink) {
    return checkSourceAndSinkConnectionError(source.first, source.second, sink.first, sink.second);
}
void traact::pattern::instance::GraphInstance::initializeGraphPortConnections(traact::pattern::instance::PortGroupInstance &port_group_instance) {

    for (auto &producer_port : port_group_instance.producer_ports) {
        producer_port.port_group_instance = &port_group_instance;
    }
    for (auto &consumer_port : port_group_instance.consumer_ports) {
        consumer_port.port_group_instance = &port_group_instance;
    }
}
void GraphInstance::forAllPatternInstances(const std::function<void(const PortGroupInstance &)> &func) const {
    for (auto &pattern_instance : pattern_instances) {
        for (auto &group_port_instances : pattern_instance.second->port_groups) {
            for (auto &group_port_instance : group_port_instances) {
                func(*group_port_instance);
            }
        }
    }

}
std::optional<std::string> GraphInstance::checkRunnable() {
    std::stringstream error_stream;
    for (auto &pattern_instance : pattern_instances) {
        for (auto &group_port_instances : pattern_instance.second->port_groups) {
            for (auto &group_port_instance : group_port_instances) {
                for(auto& consumer_port : group_port_instance->consumer_ports) {
                    if(!consumer_port.isConnected()){
                        error_stream << fmt::format("Input Port {0}:{1} is not connected\n", consumer_port.getId().first, consumer_port.getId().second);
                    }
                }
            }
        }
    }

    auto error_string = error_stream.str();
    if(error_string.empty()){
        return {};
    } else {
        return {error_string};
    }


}

}