/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TestSyncFunctionalComponent.h"
TestSyncFunctionalComponent::TestSyncFunctionalComponent(const std::string &name) : TestComponent(name) {

}

CREATE_TRAACT_COMPONENT_FACTORY(TestSyncFunctionalComponent)

BEGIN_TRAACT_PLUGIN_REGISTRATION
    REGISTER_DEFAULT_COMPONENT(TestSyncFunctionalComponent)
END_TRAACT_PLUGIN_REGISTRATION
