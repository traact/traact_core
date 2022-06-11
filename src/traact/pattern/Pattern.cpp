/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "traact/pattern/Pattern.h"
#include "traact/util/Utils.h"

traact::pattern::Pattern::Pattern(std::string name,
                                  Concurrency concurrency,
                                  component::ComponentType component_type)
    : name(std::move(name)), concurrency(concurrency) {
    time_domain_component_type.emplace_back(component_type);
}

traact::pattern::Pattern::Pattern() : name("Invalid"), concurrency(Concurrency::SERIAL) {

}

traact::pattern::Pattern &traact::pattern::Pattern::addProducerPort(const std::string &name,
                                                                    const std::string &data_meta_type,
                                                                    int port_index, int time_domain) {

    if (util::vectorContainsName(producer_ports, name))
        throw std::invalid_argument("Name of port already in use, Component: " + name + " Port: " + name);

    if (port_index < 0)
        port_index = producer_ports.size();

    if (is_group_port)
        group_ports.back().producer_ports.emplace_back(name, data_meta_type, PortType::PRODUCER, port_index,time_domain);
    else
        producer_ports.emplace_back(name, data_meta_type, PortType::PRODUCER, port_index,time_domain);

    return *this;
}
traact::pattern::Pattern &traact::pattern::Pattern::addConsumerPort(const std::string &name,
                                                                    const std::string &data_meta_type, int port_index, int time_domain) {
    if (util::vectorContainsName(consumer_ports, name))
        throw std::invalid_argument("Name of port already in use, Component: " + name + " Port: " + name);

    if (port_index < 0)
        port_index = consumer_ports.size();

    if (is_group_port)
        group_ports.back().consumer_ports.emplace_back(name, data_meta_type, PortType::CONSUMER, port_index, time_domain);
    else
        consumer_ports.emplace_back(name, data_meta_type, PortType::CONSUMER, port_index,time_domain);
    return *this;
}

traact::pattern::Pattern::~Pattern() {

}

traact::pattern::Pattern &traact::pattern::Pattern::addStringParameter(const std::string &name,
                                                                       const std::string &default_value) {
    if (is_group_port) {
        auto &group_parameter = group_ports.back().parameter;
        group_parameter[name]["default"] = default_value;
        group_parameter[name]["value"] = default_value;
    } else {
        parameter[name]["default"] = default_value;
        parameter[name]["value"] = default_value;
    }

    return *this;
}
traact::pattern::Pattern &traact::pattern::Pattern::addParameter(const std::string &name,
                                                                 const std::string &default_value,
                                                                 const std::set<std::string> &enum_values) {
    if (is_group_port) {
        auto &group_parameter = group_ports.back().parameter;
        group_parameter[name]["default"] = default_value;
        group_parameter[name]["value"] = default_value;
        group_parameter[name]["enum_values"] = enum_values;
    } else {
        parameter[name]["default"] = default_value;
        parameter[name]["value"] = default_value;
        parameter[name]["enum_values"] = enum_values;
    }

    return *this;
}
traact::pattern::Pattern &traact::pattern::Pattern::addParameter(const std::string &name,
                                                                 const nlohmann::json &json_value) {

    if (is_group_port) {
        auto &group_parameter = group_ports.back().parameter;
        group_parameter[name]["json_value"] = json_value;
    } else {
        parameter[name]["json_value"] = json_value;
    }

    return *this;
}

traact::pattern::Pattern &traact::pattern::Pattern::addCoordinateSystem(const std::string &name, bool is_multi) {
    spatial::CoordinateSystem newCoord(name, is_multi);
    if (is_group_port) {
        group_ports.back().coordinate_systems.emplace(std::make_pair(name, std::move(newCoord)));
    } else {
        coordinate_systems_.emplace(std::make_pair(name, std::move(newCoord)));
    }

    return *this;
}
traact::pattern::Pattern &traact::pattern::Pattern::addEdge(const std::string &source,
                                                            const std::string &destination,
                                                            const std::string &port) {

    //TODO check for valid input
    if (is_group_port) {
        group_ports.back().edges.emplace(std::make_tuple(source, destination, port));
    } else {
        edges_.emplace(std::make_tuple(source, destination, port));
    }

    return *this;
}

traact::pattern::Pattern &traact::pattern::Pattern::beginPortGroup(const std::string &name, int min, int max) {
    is_group_port = true;
    group_ports.emplace_back(PortGroup());
    group_ports.back().name = name;
    return *this;
}

traact::pattern::Pattern &traact::pattern::Pattern::endPortGroup() {
    is_group_port = false;
    return *this;
}
traact::pattern::Pattern &traact::pattern::Pattern::addTimeDomain(traact::component::ComponentType component_type) {
    time_domain_component_type.emplace_back(component_type);
    return *this;
}