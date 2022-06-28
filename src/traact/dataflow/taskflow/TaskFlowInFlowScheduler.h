/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWINFLOWSCHEDULER_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWINFLOWSCHEDULER_H_

#include "traact/buffer/SourceComponentBuffer.h"
#include "traact/util/Semaphore.h"
#include "traact/buffer/TimeDomainBuffer.h"
#include "TaskFlowUtils.h"
#include <taskflow/taskflow.hpp>
#include "TimeDomainClock.h"

namespace traact::dataflow {
class TaskFlowInFlowScheduler {
 public:
    TaskFlowInFlowScheduler(const buffer::TimeDomainManagerConfig &config, std::shared_ptr<
        buffer::TimeDomainBuffer> time_domain_buffer, std::string graph_name, int time_domain, tf::Taskflow *taskflow, std::function<void(void)> on_timeout);
    ~TaskFlowInFlowScheduler();
    void start();
    void globalTaskFlowStart();

    void timeStepStart(int time_step_index);
    std::future<buffer::SourceComponentBuffer *> requestSourceBuffer(Timestamp timestamp, int component_index);
    void timeStepEnded(int time_step_index);

    void stop();

    void propertyChanged();

 private:
    struct ScheduledEvent {
        ScheduledEvent() = default;
        ScheduledEvent(Timestamp &timestamp, EventType event_type, std::vector<int> invalid_sources);
        Timestamp timestamp{Timestamp::max()};
        EventType event_type{EventType::INVALID};
        std::vector<int> invalid_sources{};
    };
    buffer::TimeDomainManagerConfig config_;
    std::shared_ptr<buffer::TimeDomainBuffer> time_domain_buffer_;

    std::string graph_name_;
    int time_domain_;
    tf::Taskflow *taskflow_;

    tf::Executor executor_;
    tf::Future<void> taskflow_future_;
    std::future<void> scheduler_task_;
    std::atomic_bool running_{false};

    traact::WaitForInit configure_finished_;
    traact::WaitForInit start_finished_;
    traact::WaitForInit stop_finished_;
    traact::WaitForInit teardown_finished_;
    traact::Semaphore taskflow_started_{1, 0, kFreeTaskFlowTimeout};

    Timestamp latest_scheduled_ts_;
    int time_step_count_;
    WaitForTimestamp latest_running_ts_;
    std::vector<std::vector<std::unique_ptr<std::atomic_bool>>> source_set_input_;
    std::mutex flow_mutex_;
    std::vector<bool> running_taskflows_;
    std::deque<ScheduledEvent> queued_messages_;
    int time_step_latest_;
    std::mutex running_mutex_;
    std::vector<Timestamp> running_timestamps_;
    Semaphore free_taskflows_semaphore_;
    std::mutex request_mutex_;
    Timestamp latest_queued_ts_;
    std::vector<Timestamp> latest_scheduled_component_timestamp_;
    //-----------------------------------------
    TimeDomainClock time_domain_clock_;
    bool stop_scheduled_{false};
    std::function<void(void)> on_timeout_;



    void cancelOlderEvents(Timestamp timestamp, int component_index);
    void takeTaskflow(int taskflow_id, Timestamp &next_ts);
    void runTaskFlowFromQueue();

    void setTaskflowFree(int time_step_index);
    int isTimestampRunning(const Timestamp &timestamp);

    void scheduleEvent(EventType message_type, Timestamp timestamp);

    void freeTimeStep(int time_step_index);

    void scheduleNonDataEventAndWait(EventType type, WaitForInit &init_finished);
    std::future<buffer::SourceComponentBuffer *> requestSourceBufferRunning(Timestamp timestamp,
                                                                            int component_index,
                                                                            int time_step_index);
    bool isTimestampScheduled(Timestamp timestamp);
    std::future<buffer::SourceComponentBuffer *> requestSourceBufferScheduled(Timestamp timestamp,
                                                                              int component_index);
    void cancelSourceWaitForEvent(Timestamp timestamp, int component_index);
    void cancelSourceCancelOldest(Timestamp timestamp);
    std::future<buffer::SourceComponentBuffer *> requestSourceBufferNewTimestamp(Timestamp timestamp,
                                                                                 int component_index);
    std::future<buffer::SourceComponentBuffer *> requestSourceBufferNewTimestampWait(Timestamp timestamp,
                                                                                     int component_index);
    std::future<buffer::SourceComponentBuffer *> requestSourceBufferNewTimestampImmediate(Timestamp timestamp,
                                                                                          int component_index);

    bool isInvalidNextRequestedTimeStamp(Timestamp timestamp, int component_index);
    std::future<buffer::SourceComponentBuffer *> requestSourceBufferInvalid();

    void schedulePaddingEvent(EventType message_type, Timestamp timestamp, int component_index);
    int scheduleDataEventImmediately(Timestamp timestamp, int component_index);
};
}

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWINFLOWSCHEDULER_H_
