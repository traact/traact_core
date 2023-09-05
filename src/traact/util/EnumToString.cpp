/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include <traact/util/EnumToString.h>
#include <traact/datatypes.h>
#include <traact/component/ComponentTypes.h>

namespace traact {
template<>
std::string toString<EventType>(const EventType &value) {
    switch (value) {
        case traact::EventType::INVALID:return "INVALID";
        case traact::EventType::CONFIGURE:return "CONFIGURE";
        case traact::EventType::START:return "START";
        case traact::EventType::DATA:return "DATA";
        case EventType::PARAMETER_CHANGE:return "PARAMETER_CHANGE";
        case traact::EventType::STOP:return "STOP";
        case traact::EventType::TEARDOWN:return "TEARDOWN";
        case traact::EventType::DATAFLOW_NO_OP:return "DATAFLOW_NO_OP";
        case traact::EventType::DATAFLOW_STOP:return "DATAFLOW_STOP";
        default:return "unknown EventType value";

    }
}

template<>
std::string toString<Concurrency>(const Concurrency &value) {
    switch (value) {

        case Concurrency::UNLIMITED:return "UNLIMITED";
        case Concurrency::SERIAL:return "SERIAL";
        default:return "unknown Concurrency value";
    }
}

template<>
std::string toString<component::ComponentType>(const component::ComponentType &value) {
    switch (value) {

        case component::ComponentType::INVALID:return "INVALID";
        case component::ComponentType::ASYNC_SOURCE:return "ASYNC_SOURCE";
        case component::ComponentType::INTERNAL_SYNC_SOURCE: return "INTERNAL_SYNC_SOURCE";
        case component::ComponentType::SYNC_SOURCE:return "SYNC_SOURCE";
        case component::ComponentType::SYNC_FUNCTIONAL:return "SYNC_FUNCTIONAL";
        case component::ComponentType::ASYNC_FUNCTIONAL:return "ASYNC_FUNCTIONAL";
        case component::ComponentType::SYNC_SINK:return "SYNC_SINK";
        case component::ComponentType::ASYNC_SINK:return "ASYNC_SINK";
        default:return "unknown ComponentType value";

    }
}

} // traact