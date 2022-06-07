/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/
#include "traact/util/Logging.h"
#include "traact/pattern/Port.h"

#include "traact/pattern/Pattern.h"

traact::pattern::Port::Port(std::string name,
                            std::string datatype,
                            PortType port_type,
                            int port_index)
    : name(std::move(name)),
      datatype(std::move(datatype)),
      porttype(port_type),
      port_index(port_index) {

}

traact::pattern::Port::Port() :
    name("Invalid"),
    datatype("Invalid"),
    porttype(PortType::NONE),
    port_index(-1) {

}
const std::string &traact::pattern::Port::getName() const {
    return name;
}



