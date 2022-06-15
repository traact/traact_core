/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/
#include "traact/util/Logging.h"
#include "traact/pattern/Port.h"

#include "traact/pattern/Pattern.h"

traact::pattern::Port::Port(std::string t_name,
                            std::string t_datatype,
                            PortType t_port_type,
                            int t_port_index,
                            int t_time_domain)
    : name(std::move(t_name)),
      datatype(std::move(t_datatype)),
      port_type(t_port_type),
      port_index(t_port_index), time_domain(t_time_domain) {

}

traact::pattern::Port::Port() :
    name("Invalid"),
    datatype("Invalid"),
    port_type(PortType::NONE),
    port_index(-1) {

}
const std::string &traact::pattern::Port::getName() const {
    return name;
}

traact::pattern::PortGroup::PortGroup(const std::string &name,
                                      int group_index,
                                      int min,
                                      int max) : name(name), min(min), max(max), group_index(group_index) {}
traact::pattern::PortGroup::PortGroup() : name("Invalid"), min(-1), max(-1){

}
const std::string &traact::pattern::PortGroup::getName() const {
    return name;
}
int traact::pattern::PortGroup::getPortCount(traact::pattern::PortType type, int time_domain) {
    int result = 0;
    if(type == PortType::PRODUCER){
        for (const auto& port : producer_ports) {
            if(port.time_domain == time_domain){
                ++result;
            }
        }
    } else if(type == PortType::CONSUMER){
        for (const auto& port : consumer_ports) {
            if(port.time_domain == time_domain){
                ++result;
            }
        }
    } else {
        throw std::invalid_argument("invalid port type in PortGroup::getPortCount");
    }
    return result;
}
