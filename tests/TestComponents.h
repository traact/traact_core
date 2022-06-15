/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_TESTS_TESTCOMPONENTS_H_
#define TRAACT_CORE_TESTS_TESTCOMPONENTS_H_

#include <traact/traact.h>
#include "TestDatatypes.h"
#include "TestComponentState.h"

class TestComponent : public traact::component::Component{
 public:
    TestComponent(const std::string &name);
    ~TestComponent() override = default;

    bool configure(const traact::pattern::instance::PatternInstance &pattern_instance, traact::buffer::ComponentBufferConfig *data) override;
    bool start() override;
    bool stop() override;
    bool teardown() override;
    bool processTimePoint(traact::buffer::ComponentBuffer &data) override;
    bool processTimePointWithInvalid(traact::buffer::ComponentBuffer &data) override;
    TestComponentState component_state_{};
};



#endif //TRAACT_CORE_TESTS_TESTCOMPONENTS_H_
