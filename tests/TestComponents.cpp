/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TestComponents.h"


bool TestComponent::configure(const nlohmann::json &parameter, traact::buffer::ComponentBufferConfig *data) {
    SPDLOG_INFO("call configure: {0}", getName());
    component_state_.callConfigure();
    return Component::configure(parameter, data);
}
bool TestComponent::start() {
    SPDLOG_INFO("call start: {0}", getName());
    component_state_.callStart();
    return Component::start();
}
bool TestComponent::stop() {
    SPDLOG_INFO("call stop: {0}", getName());
    component_state_.callStop();
    return Component::stop();
}
bool TestComponent::teardown() {
    SPDLOG_INFO("call teardown: {0}", getName());
    component_state_.callTeardown();
    return Component::teardown();
}
bool TestComponent::processTimePoint(traact::buffer::ComponentBuffer &data) {
    SPDLOG_INFO("call process time point: {0}", getName());
    component_state_.callProcessTimePoint(data.getTimestamp());
    return Component::processTimePoint(data);
}
bool TestComponent::processTimePointWithInvalid(traact::buffer::ComponentBuffer &data) {
    SPDLOG_INFO("call invalid time point: {0}", getName());
    component_state_.callInvalidTimePoint(data.getTimestamp());
    return true;
}
TestComponent::TestComponent(const std::string &name)
    : Component(name) {}