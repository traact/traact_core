/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#include "TestDatatypes.h"

#include <utility>
ExpectedPortState::ExpectedPortState(size_t port,
                                     traact::buffer::PortState state,
                                     std::string value,
                                     const traact::Timestamp &timestamp)
    : port(port), state(state), value(std::move(value)), timestamp(timestamp) {}

void checkInput(const traact::buffer::ComponentBuffer &buffer, size_t port,
                traact::buffer::PortState state,
                const std::string &value,
                const traact::Timestamp &timestamp) {
    auto input_valid = state == traact::buffer::PortState::VALID;
    EXPECT_EQ(input_valid, buffer.isInputValid(port));
    if(input_valid){
        const auto &buffer_value = buffer.getInput<TestStringHeader>(port);
        EXPECT_EQ(value, buffer_value);
        EXPECT_EQ(timestamp.time_since_epoch().count(), buffer.getInputTimestamp(port).time_since_epoch().count());
    }

}

void checkOutput(const traact::buffer::ComponentBuffer &buffer, size_t port,
                 traact::buffer::PortState state,
                 const std::string &value,
                 const traact::Timestamp &timestamp) {
    auto output_valid = state == traact::buffer::PortState::VALID;
    EXPECT_EQ(output_valid, buffer.isOutputValid(port));
    if(output_valid){
        EXPECT_EQ(timestamp.time_since_epoch().count(), buffer.getOutputTimestamp(port).time_since_epoch().count());

        const auto &buffer_value = buffer.getOutput<TestStringHeader>(port, timestamp);
        EXPECT_EQ(value, buffer_value);

    }
}
void setOutputInvalid(const traact::buffer::ComponentBuffer &buffer, size_t port) {
    auto& value = buffer.getOutput<TestStringHeader>(port, traact::Timestamp::max());
    value = "invalid";

    EXPECT_EQ(traact::Timestamp::max().time_since_epoch().count(), buffer.getOutputTimestamp(port).time_since_epoch().count());
    {
        const auto &buffer_value = buffer.getOutput<TestStringHeader>(port);
        EXPECT_EQ("invalid", buffer_value);

    }
    buffer.setOutputInvalid(port);
    EXPECT_FALSE(buffer.isOutputValid(port));

}

RTTR_PLUGIN_REGISTRATION // remark the different registration macro!
{

    using namespace rttr;
    registration::class_<TestStringFactoryObject>("TestStringFactoryObject").constructor<>()();
}