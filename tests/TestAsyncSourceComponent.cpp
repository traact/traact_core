/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#include "TestAsyncSourceComponent.h"


traact::pattern::Pattern::Ptr TestAsyncSourceComponent::GetPattern() const {
    std::string pattern_name = "TestAsyncSourceComponent";
    traact::pattern::Pattern::Ptr
        pattern = std::make_shared<traact::pattern::Pattern>(pattern_name, traact::Concurrency::SERIAL);
    pattern->addProducerPort("output", TestStringHeader::MetaType);
    pattern->addCoordinateSystem("A").addCoordinateSystem("B").addEdge("A", "B", "output");
    return pattern;
}

TestAsyncSourceComponent::TestAsyncSourceComponent(const std::string &name)
    : TestComponent(name, traact::component::ComponentType::ASYNC_SOURCE) {}

void TestAsyncSourceComponent::new_value(traact::Timestamp timestamp, std::string value) {
    spdlog::info("call from source, new value: {0}", getName());
    component_state_.callRequest(timestamp);
    auto buffer = request_callback_(timestamp);
    buffer.wait();
    auto *buffer_p = buffer.get();
    if (buffer_p == nullptr){
        throw std::runtime_error("source buffer is null");
    }
    auto &new_data = buffer_p->template getOutput<TestStringHeader>(0);
    new_data = value;
    component_state_.callCommitDone(timestamp);
    buffer_p->commit(true);

}

RTTR_PLUGIN_REGISTRATION // remark the different registration macro!
{
    using namespace rttr;
    registration::class_<TestAsyncSourceComponent>("TestAsyncSourceComponent").constructor<std::string>()();
}