/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_TESTS_TESTSYNCSINKCOMPONENT_H_
#define TRAACT_CORE_TESTS_TESTSYNCSINKCOMPONENT_H_

#include "TestComponents.h"

class TestSyncSinkComponent : public TestComponent{
 public:
    TestSyncSinkComponent(const std::string &name);
    traact::pattern::Pattern::Ptr GetPattern() const override;

};


#endif //TRAACT_CORE_TESTS_TESTSYNCSINKCOMPONENT_H_
