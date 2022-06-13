/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWASYNCSCHEDULER_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWASYNCSCHEDULER_H_

#include "traact/buffer/SourceComponentBuffer.h"
#include "traact/util/Semaphore.h"
#include "traact/buffer/TimeDomainBuffer.h"
#include "TaskFlowUtils.h"
#include <taskflow/taskflow.hpp>

namespace traact::dataflow {
class TaskFlowAsyncScheduler {
 public:
    TaskFlowAsyncScheduler(const buffer::TimeDomainManagerConfig &config,
                            std::shared_ptr<
                                buffer::TimeDomainBuffer> time_domain_buffer);

    void configure(std::string graph_name, int time_domain, tf::Taskflow *taskflow);
    void start();
    void globalTaskFlowStart();

    void timeStepStart(int time_step_index);
    std::future<buffer::SourceComponentBuffer *> requestSourceBuffer(Timestamp timestamp, int component_index);
    void timeStepEnded(int time_step_index);


    void stop();



 private:
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

    //-----------------------------------------
    static const constexpr int kMaxRequestedEvents = 10;
    struct Event {
        Event(const Timestamp &timestamp, EventType event_type, int component_index);
        Event() = default;
        Timestamp timestamp;
        EventType event_type;
        int component_index;
    };
    std::mutex schedule_lock_;

    Semaphore free_time_step_semaphore_;
    WaitForTimestamp latest_running_ts_;

    std::mutex requested_events_lock_;
    std::deque<Event> requested_events_;
    Semaphore requested_events_semaphore_;

    std::vector<Timestamp> running_ts_;

    WaitForExactValue<int> time_step_ready_;
    std::vector<Event> scheduled_events_;

    int time_step_count_{};
    int time_step_latest_{-1};
    int running_min_{-1};
    int running_max_{-1};

    //std::vector<Timestamp>








    void runScheduleTask();

    int getTimeStepForTimestamp(Timestamp timestamp);
    void requestEvent(Timestamp timestamp, EventType type, int component_index);
};
}

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWASYNCSCHEDULER_H_
