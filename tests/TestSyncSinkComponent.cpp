/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#include "TestSyncSinkComponent.h"
traact::pattern::Pattern::Ptr TestSyncSinkComponent::GetPattern() const {
    std::string pattern_name = "TestSyncSinkComponent";
    traact::pattern::Pattern::Ptr
        pattern = std::make_shared<traact::pattern::Pattern>(pattern_name, traact::Concurrency::SERIAL);
    pattern->addConsumerPort("input", TestStringHeader::MetaType);
    pattern->addCoordinateSystem("A").addCoordinateSystem("B").addEdge("A", "B", "output");
    return pattern;
}

TestSyncSinkComponent::TestSyncSinkComponent(const std::string &name)
    : TestComponent(name, traact::component::ComponentType::SYNC_SINK) {}

RTTR_PLUGIN_REGISTRATION // remark the different registration macro!
{
    using namespace rttr;
    registration::class_<TestSyncSinkComponent>("TestSyncSinkComponent").constructor<std::string>()();
}