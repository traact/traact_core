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

namespace traact::pattern {

// TODO how to fix loss of method chaining: https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern. But this would lead to a set of base classes, ...
struct TRAACT_CORE_EXPORT Pattern {
 public:
    typedef typename std::shared_ptr<Pattern> Ptr;
    Pattern();

    Pattern(std::string name, Concurrency concurrency);
    virtual ~Pattern();

    Pattern &addProducerPort(const std::string &name, const std::string &data_meta_type, int port_index = -1);
    Pattern &addConsumerPort(const std::string &name, const std::string &data_meta_type, int port_index = -1);

    Pattern &beginPortGroup(const std::string &name);
    Pattern &endPortGroup();

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

    std::map<std::string, spatial::CoordinateSystem> coordinate_systems_;
    // set of edges: source name, destination name, port name
    std::set<std::tuple<std::string, std::string, std::string> > edges_;

    std::string name;
    Concurrency concurrency;
    std::vector<Port> producer_ports;
    std::vector<Port> consumer_ports;
    std::vector<PortGroup> group_ports;
    nlohmann::json parameter;

 private:
    bool is_group_port{false};

};

}

#endif //TRAACT_INCLUDE_TRAACT_PATTERN_DATAFLOWPATTERN_H_

#pragma clang diagnostic pop