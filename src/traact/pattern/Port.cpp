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

traact::pattern::PortGroup::PortGroup(const std::string &name, int min, int max) : name(name), min(min), max(max) {}
traact::pattern::PortGroup::PortGroup() : name("Invalid"), min(-1), max(-1){

}
