/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include <traact/userEvent/component/SyncUserEventComponent.h>

namespace traact::component {

SyncUserEventComponent::SyncUserEventComponent(const std::string &name) : Component(name) {}

[[nodiscard]] traact::pattern::Pattern::Ptr SyncUserEventComponent::GetPattern() {
    traact::pattern::Pattern::Ptr
        pattern =
        std::make_shared<traact::pattern::Pattern>("SyncUserEvent", Concurrency::SERIAL, ComponentType::SYNC_SOURCE);

    pattern->addProducerPort<OutEvent>("output");
    return pattern;
}

bool SyncUserEventComponent::processTimePoint(buffer::ComponentBuffer &data) {
    std::lock_guard guard(event_lock_);
    if (event_) {
        data.getOutput<OutEvent>() = event_.value();
        event_ = {};
    }

    return true;
}
void SyncUserEventComponent::fireEvent(int event) {
    std::lock_guard guard(event_lock_);
    event_ = {event};
}

CREATE_TRAACT_COMPONENT_FACTORY(SyncUserEventComponent);

}

BEGIN_TRAACT_PLUGIN_REGISTRATION
    REGISTER_DEFAULT_COMPONENT(traact::component::SyncUserEventComponent)
END_TRAACT_PLUGIN_REGISTRATION
