/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_TESTS_TESTSYNCFUNCTIONALCOMPONENT_H_
#define TRAACT_CORE_TESTS_TESTSYNCFUNCTIONALCOMPONENT_H_

#include "TestComponents.h"

class TestSyncFunctionalComponent : public TestComponent{
 public:
    using InPort_0 = traact::buffer::PortConfig<TestStringHeader, 0>;
    using InPort_1 = traact::buffer::PortConfig<TestStringHeader, 1>;
    using OutPort_0 = traact::buffer::PortConfig<TestStringHeader, 0>;
    TestSyncFunctionalComponent(const std::string &name);
    static traact::pattern::Pattern::Ptr GetPattern()  {
        std::string pattern_name = "TestSyncFunctionalComponent";
        traact::pattern::Pattern::Ptr
            pattern = std::make_shared<traact::pattern::Pattern>(pattern_name, traact::Concurrency::SERIAL, traact::component::ComponentType::SYNC_FUNCTIONAL);
        pattern->addConsumerPort<InPort_0> ("input_0")
            .addConsumerPort<InPort_1>("input_1")
                .addProducerPort<OutPort_0>("output");

        return pattern;
    }


};


#endif //TRAACT_CORE_TESTS_TESTSYNCFUNCTIONALCOMPONENT_H_
