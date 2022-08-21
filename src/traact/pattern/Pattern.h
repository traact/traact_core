/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/
#ifndef TRAACT_CORE_SRC_TRAACT_PATTERN_PATTERN_H_
#define TRAACT_CORE_SRC_TRAACT_PATTERN_PATTERN_H_

#include <string>
#include <set>
#include <map>
#include <nlohmann/json.hpp>
#include <traact/pattern/Port.h>
#include <traact/pattern/CoordinateSystem.h>
#include <traact/datatypes.h>
#include <traact/util/Utils.h>

namespace traact::pattern {

struct TRAACT_CORE_EXPORT Pattern {
 public:
    using Ptr = std::shared_ptr<Pattern>;
    Pattern();
    Pattern(Pattern &) = default;
    Pattern(Pattern &&) = default;
    Pattern(std::string name,
            Concurrency t_concurrency,
            component::ComponentType component_type);
    ~Pattern() = default;
    Pattern &operator=(const Pattern &) = default;
    Pattern &operator=(Pattern &&) = default;

    Pattern &addPort(std::string port_name,
                     int time_domain,
                     PortType port_type,
                     std::string type_name,
                     PortGroup &port_group);

    Pattern &addProducerPort(const std::string &port_name,
                             const std::string &data_type_name,
                             int port_index = -1,
                             int time_domain = 0);
    Pattern &addConsumerPort(const std::string &port_name,
                             const std::string &data_type_name,
                             int port_index = -1,
                             int time_domain = 0);

    traact::pattern::Pattern &beginPortGroup(const std::string &port_group_name,
                                             int min = 0,
                                             int max = std::numeric_limits<int>::max());
    Pattern &endPortGroup();

    template<typename Port>
    Pattern &addPort(std::string port_name, int time_domain, PortType port_type, PortGroup &port_group) {
        checkName(port_name, port_group);
        if (port_type == PortType::PRODUCER) {
            port_group.producer_ports.emplace_back(port_name,
                                                   Port::Header::NativeTypeName,
                                                   PortType::PRODUCER,
                                                   Port::PortIdx,
                                                   time_domain);
        } else {
            port_group.consumer_ports.emplace_back(port_name,
                                                   Port::Header::NativeTypeName,
                                                   PortType::CONSUMER,
                                                   Port::PortIdx,
                                                   time_domain);
        }
        return *this;
    }

    template<typename Port>
    Pattern &addProducerPort(std::string port_name, int time_domain = 0) {
        if (is_group_port) {
            auto &port_group = port_groups.back();
            return addPort<Port>(port_name, time_domain, PortType::PRODUCER, port_group);
        } else {
            auto &port_group = port_groups.front();
            return addPort<Port>(port_name, time_domain, PortType::PRODUCER, port_group);
        }
    }

    template<typename Port>
    Pattern &addConsumerPort(std::string port_name, int time_domain = 0) {
        if (is_group_port) {
            auto &port_group = port_groups.back();
            return addPort<Port>(port_name, time_domain, PortType::CONSUMER, port_group);
        } else {
            auto &port_group = port_groups.front();
            return addPort<Port>(port_name, time_domain, PortType::CONSUMER, port_group);
        }
    }

    template<typename T>
    Pattern &addParameter(const std::string &parameter_name,
                          T default_value,
                          T min_value = T(0),
                          T max_value = T(1000)) {

        nlohmann::json *parameter;
        if (is_group_port) {
            parameter = &port_groups.back().parameter;
        } else {
            parameter = &port_groups.front().parameter;
        }
        (*parameter)[parameter_name]["default"] = default_value;
        (*parameter)[parameter_name]["value"] = default_value;
        (*parameter)[parameter_name]["min_value"] = min_value;
        (*parameter)[parameter_name]["max_value"] = max_value;
        return *this;
    }

    Pattern &addStringParameter(const std::string &parameter_name,
                                const std::string &default_value);
    Pattern &addParameter(const std::string &parameter_name,
                          const std::string &default_value, const std::set<std::string> &enum_values);
    Pattern &addParameter(const std::string &parameter_name,
                          const nlohmann::json &json_value);

    /**
    * Add a node to spatial relationship graph
    * @param coordinate_system_name name of node
    */
    Pattern &addCoordinateSystem(const std::string &coordinate_system_name, bool is_multi = false);

    /**
     * Add edge between two coordinate systems.
     * Depending on port for:
     * -producer or consumer
     * -data meta type
     * @param source origin of transformation e.g. camera
     * @param destination destination of transformation e.g. marker
     * @param port transformation as data meta type
     */
    Pattern &addEdge(const std::string &source, const std::string &destination, const std::string &port);

    Pattern &addTimeDomain(component::ComponentType component_type);

    std::string name{"invalid"};
    std::vector<Concurrency> concurrency{};
    std::vector<component::ComponentType> time_domain_component_type{};
    std::vector<PortGroup> port_groups{};

 private:
    bool is_group_port{false};
    void checkName(const std::string &name, const PortGroup &port_group) const;

};

}

#endif //TRAACT_INCLUDE_TRAACT_PATTERN_DATAFLOWPATTERN_H_