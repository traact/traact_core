/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TestComponents.h"


bool TestComponent::configure(const traact::pattern::instance::PatternInstance &pattern_instance, traact::buffer::ComponentBufferConfig *data) {
    SPDLOG_INFO("call configure: {0}", getName());
    component_state_.callConfigure();
    return true;
}
bool TestComponent::start() {
    SPDLOG_INFO("call start: {0}", getName());
    component_state_.callStart();
    return true;
}
bool TestComponent::stop() {
    SPDLOG_INFO("call stop: {0}", getName());
    component_state_.callStop();
    return true;
}
bool TestComponent::teardown() {
    SPDLOG_INFO("call teardown: {0}", getName());
    component_state_.callTeardown();
    return true;
}
bool TestComponent::processTimePoint(traact::buffer::ComponentBuffer &data) {
    SPDLOG_INFO("call process time point: {0} ts: {1}", getName(), data.getTimestamp());
    component_state_.callProcessTimePoint(data);
    return true;
}
bool TestComponent::processTimePointWithInvalid(traact::buffer::ComponentBuffer &data) {
    SPDLOG_INFO("call invalid time point: {0} ts: {1}", getName(), data.getTimestamp());
    component_state_.callInvalidTimePoint(data);
    return true;
}
TestComponent::TestComponent(const std::string &name)
    : Component(name) {}