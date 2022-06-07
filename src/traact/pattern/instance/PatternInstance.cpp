/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#include "traact/pattern/instance/PatternInstance.h"
#include "traact/util/Utils.h"

traact::pattern::instance::PatternInstance::PatternInstance(std::string id,
                                                            traact::pattern::Pattern pattern_pointer,
                                                            traact::pattern::instance::GraphInstance *graph)
    : local_pattern(std::move(pattern_pointer)), instance_id(std::move(id)), parent_graph(graph) {
    for (const auto &consumer_port : pattern_pointer.consumer_ports) {
        PortInstance newPort(consumer_port, this);
        consumer_ports.emplace_back(std::move(newPort));
    }
    for (const auto &producer_port : pattern_pointer.producer_ports) {
        PortInstance newPort(producer_port, this);
        producer_ports.emplace_back(std::move(newPort));
    }

}

traact::pattern::instance::PatternInstance::PatternInstance()
    : local_pattern(), instance_id(""), parent_graph(nullptr) {

}

traact::pattern::instance::PatternInstance::~PatternInstance() {

}

traact::pattern::instance::PortInstance::ConstPtr traact::pattern::instance::PatternInstance::getProducerPort(const std::string &name) const {
    return util::vectorGetForName(producer_ports, name);
}
traact::pattern::instance::PortInstance::ConstPtr traact::pattern::instance::PatternInstance::getConsumerPort(const std::string &name) const {
    return util::vectorGetForName(consumer_ports, name);
}
std::set<traact::pattern::instance::PortInstance::ConstPtr> traact::pattern::instance::PatternInstance::getProducerPorts() const {
    std::set<traact::pattern::instance::PortInstance::ConstPtr> result;
    for (const auto &port : producer_ports) {
        result.emplace(&port);
    }
    return result;
}
std::set<traact::pattern::instance::PortInstance::ConstPtr> traact::pattern::instance::PatternInstance::getConsumerPorts() const {
    std::set<traact::pattern::instance::PortInstance::ConstPtr> result;
    for (const auto &port : consumer_ports) {
        result.emplace(&port);
    }
    return result;
}
traact::Concurrency traact::pattern::instance::PatternInstance::getConcurrency() const {
    return local_pattern.concurrency;
}
traact::pattern::instance::PortInstance::ConstPtr traact::pattern::instance::PatternInstance::getPort(const std::string &name) const {

    if (util::vectorContainsName(producer_ports, name))
        return util::vectorGetForName(producer_ports, name);
    if (util::vectorContainsName(consumer_ports, name))
        return util::vectorGetForName(consumer_ports, name);
    return nullptr;
}
std::string traact::pattern::instance::PatternInstance::getPatternName() const {
    return local_pattern.name;
}

