/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/
#include <gtest/gtest.h>
#include <traact/buffer/ComponentBuffer.h>
#include <TestDatatypes.h>
#include <traact/traact.h>
#include "TestAsyncSourceComponent.h"
#include "TestSyncSinkComponent.h"
#include "TestSyncFunctionalComponent.h"
#include <traact/facade/DefaultFacade.h>
#include <traact/util/Semaphore.h>
class Test_01_TwoAsyncSource_SyncFunctional_SyncSink: public ::testing::TestWithParam<int> {
 public:
    Test_01_TwoAsyncSource_SyncFunctional_SyncSink( ) {

    }

    void SetUp( ) {
        using namespace traact;
        using namespace traact::facade;
        using namespace traact::dataflow;



        my_facade_ = std::make_unique<traact::DefaultFacade>();

        DefaultInstanceGraphPtr pattern_graph_ptr = std::make_shared<DefaultInstanceGraph>("SourceSink");

        DefaultPatternInstancePtr
            source_0_pattern =
            pattern_graph_ptr->addPattern("source_0", my_facade_->instantiatePattern("TestAsyncSourceComponent"));
        DefaultPatternInstancePtr
            source_1_pattern =
            pattern_graph_ptr->addPattern("source_1", my_facade_->instantiatePattern("TestAsyncSourceComponent"));
        DefaultPatternInstancePtr
            functional_pattern =
            pattern_graph_ptr->addPattern("functional", my_facade_->instantiatePattern("TestSyncFunctionalComponent"));

        DefaultPatternInstancePtr
            sink_pattern = pattern_graph_ptr->addPattern("sink", my_facade_->instantiatePattern("TestSyncSinkComponent"));


        // track marker
        pattern_graph_ptr->connect("source_0", "output", "functional", "input_0");
        pattern_graph_ptr->connect("source_1", "output", "functional", "input_1");
        pattern_graph_ptr->connect("functional", "output", "sink", "input");


        buffer::TimeDomainManagerConfig td_config;
        td_config.time_domain = 0;
        td_config.ringbuffer_size = GetParam();
        td_config.master_source = "source";
        td_config.source_mode = SourceMode::WAIT_FOR_BUFFER;
        td_config.missing_source_event_mode = MissingSourceEventMode::WAIT_FOR_EVENT;
        td_config.max_offset = std::chrono::milliseconds(0);
        td_config.max_delay = std::chrono::milliseconds(100);
        td_config.sensor_frequency = sensor_frequency_;

        pattern_graph_ptr->timedomain_configs[0] = td_config;

        my_facade_->loadDataflow(pattern_graph_ptr);

        source_0_component_ = my_facade_->getComponentAs<TestAsyncSourceComponent>("source_0");
        source_1_component_ = my_facade_->getComponentAs<TestAsyncSourceComponent>("source_1");
        functional_component_ = my_facade_->getComponentAs<TestSyncFunctionalComponent>("functional");
        sink_component_ = my_facade_->getComponentAs<TestSyncSinkComponent>("sink");
    }

    void TearDown( ) {
        // code here will be called just after the test completes
        // ok to through exceptions from here if need be
    }

    std::unique_ptr<traact::DefaultFacade> my_facade_;
    std::shared_ptr<TestAsyncSourceComponent> source_0_component_;
    std::shared_ptr<TestAsyncSourceComponent> source_1_component_;
    std::shared_ptr<TestSyncFunctionalComponent> functional_component_;
    std::shared_ptr<TestSyncSinkComponent> sink_component_;
    const traact::TimeDuration sleep_time_{std::chrono::milliseconds(200)};
    const traact::Timestamp start_timestamp_{std::chrono::milliseconds(1000000000000LL)};
    const double sensor_frequency_{1000000000};
    const traact::TimeDuration time_delta_{1};
    const size_t kDataEvents = 1000;
    traact::WaitForInit wait_for_init;
};

