/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TestSyncSinkComponent.h"

TestSyncSinkComponent::TestSyncSinkComponent(const std::string &name)
    : TestComponent(name) {}

CREATE_TRAACT_COMPONENT_FACTORY(TestSyncSinkComponent)

BEGIN_TRAACT_PLUGIN_REGISTRATION
    REGISTER_DEFAULT_COMPONENT(TestSyncSinkComponent)
END_TRAACT_PLUGIN_REGISTRATION