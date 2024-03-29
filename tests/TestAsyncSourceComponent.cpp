/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TestAsyncSourceComponent.h"



TestAsyncSourceComponent::TestAsyncSourceComponent(const std::string &name)
    : TestComponent(name) {}

void TestAsyncSourceComponent::new_value(traact::Timestamp timestamp, std::string value) {
    SPDLOG_INFO("call from source {0}, ts: {1} new value", getName(), timestamp);
    component_state_.callRequest(timestamp);
    auto buffer = request_callback_(timestamp);
    buffer.wait();
    auto *buffer_p = buffer.get();
    if (buffer_p == nullptr){
        component_state_.requestRejected(timestamp);
        return;
    }
    auto &new_data = buffer_p->template getOutput<TestStringHeader>(0);
    new_data = value;
    component_state_.callCommitDone(timestamp);
    buffer_p->commit(true);
    SPDLOG_INFO("call from source {0}, ts: {1} commit done", getName(), timestamp);
}
void TestAsyncSourceComponent::invalid_value(traact::Timestamp timestamp) {
    component_state_.callRequest(timestamp);
    auto buffer = request_callback_(timestamp);
    buffer.wait();
    auto *buffer_p = buffer.get();
    if (buffer_p == nullptr){
        throw std::runtime_error("source buffer is null");
    }
    component_state_.callCommitDone(timestamp);
    buffer_p->commit(true);
}

CREATE_TRAACT_COMPONENT_FACTORY(TestAsyncSourceComponent)

BEGIN_TRAACT_PLUGIN_REGISTRATION
    REGISTER_DEFAULT_COMPONENT(TestAsyncSourceComponent)
END_TRAACT_PLUGIN_REGISTRATION
