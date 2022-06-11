#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nodiscard"
/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/
#ifndef TRAACT_INCLUDE_TRAACT_PATTERN_DATAFLOWPATTERN_H_
#define TRAACT_INCLUDE_TRAACT_PATTERN_DATAFLOWPATTERN_H_

#include <string>
#include <set>
#include <map>
#include <nlohmann/json.hpp>
#include <traact/pattern/Port.h>
#include <traact/pattern/CoordinateSystem.h>
#include <traact/datatypes.h>
#include <traact/util/Utils.h>

namespace traact::pattern {

// how to fix loss of method chaining: https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern. But this would lead to a set of base classes, ...
struct TRAACT_CORE_EXPORT Pattern {
 public:
    typedef typename std::shared_ptr<Pattern> Ptr;
    Pattern();

    Pattern(std::string name,
            Concurrency concurrency,
            component::ComponentType component_type);
    virtual ~Pattern();

    Pattern &addProducerPort(const std::string &name, const std::string &data_meta_type, int port_index = -1, int time_domain = 0);
    Pattern &addConsumerPort(const std::string &name, const std::string &data_meta_type, int port_index = -1, int time_domain = 0);
    traact::pattern::Pattern &beginPortGroup(const std::string &name, int min=0, int max = std::numeric_limits<int>::max());
    Pattern &endPortGroup();

    template<typename Port> Pattern &addProducerPort(std::string name, int time_domain = 0){
        if (util::vectorContainsName(producer_ports, name))
            throw std::invalid_argument("Name of port already in use, Component: " + name + " Port: " + name);


        if (is_group_port)
            group_ports.back().producer_ports.emplace_back(name, Port::Header::MetaType, PortType::PRODUCER, Port::PortIdx,time_domain);
        else
            producer_ports.emplace_back(name, Port::Header::MetaType, PortType::PRODUCER, Port::PortIdx,time_domain);

        return *this;
    }

    template<typename Port> Pattern &addConsumerPort(std::string name, int time_domain = 0){
        if (util::vectorContainsName(producer_ports, name))
            throw std::invalid_argument("Name of port already in use, Component: " + name + " Port: " + name);


        if (is_group_port)
            group_ports.back().consumer_ports.emplace_back(name, Port::Header::MetaType, PortType::CONSUMER, Port::PortIdx, time_domain);
        else
            consumer_ports.emplace_back(name, Port::Header::MetaType, PortType::CONSUMER, Port::PortIdx, time_domain);

        return *this;
    }

    template<typename T>
    Pattern &addParameter(std::string name,
                          T default_value,
                          T min_value = std::numeric_limits<T>::min(),
                          T max_value = std::numeric_limits<T>::max()) {
        parameter[name]["default"] = default_value;
        parameter[name]["value"] = default_value;
        parameter[name]["min_value"] = min_value;
        parameter[name]["max_value"] = max_value;
        return *this;
    }
    Pattern &addStringParameter(const std::string &name,
                                const std::string &default_value);
    Pattern &addParameter(const std::string &name,
                          const std::string &default_value, const std::set<std::string> &enum_values);
    Pattern &addParameter(const std::string &name,
                          const nlohmann::json &json_value);

    /**
    * Add a node to spatial relationship graph
    * @param name name of node
    */
    Pattern &addCoordinateSystem(const std::string &name, bool is_multi = false);

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


    std::string name;
    Concurrency concurrency;
    std::map<std::string, spatial::CoordinateSystem> coordinate_systems_;
    // set of edges: source name, destination name, port name
    std::set<std::tuple<std::string, std::string, std::string> > edges_;

    std::vector<Port> producer_ports;
    std::vector<Port> consumer_ports;
    std::vector<PortGroup> group_ports;
    nlohmann::json parameter;
    std::vector<component::ComponentType> time_domain_component_type;

 private:
    bool is_group_port{false};

};

}

#endif //TRAACT_INCLUDE_TRAACT_PATTERN_DATAFLOWPATTERN_H_

#pragma clang diagnostic pop