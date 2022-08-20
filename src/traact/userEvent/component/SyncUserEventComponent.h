/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_USEREVENT_COMPONENT_SYNCUSEREVENTCOMPONENT_H_
#define TRAACT_CORE_SRC_TRAACT_USEREVENT_COMPONENT_SYNCUSEREVENTCOMPONENT_H_

#include "traact/userEvent/UserEvent.h"
#include "traact/traact.h"

namespace traact::component {

class SyncUserEventComponent : public Component {
 public:
    using OutEvent = buffer::PortConfig<userEvent::UserEventHeader, 0>;
    SyncUserEventComponent(const std::string &name);

    [[nodiscard]] static traact::pattern::Pattern::Ptr GetPattern();

    bool processTimePoint(buffer::ComponentBuffer &data) override;
    void fireEvent(int event);
 private:
    std::mutex event_lock_;
    std::optional<int> event_{};

};

}


#endif //TRAACT_CORE_SRC_TRAACT_USEREVENT_COMPONENT_SYNCUSEREVENTCOMPONENT_H_
