/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_PATTERN_PORT_H_
#define TRAACT_CORE_SRC_TRAACT_PATTERN_PORT_H_

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
    PRODUCER,
    CONSUMER
};

//typedef typename std::pair<std::string, std::string> ComponentName_PortName;


struct TRAACT_CORE_EXPORT Port {
    Port();

    Port(std::string t_name,
         std::string t_datatype,
         PortType t_port_type,
         int t_port_index,
         int t_time_domain);

    [[nodiscard]] const std::string &getName() const;

    std::string name{"invalid"};
    std::string datatype{"invalid"};
    PortType port_type{PortType::NONE};
    int port_index{-1};
    int time_domain{-1};

};

struct TRAACT_CORE_EXPORT PortGroup {
    PortGroup();
    PortGroup(const std::string &name,
              int group_index,
              int min,
              int max);

    [[nodiscard]] const std::string &getName() const;

    std::string name{"invalid"};
    int group_index{-1};
    int min{-1};
    int max{-1};
    nlohmann::json parameter{};

    std::vector<Port> producer_ports{};
    std::vector<Port> consumer_ports{};
    std::map<std::string, spatial::CoordinateSystem> coordinate_systems{};
    // set of edges: source name, destination name, port name
    std::set<std::tuple<std::string, std::string, std::string> > edges{};
    int getPortCount(PortType type, int time_domain);
};
}

#endif //TRAACT_CORE_SRC_TRAACT_PATTERN_PORT_H_