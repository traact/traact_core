/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#include <gtest/gtest.h>
#include <traact/buffer/ComponentBuffer.h>
#include <TestDatatypes.h>


TEST(SingleComponentBuffer, Empty) {
    using namespace traact;
    using namespace traact::buffer;


    const TimeDuration kDuration(1000);
    const size_t kTimeStepIndex = 3;
    const size_t kComponentIndex = 2;

    Timestamp timestamp = Timestamp::min() + TimeDuration(kDuration);

    std::vector<std::string> buffer_data{};
    std::vector<Timestamp> buffer_timestamp{};
    std::vector<PortState> buffer_valid{};

    LocalDataBuffer input_data{};
    LocalValidBuffer input_valid{};
    LocalTimestampBuffer input_timestamp{};

    LocalDataBuffer output_data{};
    LocalValidBuffer output_valid{};
    LocalTimestampBuffer output_timestamp{};

    EventType event_type = EventType::CONFIGURE;
    ComponentBuffer buffer
        (kComponentIndex,
         input_data, input_valid, input_timestamp,
         output_data, output_valid, output_timestamp,
         kTimeStepIndex, &timestamp, &event_type);

    EXPECT_EQ(kComponentIndex, buffer.getComponentIndex());
    EXPECT_EQ(kTimeStepIndex, buffer.getTimeStepIndex());
    EXPECT_EQ(timestamp, buffer.getTimestamp());
    EXPECT_EQ(event_type, buffer.getEventType());
    EXPECT_EQ(0, buffer.getInputCount());
    EXPECT_EQ(0, buffer.getOutputCount());
}

TEST(SingleComponentBuffer, Input) {
    using namespace traact;
    using namespace traact::buffer;


    const TimeDuration kDuration(1000);
    const size_t kTimeStepIndex = 3;
    const size_t kComponentIndex = 2;
    using Port0 = buffer::PortConfig<TestStringHeader, 0>;
    const std::string kValue = "value_0";

    Timestamp timestamp = Timestamp::min() + TimeDuration(kDuration);

    std::vector<std::string> buffer_data{"invalid"};
    std::vector<Timestamp> buffer_timestamp{Timestamp::min()};
    std::vector<PortState> buffer_valid{PortState::INVALID};

    LocalDataBuffer input_data{&buffer_data[0]};
    LocalValidBuffer input_valid{&buffer_valid[0]};
    LocalTimestampBuffer input_timestamp{&buffer_timestamp[0]};

    LocalDataBuffer output_data{};
    LocalValidBuffer output_valid{};
    LocalTimestampBuffer output_timestamp{};

    EventType event_type = EventType::CONFIGURE;
    ComponentBuffer buffer
        (kComponentIndex,
         input_data, input_valid, input_timestamp,
         output_data, output_valid, output_timestamp,
         kTimeStepIndex, &timestamp, &event_type);

    EXPECT_EQ(kComponentIndex, buffer.getComponentIndex());
    EXPECT_EQ(kTimeStepIndex, buffer.getTimeStepIndex());
    EXPECT_EQ(timestamp, buffer.getTimestamp());
    EXPECT_EQ(event_type, buffer.getEventType());
    EXPECT_EQ(1, buffer.getInputCount());
    EXPECT_EQ(0, buffer.getOutputCount());

    // value and timestamp are not checked of port is invalid
    {
        SCOPED_TRACE("InputInvalid");
        checkInput(buffer, 0, traact::buffer::PortState::INVALID, "", Timestamp::max());
    }

    // usually all data should be handled by the component buffer, in this test change data from the outside, component buffer should reflect this change
    buffer_valid[0] = PortState::VALID;
    buffer_data[0] = kValue;
    buffer_timestamp[0] = timestamp;
    {
        SCOPED_TRACE("InputValid");
        checkInput(buffer, 0, traact::buffer::PortState::VALID, kValue, timestamp);
    }

    EXPECT_EQ(kValue, buffer.getInput<Port0>());
}

