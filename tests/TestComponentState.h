/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_TESTS_TESTCOMPONENTSTATE_H_
#define TRAACT_CORE_TESTS_TESTCOMPONENTSTATE_H_

#include "TestDatatypes.h"

static constexpr size_t kExpectedEvents = 1000;

enum class TestEvents {
    INVALID = 0,
    CONFIGURE,
    START,
    DATA,
    INVALID_DATA,
    CALL_REQUEST,
    CALL_COMMIT_DONE,
    STOP,
    TEARDOWN,
};

struct TestComponentEvent {
    TestComponentEvent(TestEvents t_event_type, const traact::Timestamp &t_timestamp);
    traact::TimestampSteady event_timestamp;
    TestEvents event_type;
    traact::Timestamp timestamp;
};

struct TestComponentState {
    TestComponentState();

    void callConfigure();
    void callStart();
    void callStop();
    void callTeardown();
    void callProcessTimePoint(const traact::buffer::ComponentBuffer &data);
    void callInvalidTimePoint(const traact::buffer::ComponentBuffer &data);
    void callRequest(traact::Timestamp timestamp);
    void callCommitDone(traact::Timestamp timestamp);
    std::vector<TestComponentEvent> events;
    std::vector<std::string> process_data;
    std::mutex event_lock;
    bool expectInOrder(bool data_in_order) const;
    size_t expectEventCount(TestEvents test_event) const;
    bool precedes(const TestComponentState &next_state, const std::vector<traact::Timestamp> &missing_events) const;
    const TestComponentEvent * getEventFor(const TestComponentEvent &event) const;
};

template <> struct fmt::formatter<TestEvents> {

    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const TestEvents & value, FormatContext& ctx) -> decltype(ctx.out()) {
        switch(value){

            case TestEvents::INVALID:return format_to(ctx.out(), "{}", "INVALID");
            case TestEvents::CONFIGURE:return format_to(ctx.out(), "{}", "CONFIGURE");
            case TestEvents::START:return format_to(ctx.out(), "{}", "START");
            case TestEvents::DATA:return format_to(ctx.out(), "{}", "DATA");
            case TestEvents::INVALID_DATA:return format_to(ctx.out(), "{}", "INVALID_DATA");
            case TestEvents::STOP:return format_to(ctx.out(), "{}", "STOP");
            case TestEvents::TEARDOWN:return format_to(ctx.out(), "{}", "TEARDOWN");
            case TestEvents::CALL_REQUEST:return format_to(ctx.out(), "{}", "CALL_REQUEST");
            case TestEvents::CALL_COMMIT_DONE:return format_to(ctx.out(), "{}", "REQUEST_DONE");
            default: return format_to(ctx.out(), "{}", "unknown");

        }

    }
};

#endif //TRAACT_CORE_TESTS_TESTCOMPONENTSTATE_H_
