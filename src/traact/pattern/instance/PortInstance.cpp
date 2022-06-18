/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include <utility>

#include "traact/util/Logging.h"
#include "traact/pattern/instance/PortInstance.h"
#include "traact/pattern/instance/PatternInstance.h"
#include "GraphInstance.h"

namespace traact::pattern::instance {

PortInstance::PortInstance(Port t_port, PortGroupInstance *t_port_group_instance)
    : port(std::move(t_port)), port_group_instance(t_port_group_instance) {

}

ComponentID_PortName PortInstance::getId() const {
    return std::make_pair(port_group_instance->pattern_instance->instance_id, getName());
}

const std::string PortInstance::getName() const {
    return fmt::format("{0}_{1}_{2}",
                       port_group_instance->port_group.name,
                       port_group_instance->port_group_instance_index,
                       port.name);
}
const std::string &PortInstance::getDataType() const {
    return port.datatype;
}
int PortInstance::getPortIndex() const {
    int start_index = port_group_instance->getPortGroupStartIndex(port.time_domain, port.port_type);
    return start_index + port.port_index;
}
std::set<PortInstance::ConstPtr> PortInstance::connectedToPtr() const {
    return port_group_instance->pattern_instance->parent_graph->connectedToPtr(getId());
}

bool PortInstance::isConnected() const {
    return !connectedToPtr().empty();
}

PortType PortInstance::getPortType() const {
    return port.port_type;
}
int PortInstance::getTimeDomain() const {
    return port_group_instance->pattern_instance->local_to_global_time_domain.at(port.time_domain);
}
std::string PortGroupInstance::getProducerPortName(const std::string &internal_port_name) const {
    return fmt::format("{0}_{1}_{2}", port_group.name, port_group_instance_index, internal_port_name);
}
std::string PortGroupInstance::getConsumerPortName(const std::string &internal_port_name) const {
    return fmt::format("{0}_{1}_{2}", port_group.name, port_group_instance_index, internal_port_name);
}
PortGroupInstance::PortGroupInstance(PortGroup t_port_group,
                                     PatternInstance *t_pattern_instance,
                                     int t_port_group_instance_id)
    : port_group(std::move(t_port_group)),
      pattern_instance(t_pattern_instance),
      port_group_instance_index(t_port_group_instance_id) {

    for (const auto &consumer_port : port_group.consumer_ports) {
        consumer_ports.emplace_back(consumer_port, this);
    }
    for (const auto &producer_port : port_group.producer_ports) {
        producer_ports.emplace_back(producer_port, this);
    }
}
int PortGroupInstance::getPortCount(int local_time_domain, PortType port_type) const {
    int result = 0;
    for (const auto &port : getPortList(port_type)) {
        if (port.port.time_domain == local_time_domain) {
            ++result;
        }
    }
    return result;
}
int PortGroupInstance::getPortGroupStartIndex(int local_time_domain, PortType port_type) const {
    int result = 0;
    for (int group_index = 0; group_index < pattern_instance->port_group_name_to_index_.size(); ++group_index) {
        auto group_name = pattern_instance->local_pattern.port_groups[group_index].name;
        const auto &port_group_list = pattern_instance->port_groups_[group_index];

        for (const auto &port_group_instance : port_group_list) {
            if (group_name == port_group.name
                && port_group_instance->port_group_instance_index == port_group_instance_index) {
                return result;
            }
            result += port_group_instance->getPortCount(local_time_domain, port_type);
        }
    }

    return result;
}
const std::vector<PortInstance> &PortGroupInstance::getPortList(PortType port_type) const {
    if(port_type == PortType::CONSUMER){
        return consumer_ports;
    } else if(port_type == PortType::PRODUCER){
        return producer_ports;
    } else {
        throw std::invalid_argument("unknown or NONE port type in PortGroupInstance::getPortList");
    }
}

}