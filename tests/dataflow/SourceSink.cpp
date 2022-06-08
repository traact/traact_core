/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/
#include <gtest/gtest.h>
#include <traact/buffer/ComponentBuffer.h>
#include <TestDatatypes.h>
#include <traact/traact.h>
#include "TestAsyncSourceComponent.h"
#include "TestSyncSinkComponent.h"
#include <traact/facade/DefaultFacade.h>

class SourceSink: public ::testing::TestWithParam<int> {
 public:
    SourceSink( ) {

    }

    void SetUp( ) {
        using namespace traact;
        using namespace traact::facade;
        using namespace traact::dataflow;



        my_facade_ = std::make_unique<traact::DefaultFacade>();

        DefaultInstanceGraphPtr pattern_graph_ptr = std::make_shared<DefaultInstanceGraph>("SourceSink");

        DefaultPatternInstancePtr
            source_pattern =
            pattern_graph_ptr->addPattern("source", my_facade_->instantiatePattern("TestAsyncSourceComponent"));
        DefaultPatternInstancePtr
            sink_pattern = pattern_graph_ptr->addPattern("sink", my_facade_->instantiatePattern("TestSyncSinkComponent"));


        // track marker
        pattern_graph_ptr->connect("source", "output", "sink", "input");


        buffer::TimeDomainManagerConfig td_config;
        td_config.time_domain = 0;
        td_config.ringbuffer_size = GetParam();
        td_config.master_source = "source";
        td_config.source_mode = SourceMode::WAIT_FOR_BUFFER;
        td_config.missing_source_event_mode = MissingSourceEventMode::WAIT_FOR_EVENT;
        td_config.max_offset = std::chrono::milliseconds(0);
        td_config.max_delay = std::chrono::milliseconds(100);
        td_config.measurement_delta = std::chrono::nanoseconds(33333333);

        pattern_graph_ptr->timedomain_configs[0] = td_config;

        my_facade_->loadDataflow(pattern_graph_ptr);

        source_component_ = my_facade_->getComponentAs<TestAsyncSourceComponent>("source");
        sink_component_ = my_facade_->getComponentAs<TestSyncSinkComponent>("sink");
    }

    void TearDown( ) {
        // code here will be called just after the test completes
        // ok to through exceptions from here if need be
    }

    std::unique_ptr<traact::DefaultFacade> my_facade_;
    std::shared_ptr<TestAsyncSourceComponent> source_component_;
    std::shared_ptr<TestSyncSinkComponent> sink_component_;
    const traact::TimeDuration sleep_time_{std::chrono::milliseconds(200)};
    const traact::Timestamp start_timestamp_{std::chrono::milliseconds(1000000000000LL)};
    const traact::TimeDuration time_delta_{std::chrono::nanoseconds (1)};
};

TEST_P(SourceSink, NoDataEvent) {


    my_facade_->start();
    my_facade_->stop();

    auto& source_state = source_component_->component_state_;
    auto& sink_state = sink_component_->component_state_;
    bool force_data_in_order = true;
    {
        SCOPED_TRACE("source_state");
        EXPECT_TRUE(source_state.expectInOrder(force_data_in_order));
        EXPECT_EQ(1, source_state.expectEventCount(TestEvents::CONFIGURE));
        EXPECT_EQ(1, source_state.expectEventCount(TestEvents::START));
        EXPECT_EQ(1, source_state.expectEventCount(TestEvents::STOP));
        EXPECT_EQ(1, source_state.expectEventCount(TestEvents::TEARDOWN));
        EXPECT_EQ(0, source_state.expectEventCount(TestEvents::DATA));

    }
    {
        SCOPED_TRACE("sink_state");
        EXPECT_TRUE(sink_state.expectInOrder(force_data_in_order));
        EXPECT_EQ(1, sink_state.expectEventCount(TestEvents::CONFIGURE));
        EXPECT_EQ(1, sink_state.expectEventCount(TestEvents::START));
        EXPECT_EQ(1, sink_state.expectEventCount(TestEvents::STOP));
        EXPECT_EQ(1, sink_state.expectEventCount(TestEvents::TEARDOWN));
        EXPECT_EQ(0, sink_state.expectEventCount(TestEvents::DATA));
    }
    {
        SCOPED_TRACE("source_before_sink");
        std::vector<traact::Timestamp> all_events_present{};
        EXPECT_TRUE(source_state.precedes(sink_state, all_events_present));
    }
}

TEST_P(SourceSink, DataEvents) {

    const size_t kDataEvents = 1000;



    my_facade_->start();
    for(size_t i=0;i<kDataEvents;++i){
        source_component_->new_value(start_timestamp_+time_delta_*i, fmt::format("{0}",i));
    }
    my_facade_->stop();

    auto& source_state = source_component_->component_state_;
    auto& sink_state = sink_component_->component_state_;
    bool force_data_in_order = true;
    {
        SCOPED_TRACE("source_state");
        EXPECT_TRUE(source_state.expectInOrder(force_data_in_order));
        EXPECT_EQ(1, source_state.expectEventCount(TestEvents::CONFIGURE));
        EXPECT_EQ(1, source_state.expectEventCount(TestEvents::START));
        EXPECT_EQ(1, source_state.expectEventCount(TestEvents::STOP));
        EXPECT_EQ(1, source_state.expectEventCount(TestEvents::TEARDOWN));
        EXPECT_EQ(0, source_state.expectEventCount(TestEvents::DATA));
        EXPECT_EQ(kDataEvents, source_state.expectEventCount(TestEvents::CALL_REQUEST));
        EXPECT_EQ(kDataEvents, source_state.expectEventCount(TestEvents::CALL_COMMIT_DONE));

    }
    {
        SCOPED_TRACE("sink_state");
        EXPECT_TRUE(sink_state.expectInOrder(force_data_in_order));
        EXPECT_EQ(1, sink_state.expectEventCount(TestEvents::CONFIGURE));
        EXPECT_EQ(1, sink_state.expectEventCount(TestEvents::START));
        EXPECT_EQ(1, sink_state.expectEventCount(TestEvents::STOP));
        EXPECT_EQ(1, sink_state.expectEventCount(TestEvents::TEARDOWN));
        EXPECT_EQ(kDataEvents, sink_state.expectEventCount(TestEvents::DATA));
        EXPECT_EQ(0, sink_state.expectEventCount(TestEvents::CALL_REQUEST));
        EXPECT_EQ(0, sink_state.expectEventCount(TestEvents::CALL_COMMIT_DONE));
    }
    {
        SCOPED_TRACE("source_before_sink");
        std::vector<traact::Timestamp> all_events_present{};
        EXPECT_TRUE(source_state.precedes(sink_state, all_events_present));
    }
}

INSTANTIATE_TEST_SUITE_P(ConcurrentTimeSteps, SourceSink,::testing::Range(1,11));