/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "traact/util/Logging.h"
#include "traact/pattern/instance/PortInstance.h"
#include "traact/pattern/instance/PatternInstance.h"
#include "GraphInstance.h"
traact::pattern::instance::PortInstance::PortInstance(traact::pattern::Port port,
                                                      traact::pattern::instance::PatternInstance *pattern_instance)
    : port(port), pattern_instance(pattern_instance) {

}

traact::pattern::instance::PortInstance::PortInstance() : port(), pattern_instance(nullptr) {

}

traact::pattern::instance::ComponentID_PortName traact::pattern::instance::PortInstance::getID() const {
    return std::make_pair(pattern_instance->instance_id, getName());
}

const std::string &traact::pattern::instance::PortInstance::getName() const {
    return port.name;
}
const std::string &traact::pattern::instance::PortInstance::getDataType() const {
    return port.datatype;
}
int traact::pattern::instance::PortInstance::getPortIndex() const {
    return port.port_index;
}
std::set<traact::pattern::instance::PortInstance::ConstPtr> traact::pattern::instance::PortInstance::connectedToPtr() const {
    return pattern_instance->parent_graph->connectedToPtr(getID());
}

bool traact::pattern::instance::PortInstance::IsConnected() const {
    return !connectedToPtr().empty();
}

traact::pattern::PortType traact::pattern::instance::PortInstance::GetPortType() const {
    return port.porttype;
}
