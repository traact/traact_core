/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include <gtest/gtest.h>
#include "traact/dataflow/taskflow/TimeDomainClock.h"
#include <random>
#include <fmt/format.h>
// measurements of one day with a 30Hz sensor
//static const constexpr size_t kTestTimeSteps = 3000000LL;
static const constexpr size_t kTestTimeSteps = 3000LL;

TEST(Test_TimeDomainClock, Source_1_1000000000Hz_NoNoise) {
    using namespace traact;
    using namespace traact::dataflow;

    const double kFrequency = 1000000000; // 1ns
    const TimeDuration kMaxOffset(0);
    const double kAlpha = 0.8;
    const Timestamp kStartTimestamp{std::chrono::milliseconds(1000000000000LL)};
    const TimeDuration kDelta(1);

    TimeDomainClock time_domain_clock(kFrequency, kMaxOffset, kAlpha);


    // initial time step is next time step
    time_domain_clock.newTimestamp(kStartTimestamp);
    EXPECT_TRUE(time_domain_clock.isNextExpectedTimestamp(kStartTimestamp));
    EXPECT_FALSE(time_domain_clock.isFurtherAheadThenNextExpectedTimestamp(kStartTimestamp));
    EXPECT_TRUE(time_domain_clock.isFurtherAheadThenNextExpectedTimestamp(kStartTimestamp + kDelta));
    // use timestamp
    Timestamp next_expected = time_domain_clock.getNextExpectedTimestamp();
    EXPECT_EQ(kStartTimestamp.time_since_epoch().count(), next_expected.time_since_epoch().count());

    for (size_t i = 0; i < kTestTimeSteps; ++i) {
        // next time step
        Timestamp next_timestamp = kStartTimestamp + kDelta * (i+1);
        time_domain_clock.newTimestamp(next_timestamp);
        EXPECT_TRUE(time_domain_clock.isNextExpectedTimestamp(next_timestamp));
        EXPECT_FALSE(time_domain_clock.isFurtherAheadThenNextExpectedTimestamp(next_timestamp));
        EXPECT_TRUE(time_domain_clock.isFurtherAheadThenNextExpectedTimestamp(next_timestamp + kDelta));
        // advance to next time step
        next_expected = time_domain_clock.getNextExpectedTimestamp();
        EXPECT_EQ(next_timestamp.time_since_epoch().count(), next_expected.time_since_epoch().count());
    }
}


TEST(Test_TimeDomainClock, Source_1_1000Hz_Noise) {
    using namespace traact;
    using namespace traact::dataflow;

    const double kFrequency = 1000; // 1ms
    const TimeDuration kMaxOffset(std::chrono::microseconds(240));
    const double kAlpha = 1.0;
    const Timestamp kStartTimestamp{std::chrono::milliseconds(1000000000000LL)};
    const TimeDuration kDelta(1000000);


    TimeDomainClock time_domain_clock(kFrequency, kMaxOffset, kAlpha);

    std::mt19937_64 random_generator;
    uint64_t timeSeed = 123456789123456LL;
    std::seed_seq ss{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed>>32)};
    random_generator.seed(ss);
    std::uniform_real_distribution<double> uniform_random(-1, 1);


    for (size_t i = 0; i < kTestTimeSteps; ++i) {
        // next time step
        SCOPED_TRACE(fmt::format("source_state {0}", i));
        TimeDuration random_offset(static_cast<uint64_t >(kMaxOffset.count()*uniform_random(random_generator)));
        Timestamp next_timestamp = kStartTimestamp + kDelta * (i) + random_offset ;
        time_domain_clock.newTimestamp(next_timestamp);
        EXPECT_TRUE(time_domain_clock.isNextExpectedTimestamp(next_timestamp));
        EXPECT_FALSE(time_domain_clock.isFurtherAheadThenNextExpectedTimestamp(next_timestamp));
        EXPECT_TRUE(time_domain_clock.isFurtherAheadThenNextExpectedTimestamp(next_timestamp + kDelta));
        // advance to next time step
        auto next_expected = time_domain_clock.getNextExpectedTimestamp();
        EXPECT_TRUE(isWithinRange(next_timestamp, next_expected, kMaxOffset));
    }
}

class Test_TimeDomainClockMultiSource : public ::testing::TestWithParam<int> {


};

TEST_P(Test_TimeDomainClockMultiSource, Source_Multi_1000Hz_Noise) {
    using namespace traact;
    using namespace traact::dataflow;

    const double kFrequency = 1000; // 1ms
    const TimeDuration kMaxOffset(std::chrono::microseconds(240));
    const double kAlpha = 1.0;
    const Timestamp kStartTimestamp{std::chrono::milliseconds(1000000000000LL)};
    const TimeDuration kDelta(1000000);

    int source_count = GetParam();

    TimeDomainClock time_domain_clock(kFrequency, kMaxOffset, kAlpha);

    std::mt19937_64 random_generator;
    uint64_t timeSeed = 123456789123456LL;
    std::seed_seq ss{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed>>32)};
    random_generator.seed(ss);
    std::uniform_real_distribution<double> uniform_random(-1, 1);

    std::vector<Timestamp> source_timestamps;
    source_timestamps.resize(source_count);
    for (size_t time_step = 0; time_step < kTestTimeSteps; ++time_step) {
        // next time step

        for (int source_index = 0; source_index < source_count; ++source_index) {
            SCOPED_TRACE(fmt::format("source_state {0} {1}", time_step, source_index));
            TimeDuration random_offset(static_cast<uint64_t >(kMaxOffset.count()*uniform_random(random_generator)));
            Timestamp next_timestamp = kStartTimestamp + kDelta * (time_step) + random_offset ;
            source_timestamps[source_index] = next_timestamp;
            time_domain_clock.newTimestamp(next_timestamp);
            EXPECT_TRUE(time_domain_clock.isNextExpectedTimestamp(next_timestamp));
            EXPECT_FALSE(time_domain_clock.isFurtherAheadThenNextExpectedTimestamp(next_timestamp));
            EXPECT_TRUE(time_domain_clock.isFurtherAheadThenNextExpectedTimestamp(next_timestamp + kDelta));
        }

        // advance to next time step
        auto next_expected = time_domain_clock.getNextExpectedTimestamp();
        for (int source_index = 0; source_index < source_count; ++source_index){
            EXPECT_TRUE(isWithinRange(source_timestamps[source_index], next_expected, kMaxOffset));
        }
    }
}

INSTANTIATE_TEST_CASE_P( Test_TimeDomainClock , Test_TimeDomainClockMultiSource, ::testing::Range(1,11));