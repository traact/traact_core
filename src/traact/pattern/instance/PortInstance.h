/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACT_INCLUDE_TRAACT_PATTERN_INSTANTIATEDPORT_H_
#define TRAACT_INCLUDE_TRAACT_PATTERN_INSTANTIATEDPORT_H_

#include <traact/pattern/Port.h>
#include <traact/traact_core_export.h>
#include <optional>
namespace traact::pattern::instance {

class TRAACT_CORE_EXPORT PatternInstance;

//
typedef typename std::pair<std::string, std::string> ComponentID_PortName;

struct TRAACT_CORE_EXPORT PortInstance {
    typedef PortInstance *Ptr;
    typedef const PortInstance *ConstPtr;

    PortInstance();
    PortInstance(Port port, PatternInstance *pattern_instance);

    const std::string &getName() const;

    const std::string &getDataType() const;

    int getPortIndex() const;

    std::set<traact::pattern::instance::PortInstance::ConstPtr> connectedToPtr() const;

    ComponentID_PortName getID() const;

    bool IsConnected() const;

    PortType GetPortType() const;

    Port port;
    bool is_active;
    ComponentID_PortName connected_to;
    PatternInstance *pattern_instance;

};

}

#endif //TRAACT_INCLUDE_TRAACT_PATTERN_INSTANTIATEDPORT_H_
