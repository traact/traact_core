/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_TESTS_TESTSYNCSINKCOMPONENT_H_
#define TRAACT_CORE_TESTS_TESTSYNCSINKCOMPONENT_H_

#include "TestComponents.h"

class TestSyncSinkComponent : public TestComponent{
 public:
    TestSyncSinkComponent(const std::string &name);
    static traact::pattern::Pattern::Ptr GetPattern()  {
        std::string pattern_name = "TestSyncSinkComponent";
        traact::pattern::Pattern::Ptr
            pattern = std::make_shared<traact::pattern::Pattern>(pattern_name, traact::Concurrency::SERIAL, traact::component::ComponentType::SYNC_SINK);
        pattern->addConsumerPort("input", TestStringHeader::MetaType);
        pattern->addCoordinateSystem("A").addCoordinateSystem("B").addEdge("A", "B", "output");
        return pattern;
    }


};


#endif //TRAACT_CORE_TESTS_TESTSYNCSINKCOMPONENT_H_
