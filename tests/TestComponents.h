/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACT_CORE_TESTS_TESTCOMPONENTS_H_
#define TRAACT_CORE_TESTS_TESTCOMPONENTS_H_

#include <traact/traact.h>
#include "TestDatatypes.h"
#include "TestComponentState.h"

class TestComponent : public traact::component::Component{
 public:
    TestComponent(const std::string &name, traact::component::ComponentType component_type);
    ~TestComponent() override = default;

    bool configure(const nlohmann::json &parameter, traact::buffer::ComponentBufferConfig *data) override;
    bool start() override;
    bool stop() override;
    bool teardown() override;
    bool processTimePoint(traact::buffer::ComponentBuffer &data) override;
    void invalidTimePoint(traact::Timestamp timestamp, size_t mea_idx) override;
    TestComponentState component_state_{};

    RTTR_ENABLE(traact::component::Component)
};



#endif //TRAACT_CORE_TESTS_TESTCOMPONENTS_H_
