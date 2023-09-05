/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include <traact/pattern/instance/PatternInstance.h>
#include <traact/util/Utils.h>

namespace traact::pattern::instance {

PatternInstance::PatternInstance(std::string t_instance_id,
                                                            Pattern pattern,
                                                            GraphInstance *graph)
    : local_pattern(std::move(pattern)), instance_id(std::move(t_instance_id)), parent_graph(graph) {

    display_name = instance_id;
    port_groups.resize(local_pattern.port_groups.size());
    auto default_group_instance = std::make_shared<PortGroupInstance>(local_pattern.port_groups.at(0), this, 0);


    port_groups[kDefaultPortGroupIndex].emplace_back(default_group_instance);

    for (const auto &port_group : local_pattern.port_groups) {
        port_group_name_to_index.emplace(port_group.name, port_group.group_index);
    }

}

PortInstance::ConstPtr PatternInstance::getProducerPort(const std::string &name) const {
    // if name is from the default port group, then when connecting the port name does not contain the port group name extension
    // check default group directly first, then rest
    for (const auto &default_port : port_groups.at(kDefaultPortGroupIndex)[0]->producer_ports) {
        if (default_port.port.name == name) {
            return &default_port;
        }
    }

    for (const auto &group_port_list : port_groups) {
        for (const auto &group_port_instance : group_port_list) {
            auto *result = util::vectorGetForName(group_port_instance->producer_ports, name);
            if (result != nullptr) {
                return result;
            }
        }
    }
    return nullptr;
}
PortInstance::ConstPtr PatternInstance::getConsumerPort(const std::string &name) const {
    // if name is from the default port group, then when connecting the port name does not contain the port group name extension
    // check default group directly first, then rest
    for (const auto &default_port : port_groups.at(kDefaultPortGroupIndex)[0]->consumer_ports) {
        if (default_port.port.name == name) {
            return &default_port;
        }
    }

    for (const auto &group_port_list : port_groups) {
        for (const auto &group_port_instance : group_port_list) {
            auto *result = util::vectorGetForName(group_port_instance->consumer_ports, name);
            if (result != nullptr) {
                return result;
            }
        }
    }
    return nullptr;
}
PortInstance::Ptr PatternInstance::getConsumerPort(const std::string &name) {
    // if name is from the default port group, then when connecting the port name does not contain the port group name extension
    // check default group directly first, then rest
    for (auto &default_port : port_groups.at(kDefaultPortGroupIndex)[0]->consumer_ports) {
        if (default_port.port.name == name) {
            return &default_port;
        }
    }
    for (auto &group_port_list : port_groups) {
        for (auto &group_port_instance : group_port_list) {
            auto *result = util::vectorGetForName(group_port_instance->consumer_ports, name);
            if (result != nullptr) {
                return result;
            }
        }
    }
    return nullptr;
}

std::vector<PortInstance::ConstPtr> PatternInstance::getProducerPorts(
    int global_time_domain) const {
    std::vector<PortInstance::ConstPtr> result;
    for (const auto &port_group : port_groups) {
        for (const auto &port_group_instance : port_group) {
            for (const auto &port_instance : port_group_instance->producer_ports) {
                if (port_instance.getTimeDomain() == global_time_domain) {
                    result.emplace_back(&port_instance);
                }
            }
        }
    }
    return result;
}
std::vector<PortInstance::ConstPtr> PatternInstance::getConsumerPorts(
    int time_domain) const {
    std::vector<PortInstance::ConstPtr> result;

    for (const auto &port_group : port_groups) {
        for (const auto &port_group_instance : port_group) {
            for (const auto &port_instance : port_group_instance->consumer_ports) {
                if (port_instance.getTimeDomain() == time_domain)
                    result.emplace_back(&port_instance);
            }
        }
    }
    return result;
}
Concurrency PatternInstance::getConcurrency(int time_domain) const {
    return local_pattern.concurrency.at(time_domain);
}
PortInstance::ConstPtr PatternInstance::getPort(const std::string &name) const {

    auto result = getProducerPort(name);
    if (result == nullptr) {
        result = getConsumerPort(name);
    }
    return result;
}
std::string PatternInstance::getPatternName() const {
    return local_pattern.name;
}
component::ComponentType PatternInstance::getComponentType(int time_domain) const {
    return local_pattern.time_domain_component_type.at(time_domain);
}
LocalConnectedOutputPorts PatternInstance::getOutputPortsConnected(
    int time_domain) const {
    LocalConnectedOutputPorts result;
    for (const auto &port_group : port_groups) {
        for (const auto &port_group_instance : port_group) {
            for (const auto &port_instance : port_group_instance->producer_ports) {
                if (port_instance.getTimeDomain() == time_domain)
                    result.emplace_back(port_instance.isConnected());
            }
        }
    }
    return result;
}
int PatternInstance::getPortGroupCount(const std::string &port_group_name) const {
    auto port_group_index = getPortGroupIndex(port_group_name);
    return port_groups.at(port_group_index).size();
}
PortGroupInfo PatternInstance::getPortGroupInfo(const std::string &port_group_name) const {
    auto port_group_index = getPortGroupIndex(port_group_name);

    return {port_group_index, getPortGroupCount(port_group_name)};
}
PortGroupInstance &PatternInstance::instantiatePortGroup(const std::string &port_group_name) {
    auto *group_blue_print = util::vectorGetForName(local_pattern.port_groups, port_group_name);
    if (group_blue_print == nullptr) {
        throw std::invalid_argument(fmt::format("unknown pattern group: {0} in pattern {1}",
                                                port_group_name,
                                                local_pattern.name));
    }

    auto port_group_index = getPortGroupIndex(port_group_name);

    auto &group_list = port_groups[port_group_index];

    auto new_port_group = std::make_shared<PortGroupInstance>(*group_blue_print, this, group_list.size());
    group_list.emplace_back(new_port_group);

    return *new_port_group;
}
int PatternInstance::getPortGroupIndex(const std::string &group_name) const {
    auto find_result = port_group_name_to_index.find(group_name);
    if (find_result == port_group_name_to_index.end()) {
        throw std::invalid_argument(fmt::format("unknown port group {0} in pattern {1}",
                                                group_name,
                                                local_pattern.name));
    }
    return find_result->second;
}
int PatternInstance::getTimeDomainCount() const {
    return static_cast<int>(local_pattern.time_domain_component_type.size());
}
bool PatternInstance::isInTimeDomain(int global_time_d) const {
    auto is_part = std::find(local_to_global_time_domain.begin(), local_to_global_time_domain.end(), global_time_d);
    return is_part != local_to_global_time_domain.end();
}
std::tuple<int, int, int> PatternInstance::getPortGroupOffset(int port_group_index, PortType port_type, int local_time_domain) const {
    const auto& port_group_list = port_groups[port_group_index];

    if(port_group_list.empty())
        return {-1,0, 0};
    int group_offset = port_group_list.front()->getPortGroupStartIndex(local_time_domain, port_type);
    int group_port_count = port_group_list.front()->getPortCount(local_time_domain, port_type);
    return {group_offset, group_port_count,port_group_list.size()};
}
const std::string &PatternInstance::getName() const {
    return instance_id;
}

}