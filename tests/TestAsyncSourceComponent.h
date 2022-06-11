/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_TESTS_TESTASYNCSOURCECOMPONENT_H_
#define TRAACT_CORE_TESTS_TESTASYNCSOURCECOMPONENT_H_

#include "TestComponents.h"

class TestAsyncSourceComponent : public TestComponent{
 public:
    TestAsyncSourceComponent(const std::string &name);
    static traact::pattern::Pattern::Ptr GetPattern()  {
        std::string pattern_name = "TestAsyncSourceComponent";
        traact::pattern::Pattern::Ptr
            pattern = std::make_shared<traact::pattern::Pattern>(pattern_name, traact::Concurrency::SERIAL, traact::component::ComponentType::ASYNC_SOURCE);
        pattern->addProducerPort("output", TestStringHeader::MetaType);

        return pattern;
    }


    void new_value(traact::Timestamp timestamp, std::string value);

    void invalid_value(traact::Timestamp timestamp);

};


#endif //TRAACT_CORE_TESTS_TESTASYNCSOURCECOMPONENT_H_