void testDefaultOrder(const std::string &test_trace,
                      const TestComponentState &component_state,
                      size_t expected_process,
                      size_t expected_invalid,
                      size_t expected_request,
                      size_t expected_commit) {
    bool force_data_in_order = true;
    SCOPED_TRACE(test_trace);
    EXPECT_TRUE(component_state.expectInOrder(force_data_in_order));
    EXPECT_EQ(1, component_state.expectEventCount(TestEvents::CONFIGURE));
    EXPECT_EQ(1, component_state.expectEventCount(TestEvents::START));
    EXPECT_EQ(1, component_state.expectEventCount(TestEvents::STOP));
    EXPECT_EQ(1, component_state.expectEventCount(TestEvents::TEARDOWN));
    EXPECT_EQ(expected_process, component_state.expectEventCount(TestEvents::DATA));
    EXPECT_EQ(expected_invalid, component_state.expectEventCount(TestEvents::INVALID_DATA));
    EXPECT_EQ(expected_request, component_state.expectEventCount(TestEvents::CALL_REQUEST));
    EXPECT_EQ(expected_commit, component_state.expectEventCount(TestEvents::CALL_COMMIT_DONE));

}

TEST_P(Test_01_TwoAsyncSource_SyncFunctional_SyncSink, NoDataEvent) {


    my_facade_->start();
    my_facade_->stop();

    testDefaultOrder("source_0", source_0_component_->component_state_, 0, 0, 0, 0);
    testDefaultOrder("source_1", source_1_component_->component_state_, 0, 0, 0, 0);
    testDefaultOrder("functional", functional_component_->component_state_, 0, 0, 0, 0);
    testDefaultOrder("sink", sink_component_->component_state_, 0, 0, 0, 0);

    {
        auto& source_0_state = source_0_component_->component_state_;
        auto& source_1_state = source_1_component_->component_state_;
        auto& functional_state = functional_component_->component_state_;
        auto& sink_state = sink_component_->component_state_;
        SCOPED_TRACE("source_before_sink");
        std::vector<traact::Timestamp> all_events_present{};
        EXPECT_TRUE(source_0_state.precedes(functional_state, all_events_present));
        EXPECT_TRUE(source_1_state.precedes(functional_state, all_events_present));
        EXPECT_TRUE(source_0_state.precedes(sink_state, all_events_present));
        EXPECT_TRUE(source_1_state.precedes(sink_state, all_events_present));
        EXPECT_TRUE(functional_state.precedes(sink_state, all_events_present));
    }
}

TEST_P(Test_01_TwoAsyncSource_SyncFunctional_SyncSink, DataEvents) {

    



    my_facade_->start();

    auto source_0 = std::async([&, local_source = source_0_component_]() {
        for(size_t i=0;i<kDataEvents;++i){
            auto timestamp = start_timestamp_+time_delta_*i;
            SPDLOG_INFO("send source 0: {0}", timestamp);
            local_source->new_value(timestamp, fmt::format(kTestValue,i));
        }
    });

    auto source_1 = std::async([&, local_source = source_1_component_]() {
        for(size_t i=0;i<kDataEvents;++i){
            auto timestamp = start_timestamp_+time_delta_*i;
            SPDLOG_INFO("send source 1: {0}", timestamp);
            local_source->new_value(timestamp, fmt::format(kTestValue,i));
        }
    });

    source_0.wait();
    source_1.wait();

    my_facade_->stop();

    testDefaultOrder("source_0", source_0_component_->component_state_, 0, 0, kDataEvents, kDataEvents);
    testDefaultOrder("source_1", source_1_component_->component_state_, 0, 0, kDataEvents, kDataEvents);
    testDefaultOrder("functional", functional_component_->component_state_, kDataEvents, 0, 0, 0);
    testDefaultOrder("sink", sink_component_->component_state_, kDataEvents, 0, 0, 0);

    {
        auto& source_0_state = source_0_component_->component_state_;
        auto& source_1_state = source_1_component_->component_state_;
        auto& functional_state = functional_component_->component_state_;
        auto& sink_state = sink_component_->component_state_;
        SCOPED_TRACE("source_before_sink");
        std::vector<traact::Timestamp> all_events_present{};
        EXPECT_TRUE(source_0_state.precedes(functional_state, all_events_present));
        EXPECT_TRUE(source_1_state.precedes(functional_state, all_events_present));
        EXPECT_TRUE(source_0_state.precedes(sink_state, all_events_present));
        EXPECT_TRUE(source_1_state.precedes(sink_state, all_events_present));
        EXPECT_TRUE(functional_state.precedes(sink_state, all_events_present));
    }
}