TEST(SingleComponentBuffer, Output) {
    using namespace traact;
    using namespace traact::buffer;


    const TimeDuration kDuration(1000);
    const size_t kTimeStepIndex = 3;
    const size_t kComponentIndex = 2;
    const std::string kValue = "value_0";
    using Port0 = buffer::PortConfig<TestStringHeader, 0>;

    Timestamp timestamp = Timestamp::min() + TimeDuration(kDuration);

    std::vector<std::string> buffer_data{"invalid"};
    std::vector<Timestamp> buffer_timestamp{Timestamp::min()};
    std::vector<PortState> buffer_valid{PortState::INVALID};

    LocalDataBuffer input_data{};
    LocalValidBuffer input_valid{};
    LocalTimestampBuffer input_timestamp{};

    LocalDataBuffer output_data{&buffer_data[0]};
    LocalValidBuffer output_valid{&buffer_valid[0]};
    LocalTimestampBuffer output_timestamp{&buffer_timestamp[0]};

    EventType event_type = EventType::CONFIGURE;
    ComponentBuffer buffer
        (kComponentIndex,
         input_data, input_valid, input_timestamp,
         output_data, output_valid, output_timestamp,
         kTimeStepIndex, &timestamp, &event_type);

    EXPECT_EQ(kComponentIndex, buffer.getComponentIndex());
    EXPECT_EQ(kTimeStepIndex, buffer.getTimeStepIndex());
    EXPECT_EQ(timestamp, buffer.getTimestamp());
    EXPECT_EQ(event_type, buffer.getEventType());
    EXPECT_EQ(0, buffer.getInputCount());
    EXPECT_EQ(1, buffer.getOutputCount());

    {
        SCOPED_TRACE("Output_0");
        setOutputInvalid(buffer, 0);
        auto& output_value = buffer.getOutput<TestStringHeader>(0);
        output_value = kValue;
        checkOutput(buffer, 0, traact::buffer::PortState::VALID, kValue, timestamp);
    }

    {
        SCOPED_TRACE("Output_1");
        setOutputInvalid(buffer, 0);
        Timestamp new_timestamp = timestamp + kDuration;
        auto& output_value = buffer.getOutput<TestStringHeader>(0, new_timestamp);
        output_value = kValue;
        checkOutput(buffer, 0, traact::buffer::PortState::VALID, kValue, new_timestamp);
    }
    {
        SCOPED_TRACE("Output_2");
        setOutputInvalid(buffer, 0);
        auto& output_value = buffer.getOutput<Port0>();
        output_value = kValue;
        checkOutput(buffer, 0, traact::buffer::PortState::VALID, kValue, timestamp);
    }

    {
        SCOPED_TRACE("Output_3");
        setOutputInvalid(buffer, 0);
        Timestamp new_timestamp = timestamp + kDuration;
        auto& output_value = buffer.getOutput<TestStringHeader>(0, new_timestamp);
        output_value = kValue;
        checkOutput(buffer, 0, traact::buffer::PortState::VALID, kValue, new_timestamp);
    }
    {
        SCOPED_TRACE("Output_4");
        setOutputInvalid(buffer, 0);
        Timestamp new_timestamp = timestamp + kDuration;
        auto& output_value = buffer.getOutput<TestStringHeader>(0, new_timestamp);
        output_value = "foobar";
        buffer.setOutputInvalid(0);
        checkOutput(buffer, 0, traact::buffer::PortState::INVALID, kValue, new_timestamp);
    }
    {
        SCOPED_TRACE("Output_5");
        setOutputInvalid(buffer, 0);
        Timestamp new_timestamp = timestamp + kDuration;
        auto& output_value = buffer.getOutput<TestStringHeader>(0, new_timestamp);
        output_value = "foobar";
        buffer.setOutputInvalid<Port0>();
        checkOutput(buffer, 0, traact::buffer::PortState::INVALID, kValue, new_timestamp);
    }

}
