/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TaskFlowAsyncScheduler.h"

namespace traact::dataflow {

TaskFlowAsyncScheduler::TaskFlowAsyncScheduler(const buffer::TimeDomainManagerConfig &config,
                                                 std::shared_ptr<
                                                     buffer::TimeDomainBuffer> time_domain_buffer) : config_(config),
                                                                                                     latest_running_ts_(
                                                                                                         config.max_offset),
                                                                                                     time_domain_buffer_(
                                                                                                         time_domain_buffer),
                                                                                                     free_time_step_semaphore_(
                                                                                                         config_.ringbuffer_size,
                                                                                                         config_.ringbuffer_size,
                                                                                                         kFreeTaskFlowTimeout),
                                                                                                     requested_events_semaphore_(
                                                                                                         kMaxRequestedEvents,
                                                                                                         0,
                                                                                                         kFreeTaskFlowTimeout) {
    time_step_count_ = config_.ringbuffer_size;
    scheduled_events_.resize(time_step_count_);
    running_ts_.resize(time_step_count_);
}

TaskFlowAsyncScheduler::Event::Event(const Timestamp &timestamp, EventType event_type, int component_index)
    : timestamp(timestamp), event_type(event_type), component_index(component_index) {}

void TaskFlowAsyncScheduler::configure(std::string graph_name, int time_domain, tf::Taskflow *taskflow) {
    SPDLOG_TRACE("scheduler configure {0} {1}", graph_name, time_domain);
    graph_name_ = graph_name;
    time_domain_ = time_domain;
    taskflow_ = taskflow;

}

void TaskFlowAsyncScheduler::start() {
    SPDLOG_TRACE("scheduler start {0} {1}", graph_name_, time_domain_);
    taskflow_future_ = executor_.run(*taskflow_);
    running_ = true;
    scheduler_task_ = std::async([&]() {
        SPDLOG_TRACE("scheduler task started");
        while (running_) {
            runScheduleTask();
        }
        SPDLOG_TRACE("scheduler task ended");
    });

    if (!taskflow_started_.wait()) {
        SPDLOG_ERROR("could not start taskflow");
        return;
    }
    requestEvent(kTimestampZero, EventType::CONFIGURE, -1);
    while (!configure_finished_.tryWait()) {
        SPDLOG_INFO("waiting for configure message to finish: graph {0} time domain {1}",
                    graph_name_,
                    time_domain_);
    }

    requestEvent(kTimestampZero, EventType::START, -1);
    while (!start_finished_.tryWait()) {
        SPDLOG_INFO("waiting for start message to finish: graph {0} time domain {1}",
                    graph_name_,
                    time_domain_);
    }
}

void TaskFlowAsyncScheduler::globalTaskFlowStart() {
//    for (int time_step_index = 0; time_step_index < time_step_count_; ++time_step_index) {
//        time_domain_buffer_->getTimeStepBuffer(time_step_index).resetNewEvent();
//    }
    taskflow_started_.notify();
}

std::future<buffer::SourceComponentBuffer *> TaskFlowAsyncScheduler::requestSourceBuffer(Timestamp timestamp,
                                                                                          int component_index) {
    SPDLOG_TRACE("scheduler request buffer {0} {1} : ts:{2} component:{3}",
                 graph_name_,
                 time_domain_,
                 timestamp,
                 component_index);

    requestEvent(timestamp, EventType::DATA, component_index);

    return std::async(std::launch::deferred,
                      [&, timestamp, component_index]() -> buffer::SourceComponentBuffer * {
                          //if (config_.source_mode == SourceMode::WAIT_FOR_BUFFER) {
                          latest_running_ts_.wait(timestamp);
                          //}

                          auto time_step_index = getTimeStepForTimestamp(timestamp);
                          if (time_step_index < 0) {
                              SPDLOG_WARN("source component index {0} timestamp {1}, rejected value");
                              return nullptr;
                          } else {
                              SPDLOG_TRACE("source component future, taskflow {0} component {1} timestamp {2}",
                                           time_step_index,
                                           component_index,
                                           timestamp);
                              return time_domain_buffer_->getTimeStepBuffer(time_step_index).getSourceComponentBuffer(
                                  component_index);
                          }
                      });
}

void TaskFlowAsyncScheduler::timeStepEnded(int time_step_index) {
    SPDLOG_TRACE("scheduler time step ended {0} {1}: time step index: {2} ",
                 graph_name_,
                 time_domain_,
                 time_step_index);

    switch (time_domain_buffer_->getTimeStepBuffer(time_step_index).getEventType()) {
        case EventType::CONFIGURE:configure_finished_.SetInit(true);
            break;
        case EventType::START:start_finished_.SetInit(true);
            break;
        case EventType::STOP: stop_finished_.SetInit(true);
            break;
        case EventType::TEARDOWN: teardown_finished_.SetInit(true);
            break;
        default:break;
    }

    time_domain_buffer_->getTimeStepBuffer(time_step_index).resetNewEvent();
    free_time_step_semaphore_.notify();

}

void TaskFlowAsyncScheduler::stop() {
    SPDLOG_TRACE("scheduler stop {0} {1}", graph_name_, time_domain_);
    requestEvent(kTimestampZero, EventType::STOP, -1);
    while (!stop_finished_.tryWait()) {
        SPDLOG_INFO("waiting for stop message to finish: graph {0} time domain {1}",
                    graph_name_, time_domain_);
    }
    requestEvent(kTimestampZero, EventType::TEARDOWN, -1);
    while (!teardown_finished_.tryWait()) {
        SPDLOG_INFO("waiting for teardown message to finish: graph {0} time domain {1}",
                    graph_name_, time_domain_);
    }

    // stop event for every time step in the taskflow
    for (int time_step_index = 0; time_step_index < time_step_count_; ++time_step_index) {
        requestEvent(kTimestampZero, EventType::DATAFLOW_STOP, -1);
    }

    auto status = taskflow_future_.wait_for(kDataflowStopTimeout);
    if (status != std::future_status::ready) {
        SPDLOG_WARN("could not stop task flow graph: {0} time domain: {1}", graph_name_, time_domain_);
    }
    running_ = false;
    auto scheduler_status = scheduler_task_.wait_for(kDataflowStopTimeout);
    if (scheduler_status != std::future_status::ready) {
        SPDLOG_WARN("could not stop scheduler for graph: {0} time domain: {1}", graph_name_, time_domain_);
    }
}

int TaskFlowAsyncScheduler::getTimeStepForTimestamp(Timestamp timestamp) {
    std::unique_lock guard(schedule_lock_);
    for (int i = 0; i < time_step_count_; ++i) {
        if (isWithinRange(timestamp, running_ts_[i], config_.max_offset)) {
            return i;
        }
    }
    return -1;
}

void TaskFlowAsyncScheduler::requestEvent(Timestamp timestamp, EventType type, int component_index) {
    {
        std::unique_lock guard(requested_events_lock_);
        requested_events_.emplace_back(timestamp, type, component_index);
    }

    requested_events_semaphore_.notify();
}

void TaskFlowAsyncScheduler::runScheduleTask() {
    SPDLOG_INFO("check for event to schedule");
    Event next_event;
    {

        if(!requested_events_semaphore_.try_wait(kDataflowStopTimeout)) {
            SPDLOG_TRACE("no events waiting");
            return;
        }
        {
            std::unique_lock guard(requested_events_lock_);
            next_event = requested_events_.front();
            requested_events_.pop_front();
        }

    }
    // get free time step
    while(!free_time_step_semaphore_.try_wait(kDataflowStopTimeout) && running_) {
        SPDLOG_TRACE("timeout waiting for free time step for next event: {0} ts:{1}",next_event.event_type, next_event.timestamp);
    }
    if(!running_){
        return;
    }

    time_step_latest_++;
    time_step_latest_ = time_step_latest_ % time_step_count_;

    SPDLOG_INFO("Run task from queue, using time step: {0} ts: {1} {2} {3}",
                time_step_latest_,
                next_event.timestamp,
                next_event.event_type,
                next_event.component_index);


    running_ts_[time_step_latest_] = next_event.timestamp;
    scheduled_events_[time_step_latest_] = next_event;
    time_step_ready_.notifyAll(time_step_latest_);

    //auto &time_step_buffer = time_domain_buffer_->getTimeStepBuffer(time_step_latest_);
    //time_step_buffer.setEvent(next_event.timestamp, next_event.event_type);

//    if (next_event.event_type == EventType::DATA) {
//        latest_running_ts_.notifyAll(next_event.timestamp);
//    } else {
//        for (int source_index = 0; source_index < time_domain_buffer_->getCountSources(); ++source_index) {
//            time_step_buffer.getSourceComponentBuffer(source_index)->commit(true);
//        }
//    }



}

void TaskFlowAsyncScheduler::timeStepStart(int time_step_index) {
    while(!time_step_ready_.try_wait(time_step_index, kDataflowStopTimeout)) {
        SPDLOG_TRACE("timeout waiting for new data for timestep");
    }
    auto& time_step_buffer = time_domain_buffer_->getTimeStepBuffer(time_step_index);
    time_step_buffer.resetNewEvent();
    auto& event = scheduled_events_[time_step_index];
    time_step_buffer.setEvent(event.timestamp, event.event_type);

    if (event.event_type == EventType::DATA) {
        latest_running_ts_.notifyAll(event.timestamp);
    } else {
        for (int source_index = 0; source_index < time_domain_buffer_->getCountSources(); ++source_index) {
            time_step_buffer.getSourceComponentBuffer(source_index)->commit(true);
        }
    }
}

}