TEST_P(Test_01_TwoAsyncSource_SyncFunctional_SyncSink, DataEventsEverySecondEventInvalid) {
    



    my_facade_->start();

    auto source_0 = std::async([&, local_source = source_0_component_]() {
        for(size_t i=0;i<kDataEvents;++i){
            auto timestamp = start_timestamp_+time_delta_*i;
            SPDLOG_INFO("send source 0: {0}", timestamp);
            local_source->new_value(timestamp, fmt::format(kTestValue,i));
        }
    });

    auto source_1 = std::async([&, local_source = source_1_component_]() {
        for(size_t i=0;i<kDataEvents;++i){
            auto timestamp = start_timestamp_+time_delta_*i;
            SPDLOG_INFO("send source 1: {0}", timestamp);
            if(i % 2 == 0){
                local_source->invalid_value(timestamp);
            } else {
                local_source->new_value(timestamp, fmt::format(kTestValue,i));
            }
        }
    });

    source_0.wait();
    source_1.wait();

    my_facade_->stop();

    testDefaultOrder("source_0", source_0_component_->component_state_, 0, 0, kDataEvents, kDataEvents);
    testDefaultOrder("source_1", source_1_component_->component_state_, 0, 0, kDataEvents, kDataEvents);
    testDefaultOrder("functional", functional_component_->component_state_, kDataEvents/2, kDataEvents/2, 0, 0);
    testDefaultOrder("sink", sink_component_->component_state_, kDataEvents/2, kDataEvents/2, 0, 0);

    {
        auto& source_0_state = source_0_component_->component_state_;
        auto& source_1_state = source_1_component_->component_state_;
        auto& functional_state = functional_component_->component_state_;
        auto& sink_state = sink_component_->component_state_;
        SCOPED_TRACE("source_before_functional_before_sink");
        std::vector<traact::Timestamp> all_events_present{};
        EXPECT_TRUE(source_0_state.precedes(functional_state, all_events_present));
        EXPECT_TRUE(source_1_state.precedes(functional_state, all_events_present ));
        EXPECT_TRUE(source_0_state.precedes(sink_state, all_events_present));
        EXPECT_TRUE(source_1_state.precedes(sink_state, all_events_present));
        EXPECT_TRUE(functional_state.precedes(sink_state, all_events_present));
    }

}

TEST_P(Test_01_TwoAsyncSource_SyncFunctional_SyncSink, DataEventsEverySecondEventMissing_0_CompleteSourceFirst) {

    my_facade_->start();



    auto source_0 = std::async([&, local_source = source_0_component_]() {
        try{
            for(size_t i=0;i<kDataEvents;++i){
                auto timestamp = start_timestamp_+time_delta_*i;
                SPDLOG_INFO("send source 0: {0}", timestamp);
                local_source->new_value(timestamp, fmt::format(kTestValue,i));
                wait_for_init.SetInit(true);
            }
        }catch(std::exception e){
            SPDLOG_ERROR(e.what());
        }catch (...){
            SPDLOG_ERROR("some unknown error in async source");
        }

    });
    std::vector<traact::Timestamp> missing_events;
    missing_events.reserve(kDataEvents);
    auto source_1 = std::async([&, local_source = source_1_component_]() {
        try{
            while(!wait_for_init.tryWait()) {
                SPDLOG_INFO("wait for init");
            }
            for(size_t i=0;i<kDataEvents;++i){
                auto timestamp = start_timestamp_+time_delta_*i;
                SPDLOG_INFO("send source 1: {0}, is missing {1}", timestamp, i % 2 == 0);
                if(i % 2 == 0){
                    // no call to framework, data does not exist
                    //local_source->invalid_value(timestamp);
                    missing_events.emplace_back(timestamp);
                } else {
                    local_source->new_value(timestamp, fmt::format(kTestValue,i));
                }
            }
        }catch(std::exception e){
            SPDLOG_ERROR(e.what());
        }catch (...){
            SPDLOG_ERROR("some unknown error in async source");
        }

    });

    source_0.wait();
    source_1.wait();

    my_facade_->stop();

    testDefaultOrder("source_0", source_0_component_->component_state_, 0, 0, kDataEvents, kDataEvents);
    testDefaultOrder("source_1", source_1_component_->component_state_, 0, 0, kDataEvents/2, kDataEvents/2);
    testDefaultOrder("functional", functional_component_->component_state_, kDataEvents/2, kDataEvents/2, 0, 0);
    testDefaultOrder("sink", sink_component_->component_state_, kDataEvents/2, kDataEvents/2, 0, 0);

    {
        auto& source_0_state = source_0_component_->component_state_;
        auto& source_1_state = source_1_component_->component_state_;
        auto& functional_state = functional_component_->component_state_;
        auto& sink_state = sink_component_->component_state_;
        SCOPED_TRACE("source_before_functional_before_sink");
        std::vector<traact::Timestamp> all_events_present{};
        EXPECT_TRUE(source_0_state.precedes(functional_state, all_events_present));
        EXPECT_TRUE(source_1_state.precedes(functional_state, all_events_present ));
        EXPECT_TRUE(source_0_state.precedes(sink_state, all_events_present));
        EXPECT_TRUE(source_1_state.precedes(sink_state, all_events_present));
        EXPECT_TRUE(functional_state.precedes(sink_state, all_events_present));
    }

}


