#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nodiscard" // class should be used in c++ 11 inferfaces
/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_INCLUDE_TRAACT_PATTERN_DATAFLOWPORT_H_
#define TRAACT_INCLUDE_TRAACT_PATTERN_DATAFLOWPORT_H_

#include <string>
#include <set>
#include <memory>
#include <traact/traact_core_export.h>
#include <traact/pattern/CoordinateSystem.h>
#include <vector>
#include <nlohmann/json.hpp>

namespace traact::pattern {

enum class TRAACT_CORE_EXPORT PortType {
    NONE = 0,
    Producer,
    Consumer
};

//typedef typename std::pair<std::string, std::string> ComponentName_PortName;


struct TRAACT_CORE_EXPORT Port {
    Port();

    Port(std::string name, std::string datatype, PortType port_type, int port_index);

    const std::string &getName() const;

    std::string name;
    std::string datatype;
    PortType porttype;
    int port_index;

};

struct TRAACT_CORE_EXPORT PortGroup {
    std::vector<Port> producer_ports;
    std::vector<Port> consumer_ports;

    std::map<std::string, spatial::CoordinateSystem> coordinate_systems_;
    // set of edges: source name, destination name, port name
    std::set<std::tuple<std::string, std::string, std::string> > edges_;
    nlohmann::json parameter;
    std::string name;
};
}

#endif //TRAACT_INCLUDE_TRAACT_PATTERN_DATAFLOWPORT_H_

#pragma clang diagnostic pop