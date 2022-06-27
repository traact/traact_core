/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "traact/pattern/Pattern.h"
#include "traact/util/Utils.h"

namespace traact::pattern {

Pattern::Pattern(std::string name,
                 Concurrency t_concurrency,
                 component::ComponentType component_type)
    : name(std::move(name)) {
    concurrency.emplace_back(t_concurrency);
    time_domain_component_type.emplace_back(component_type);
    port_groups.emplace_back(kDefaultPortGroupName, kDefaultPortGroupIndex, 1, 1);
}

Pattern &Pattern::addPort(std::string port_name,
                          int time_domain,
                          PortType port_type,
                          std::string type_name,
                          PortGroup &port_group) {
    checkName(port_name, port_group);
    if (port_type == PortType::PRODUCER) {
        int port_index = port_group.getPortCount(PortType::PRODUCER, time_domain);
        port_group.producer_ports.emplace_back(port_name,
                                               type_name,
                                               PortType::PRODUCER,
                                               port_index,
                                               time_domain);
    } else {
        int port_index = port_group.getPortCount(PortType::CONSUMER, time_domain);
        port_group.consumer_ports.emplace_back(port_name,
                                               type_name,
                                               PortType::CONSUMER,
                                               port_index,
                                               time_domain);
    }
    return *this;
}

Pattern &Pattern::addProducerPort(const std::string &port_name,
                                  const std::string &data_type_name,
                                  int port_index, int time_domain) {
    if (is_group_port) {
        auto &port_group = port_groups.back();
        return addPort(port_name, time_domain, PortType::PRODUCER, data_type_name, port_group);
    } else {
        auto &port_group = port_groups.front();
        return addPort(port_name, time_domain, PortType::PRODUCER, data_type_name, port_group);
    }
}
Pattern &Pattern::addConsumerPort(const std::string &port_name,
                                  const std::string &data_type_name,
                                  int port_index,
                                  int time_domain) {
    if (is_group_port) {
        auto &port_group = port_groups.back();
        return addPort(port_name, time_domain, PortType::CONSUMER, data_type_name, port_group);
    } else {
        auto &port_group = port_groups.front();
        return addPort(port_name, time_domain, PortType::CONSUMER, data_type_name, port_group);
    }
}

Pattern &Pattern::addStringParameter(const std::string &parameter_name,
                                     const std::string &default_value) {
    nlohmann::json *parameter;
    if (is_group_port) {
        parameter = &port_groups.back().parameter;
    } else {
        parameter = &port_groups.front().parameter;
    }

    (*parameter)[parameter_name]["default"] = default_value;
    (*parameter)[parameter_name]["value"] = default_value;

    return *this;
}
Pattern &Pattern::addParameter(const std::string &parameter_name,
                               const std::string &default_value,
                               const std::set<std::string> &enum_values) {
    nlohmann::json *parameter;
    if (is_group_port) {
        parameter = &port_groups.back().parameter;
    } else {
        parameter = &port_groups.front().parameter;
    }
    (*parameter)[parameter_name]["default"] = default_value;
    (*parameter)[parameter_name]["value"] = default_value;
    (*parameter)[parameter_name]["enum_values"] = enum_values;

    return *this;
}
Pattern &Pattern::addParameter(const std::string &parameter_name,
                               const nlohmann::json &json_value) {

    nlohmann::json *parameter;
    if (is_group_port) {
        parameter = &port_groups.back().parameter;
    } else {
        parameter = &port_groups.front().parameter;
    }
    (*parameter)[parameter_name]["json_value"] = json_value;

    return *this;
}

Pattern &Pattern::addCoordinateSystem(const std::string &coordinate_system_name,
                                      bool is_multi) {
    spatial::CoordinateSystem newCoord(coordinate_system_name, is_multi);
    if (is_group_port) {
        port_groups.back().coordinate_systems.emplace(std::make_pair(coordinate_system_name, std::move(newCoord)));
    } else {
        port_groups.front().coordinate_systems.emplace(std::make_pair(coordinate_system_name, std::move(newCoord)));
    }

    return *this;
}
Pattern &Pattern::addEdge(const std::string &source,
                          const std::string &destination,
                          const std::string &port) {

    //TODO check for valid input
    if (is_group_port) {
        port_groups.back().edges.emplace(std::make_tuple(source, destination, port));
    } else {
        port_groups.front().edges.emplace(std::make_tuple(source, destination, port));
    }

    return *this;
}

Pattern &Pattern::beginPortGroup(const std::string &port_group_name,
                                 int min,
                                 int max) {
    is_group_port = true;
    int group_index = static_cast<int>(port_groups.size());
    port_groups.emplace_back(port_group_name, group_index, min, max);
    return *this;
}

Pattern &Pattern::endPortGroup() {
    is_group_port = false;
    return *this;
}
Pattern &Pattern::addTimeDomain(traact::component::ComponentType component_type) {
    time_domain_component_type.emplace_back(component_type);
    return *this;
}
void Pattern::checkName(const std::string &name, const PortGroup &port_group) const {
    if (util::vectorContainsName(port_group.producer_ports, name))
        throw std::invalid_argument("Name of port already in use, Component: " + name + " Port: " + name);
    if (util::vectorContainsName(port_group.consumer_ports, name))
        throw std::invalid_argument("Name of port already in use, Component: " + name + " Port: " + name);
}
Pattern::Pattern() {
    port_groups.emplace_back("Default", 0, 1, 1);
}

}