TEST_P(Test_01_TwoAsyncSource_SyncFunctional_SyncSink, DataEventsEverySecondEventMissing_1_MissingSourceFirst) {

    my_facade_->start();


    auto source_0 = std::async([&, local_source = source_0_component_]() {
        try{
            while(!wait_for_init.tryWait()) {
                SPDLOG_INFO("wait for init");
            }
            for(size_t i=0;i<kDataEvents;++i){
                auto timestamp = start_timestamp_+time_delta_*i;
                SPDLOG_INFO("send source 0: {0}", timestamp);
                local_source->new_value(timestamp, fmt::format(kTestValue,i));
            }
        }catch(std::exception e){
            SPDLOG_ERROR(e.what());
        }catch (...){
            SPDLOG_ERROR("some unknown error in async source");
        }

    });
    std::vector<traact::Timestamp> missing_events;
    missing_events.reserve(kDataEvents);
    auto source_1 = std::async([&, local_source = source_1_component_]() {

        try{
            missing_events.emplace_back(start_timestamp_);
            for(size_t i=0;i<kDataEvents;++i){
                auto timestamp = start_timestamp_+time_delta_*i;
                SPDLOG_INFO("send source 1: {0}, is missing {1}", timestamp, i % 2 == 0);
                if(i % 2 == 0){
                    // no call to framework, data does not exist
                    //local_source->invalid_value(timestamp);
                    missing_events.emplace_back(timestamp);
                } else {
                    local_source->new_value(timestamp, fmt::format(kTestValue,i));
                    wait_for_init.SetInit(true);
                }

            }
        }catch(std::exception e){
            SPDLOG_ERROR(e.what());
        }catch (...){
            SPDLOG_ERROR("some unknown error in async source");
        }
    });

    source_0.wait();
    source_1.wait();

    my_facade_->stop();

    testDefaultOrder("source_0", source_0_component_->component_state_, 0, 0, kDataEvents, kDataEvents-1);
    testDefaultOrder("source_1", source_1_component_->component_state_, 0, 0, kDataEvents/2, kDataEvents/2);
    testDefaultOrder("functional", functional_component_->component_state_, kDataEvents/2, kDataEvents/2-1, 0, 0);
    testDefaultOrder("sink", sink_component_->component_state_, kDataEvents/2, kDataEvents/2-1, 0, 0);

    {
        auto& source_0_state = source_0_component_->component_state_;
        auto& source_1_state = source_1_component_->component_state_;
        auto& functional_state = functional_component_->component_state_;
        auto& sink_state = sink_component_->component_state_;
        SCOPED_TRACE("source_before_functional_before_sink");
        std::vector<traact::Timestamp> all_events_present{};
        EXPECT_TRUE(source_0_state.precedes(functional_state, all_events_present));
        EXPECT_TRUE(source_1_state.precedes(functional_state, all_events_present ));
        EXPECT_TRUE(source_0_state.precedes(sink_state, all_events_present));
        EXPECT_TRUE(source_1_state.precedes(sink_state, all_events_present));
        EXPECT_TRUE(functional_state.precedes(sink_state, all_events_present));
    }

}

INSTANTIATE_TEST_SUITE_P(ConcurrentTimeSteps, Test_01_TwoAsyncSource_SyncFunctional_SyncSink, ::testing::Range(1, 11));