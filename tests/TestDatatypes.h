/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_TESTS_TESTDATATYPES_H_
#define TRAACT_CORE_TESTS_TESTDATATYPES_H_
#include <gtest/gtest.h>
#include <string>
#include <traact/traact.h>

const static constexpr char* kTestValue{"{0}_"};
const static constexpr size_t kExpectedTestValueLength = 5;

CREATE_TRAACT_HEADER_TYPE(TestStringHeader, std::string, "TestString", )

struct ExpectedPortState {
    ExpectedPortState(size_t port,
                      traact::buffer::PortState state,
                      std::string value,
                      const traact::Timestamp &timestamp);
    size_t port;
    traact::buffer::PortState state;
    std::string value;
    traact::Timestamp timestamp;
};

void checkInput(const traact::buffer::ComponentBuffer &buffer, size_t port,
                traact::buffer::PortState state,
                const std::string &value,
                const traact::Timestamp &timestamp );
void checkOutput(const traact::buffer::ComponentBuffer &buffer, size_t port,
                 traact::buffer::PortState state,
                 const std::string &value,
                 const traact::Timestamp &timestamp);
void setOutputInvalid(const traact::buffer::ComponentBuffer &buffer, size_t port );


#endif //TRAACT_CORE_TESTS_TESTDATATYPES_H_
