/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_USEREVENT_H_
#define TRAACT_CORE_SRC_TRAACT_USEREVENT_H_

#include "traact/traact_plugins.h"
#include "traact/traact_core_export.h"
#include "traact/buffer/DataFactory.h"
namespace traact::userEvent {
using UserEvent = int;


struct TRAACT_CORE_EXPORT UserEventHeader {
    using NativeType = traact::userEvent::UserEvent;
    static constexpr const char *NativeTypeName{"traact::userEvent::UserEvent"};
    static constexpr const char *MetaType{"user:event"};
    static constexpr const size_t size = sizeof(traact::userEvent::UserEvent);
    UserEventHeader() = default;
    UserEventHeader(UserEventHeader const& value) = default;
    UserEventHeader& operator=(UserEventHeader const& value) = default;
    UserEventHeader(UserEventHeader && value) = default;
    UserEventHeader& operator=(UserEventHeader && value) = default;
};

class UserEventHeaderFactory : public traact::buffer::TemplatedDefaultDataFactory<UserEventHeader> {
    TRAACT_PLUGIN_ENABLE(traact::buffer::DataFactory, traact::buffer::TemplatedDefaultDataFactory<UserEventHeader>)
};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_USEREVENT_H_
