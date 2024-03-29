/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TestComponentState.h"
#include <gtest/gtest.h>
#include <traact/util/Logging.h>

void TestComponentState::callConfigure() {
    std::unique_lock guard(event_lock);
    events.emplace_back(TestEvents::CONFIGURE, traact::kTimestampZero);
}
void TestComponentState::callStart() {
    std::unique_lock guard(event_lock);
    events.emplace_back(TestEvents::START, traact::kTimestampZero);
}
void TestComponentState::callStop() {
    std::unique_lock guard(event_lock);
    events.emplace_back(TestEvents::STOP, traact::kTimestampZero);
}
void TestComponentState::callTeardown() {
    std::unique_lock guard(event_lock);
    events.emplace_back(TestEvents::TEARDOWN, traact::kTimestampZero);
}
void TestComponentState::callProcessTimePoint(const traact::buffer::ComponentBuffer &data) {
    std::unique_lock guard(event_lock);
    events.emplace_back(TestEvents::DATA, data.getTimestamp());
    std::string data_string;
    data_string.reserve(data.getInputCount()*kExpectedTestValueLength);
    for (int i = 0; i < data.getInputCount(); ++i) {
        data_string += data.getInput<TestStringHeader>(i);
    }
    for (int i = 0; i < data.getOutputCount(); ++i) {
        data.getOutput<TestStringHeader>(i) = data_string;
    }
}
void TestComponentState::callInvalidTimePoint(const traact::buffer::ComponentBuffer &data) {
    std::unique_lock guard(event_lock);
    events.emplace_back(TestEvents::INVALID_DATA, data.getTimestamp());
}
void TestComponentState::callRequest(traact::Timestamp timestamp) {
    std::unique_lock guard(event_lock);
    events.emplace_back(TestEvents::CALL_REQUEST, timestamp);

}
void TestComponentState::callCommitDone(traact::Timestamp timestamp) {
    std::unique_lock guard(event_lock);
    events.emplace_back(TestEvents::CALL_COMMIT_DONE, timestamp);
}

void TestComponentState::requestRejected(traact::Timestamp timestamp) {
    std::unique_lock guard(event_lock);
    events.emplace_back(TestEvents::REQUEST_REJECTED, timestamp);
}

TestComponentState::TestComponentState() {
    events.reserve(kExpectedEvents);
    process_data.reserve(kExpectedEvents);

}
bool TestComponentState::expectInOrder(bool data_in_order) const {
    // there must be at least the dataflow setup events
    EXPECT_TRUE(events.size() >= 4);
    // the dataflow setup events must be the first and last events for a component
    EXPECT_EQ(TestEvents::CONFIGURE, events[0].event_type);
    EXPECT_EQ(TestEvents::START, events[1].event_type);
    EXPECT_EQ(TestEvents::STOP, events[events.size() - 2].event_type);
    EXPECT_EQ(TestEvents::TEARDOWN, events[events.size() - 1].event_type);

    // only the position of the data messages needs to be checked now,
    if (data_in_order) {
        // the timestamp for non data events is always 0, skip for data event checks
        for (size_t i = 1; i < events.size() - 3; ++i) {
            EXPECT_LE(events[i].real_timestamp.time_since_epoch().count(),
                      events[i + 1].real_timestamp.time_since_epoch().count());
        }
        for (size_t i = 1; i < events.size() - 3; ++i) {
            EXPECT_LE(events[i].event_timestamp.time_since_epoch().count(),
                      events[i + 1].event_timestamp.time_since_epoch().count());
        }
    }

    return true;
}
size_t TestComponentState::expectEventCount(TestEvents test_event) const {
    return std::count_if(events.begin(), events.end(), [test_event](const auto &value) { return value.event_type == test_event; });
}
bool TestComponentState::precedes(const TestComponentState &next_state,
                                  const std::vector<traact::Timestamp> &missing_events) const {
    // assumes expectInOrder was successful
    for (const auto& event : events) {
        auto is_missing = std::count(missing_events.begin(), missing_events.end(), event.event_timestamp);
        if(is_missing)
            continue;

        // skip call requests, compare CALL_COMMIT_DONE to CALL_DATA from other components
        if(event.event_type == TestEvents::CALL_REQUEST)
            continue;
        // skip rejected requests for now, rejected either because it was an invalid timestamp
        // e.g. before first event timestamp, or SourceMode IMMEDIATE_RETURN or other reasons
        // no good way to check
        if(event.event_type == TestEvents::REQUEST_REJECTED)
            continue;


        auto next_event = next_state.getEventFor(event);


        if(next_event == nullptr){
            SPDLOG_ERROR("expected event: {0} ts: {1}", event.event_type, event.event_timestamp);
            continue;
        }

        EXPECT_LT(event.real_timestamp.time_since_epoch().count(), next_event->real_timestamp.time_since_epoch().count());
        if(event.real_timestamp >= next_event->real_timestamp){
            auto diff = event.real_timestamp - next_event->real_timestamp;
            SPDLOG_ERROR("current >= next: {0}, dataflow ts: {1}, event: {2}", diff.count(), event.real_timestamp, event.event_type);
        }
    }
    return true;
}
const TestComponentEvent * TestComponentState::getEventFor(const TestComponentEvent &event) const {
    auto has_event = std::find_if(events.begin(), events.end(), [&event](const auto& value){
        if(event.event_type == TestEvents::CALL_COMMIT_DONE){
            return event.event_timestamp == value.event_timestamp && (TestEvents::DATA == value.event_type || TestEvents::INVALID_DATA == value.event_type);
        }
        else {
            return event.event_timestamp == value.event_timestamp && event.event_type == value.event_type;
        }
    });

    if(has_event != events.end()) {
        return has_event.base();
}

    return nullptr;
}


TestComponentEvent::TestComponentEvent(TestEvents t_event_type, const traact::Timestamp &t_timestamp) : real_timestamp(
    traact::nowSteady()), event_type(
    t_event_type), event_timestamp(t_timestamp) {}