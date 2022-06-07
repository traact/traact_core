/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACT_CORE_TESTS_TESTASYNCSOURCECOMPONENT_H_
#define TRAACT_CORE_TESTS_TESTASYNCSOURCECOMPONENT_H_

#include "TestComponents.h"

class TestAsyncSourceComponent : public TestComponent{
 public:
    TestAsyncSourceComponent(const std::string &name);
    traact::pattern::Pattern::Ptr GetPattern() const override;

    void new_value(traact::Timestamp timestamp, std::string value);

};


#endif //TRAACT_CORE_TESTS_TESTASYNCSOURCECOMPONENT_H_
