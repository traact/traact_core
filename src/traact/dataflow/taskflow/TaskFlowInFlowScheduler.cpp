/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TaskFlowInFlowScheduler.h"

#include <utility>

namespace traact::dataflow {
TaskFlowInFlowScheduler::TaskFlowInFlowScheduler(const buffer::TimeDomainManagerConfig &config, std::shared_ptr<
    buffer::TimeDomainBuffer> time_domain_buffer, std::string graph_name, int time_domain, tf::Taskflow *taskflow)
    : config_(config),
      time_domain_buffer_(std::move(time_domain_buffer)),
      executor_(config_.cpu_count > 0 ? config_.cpu_count : std::thread::hardware_concurrency() - config_.cpu_count),
      time_step_count_(config_.ringbuffer_size), latest_running_ts_{config_.max_offset},
      time_step_latest_(-1), free_taskflows_semaphore_(config_.ringbuffer_size,
                                config_.ringbuffer_size,
                                kFreeTaskFlowTimeout), graph_name_(std::move(graph_name)), time_domain_(time_domain),
      taskflow_(taskflow),
      time_domain_clock_(config_.sensor_frequency, config_.max_offset, 1.0) {



    running_taskflows_.resize(time_step_count_, false);
    running_timestamps_.resize(time_step_count_, kTimestampZero);
    latest_scheduled_component_timestamp_.resize(time_domain_buffer_->getCountSources(), kTimestampZero);

    //
    //tmp.resize(time_step_count_, std::atomic_bool(false));
    for (int i = 0; i < time_domain_buffer_->getCountSources(); ++i) {
        std::vector<std::unique_ptr<std::atomic_bool>> tmp;
        for (int j = 0; j < time_step_count_; ++j) {
            tmp.emplace_back(std::make_unique<std::atomic_bool>(false));
        }
        source_set_input_.emplace_back(std::move(tmp));
    }

    latest_scheduled_ts_ = kTimestampZero;

}

TaskFlowInFlowScheduler::ScheduledEvent::ScheduledEvent(Timestamp &timestamp,
                                                        EventType event_type,
                                                        std::vector<int> invalid_sources)
    : timestamp(timestamp), event_type(event_type), invalid_sources(std::move(invalid_sources)) {}

void TaskFlowInFlowScheduler::start() {

    // initialize all time domain buffer to allow all async sources to request the future
    // this has to happen before taskflow tasks request futures from the source buffers
    // since they do that as soon as the taskflow starts, before it is even known
    // what the time step will be used for or what timestamp is has
    // reset at the end of the time step so the promises of the source buffers are ready for the use in the next time step
    for (int time_step_index = 0; time_step_index < time_step_count_; ++time_step_index) {
        time_domain_buffer_->getTimeStepBuffer(time_step_index).resetNewEvent();
    }

    taskflow_future_ = executor_.run(*taskflow_);

    while (!taskflow_started_.try_wait(kDataflowStopTimeout)) {
        SPDLOG_ERROR("timeout trying to start taskflow");
    }

    scheduleNonDataEventAndWait(EventType::CONFIGURE, configure_finished_);
    scheduleNonDataEventAndWait(EventType::START, start_finished_);
}
void TaskFlowInFlowScheduler::globalTaskFlowStart() {
    SPDLOG_TRACE("start time domain: {0}", time_domain_);
    taskflow_started_.notify();
}
void TaskFlowInFlowScheduler::timeStepStart(int time_step_index) {

}
void TaskFlowInFlowScheduler::timeStepEnded(int time_step_index) {
    SPDLOG_TRACE("timeStepEnded: {0} timestamp: {1} start",
                 time_step_index,
                 time_domain_buffer_->getTimeStepBuffer(time_step_index).getTimestamp());
    std::unique_lock guard(request_mutex_);
    SPDLOG_DEBUG("timeStepEnded: {0} timestamp: {1} has lock",
                 time_step_index,
                 time_domain_buffer_->getTimeStepBuffer(time_step_index).getTimestamp());
    freeTimeStep(time_step_index);
    switch (time_domain_buffer_->getTimeStepBuffer(time_step_index).getEventType()) {
        case EventType::CONFIGURE:configure_finished_.SetInit(true);
            break;
        case EventType::START:start_finished_.SetInit(true);
            break;
        case EventType::STOP: stop_finished_.SetInit(true);
            break;
        case EventType::TEARDOWN: teardown_finished_.SetInit(true);
            break;
        case EventType::INVALID:
        case EventType::DATA:
        case EventType::DATAFLOW_NO_OP:
        case EventType::DATAFLOW_STOP:break;
        default:SPDLOG_ERROR("Unknown EventType message in dataflow {0}",
                             static_cast<int>(time_domain_buffer_->getTimeStepBuffer(time_step_index).getEventType()));
            break;

    }
}
void TaskFlowInFlowScheduler::stop() {

    try{
        scheduleNonDataEventAndWait(EventType::STOP, stop_finished_);
        scheduleNonDataEventAndWait(EventType::TEARDOWN, teardown_finished_);

        // stop event for every time step in the taskflow
        for (int time_step_index = 0; time_step_index < time_step_count_; ++time_step_index) {
            scheduleEvent(EventType::DATAFLOW_STOP, kTimestampZero);
        }

//        auto cancel_result = taskflow_future_.cancel();
//        SPDLOG_INFO("cancel task flow {0}", cancel_result);
//        time_domain_buffer_->cancelAll();

        auto status = taskflow_future_.wait_for(kDataflowStopTimeout);
        if (status != std::future_status::ready) {
            SPDLOG_WARN("could not stop task flow time domain {0} {1}", graph_name_, time_domain_);
        }
    } catch (std::exception e){
        SPDLOG_ERROR(e.what());
    } catch (...){
        SPDLOG_ERROR("unknown error trying to stop dataflow");
    }

}

std::future<buffer::SourceComponentBuffer *>
TaskFlowInFlowScheduler::requestSourceBuffer(Timestamp timestamp, int component_index) {

    SPDLOG_TRACE("request from: {0} timestamp: {1} start", component_index, timestamp);
    std::unique_lock guard(request_mutex_);
    SPDLOG_TRACE("request from: {0} timestamp: {1} has lock", component_index, timestamp);

    /**
     * check if timestamp is newer then last requested timestamp of this source.
     * the timestamp must be increasing, no source can request a timestamp twice or
     * go back in time.
     * -> return invalid request
     * check if timestamp is older then first event, e.g. some sensor was slow to start sending the first event, a different sensor
     * was faster but had the first event missing
     * -> return invalid request
     */
    if (isInvalidNextRequestedTimeStamp(timestamp, component_index)) {
        SPDLOG_ERROR(
            "request from: {0} timestamp: {1} latest scheduled timestamp: {2} : not newer then the latest requested timestamp of this component",
            component_index,
            timestamp,
            latest_scheduled_component_timestamp_.at(component_index));

        return requestSourceBufferInvalid();
    }

    time_domain_clock_.newTimestamp(timestamp);

    /**
     * first cancel any older time steps depending time domain configuration MissingSourceEventMode.
     * check whether timestamp is running or scheduled
     */
    cancelOlderEvents(timestamp, component_index);

    /**
     * if timestamp is already running in a time step, return future that directly returns the source component buffer
     * SourceMode has no impact
     */
    auto running_time_step_index = isTimestampRunning(timestamp);
    if (running_time_step_index >= 0) {
        SPDLOG_TRACE("component: {0} timestamp: {1} request completed with running time step {2}",
                     component_index,
                     timestamp,
                     running_time_step_index);
        return requestSourceBufferRunning(timestamp, component_index, running_time_step_index);
    }
    /**
     * SourceMode::IMMEDIATE_RETURN: there can not be a scheduled timestamp, it is either running in a time step or not
     * SourceMode::WAIT_FOR_BUFFER: it might already be scheduled by a different async source thread
     *                                  -> return future that waits util the scheduled timestamp is running in a time step
     */
    if (config_.source_mode == SourceMode::WAIT_FOR_BUFFER) {
        if (isTimestampScheduled(timestamp)) {
            SPDLOG_TRACE("component: {0} timestamp: {1} request completed with scheduled time step",
                         component_index,
                         timestamp);
            return requestSourceBufferScheduled(timestamp, component_index);
        }
    }

    /**
     * neither running or scheduled, so it is a new timestamp
     * how a new timestamp is handled depends on the time domain configuration SourceMode
     */
    SPDLOG_TRACE("request from: {0} timestamp: {1} requesting new time step", component_index, timestamp);
    latest_scheduled_component_timestamp_.at(component_index) = timestamp;
    return requestSourceBufferNewTimestamp(timestamp, component_index);
}

void TaskFlowInFlowScheduler::freeTimeStep(int time_step_index) {

    std::unique_lock flow_guard(flow_mutex_);
    SPDLOG_TRACE("Free Time Step {0}, Reset Locks of ts: {1}",
                 time_step_index,
                 time_domain_buffer_->getTimeStepBuffer(time_step_index).getTimestamp());
    time_domain_buffer_->getTimeStepBuffer(time_step_index).resetNewEvent();
    running_taskflows_[time_step_index] = false;

    if (queued_messages_.empty()) {
        setTaskflowFree(time_step_index);
    } else {
        SPDLOG_TRACE("Free Time Step {0}, run from queue, queued messages {1}",
                     time_step_index,
                     queued_messages_.size());
        runTaskFlowFromQueue();
    }

}

int TaskFlowInFlowScheduler::isTimestampRunning(const Timestamp &timestamp) {
    std::unique_lock guard_running(running_mutex_);
    SPDLOG_TRACE("search for running time step ts: {0} ", timestamp);
    for (size_t index = 0; index < running_timestamps_.size(); ++index) {
        if (isWithinRange(timestamp, running_timestamps_[index], config_.max_offset)) {
            SPDLOG_TRACE("search for running time step ts: {0} match found {1} {2}",
                         timestamp,
                         running_timestamps_[index],
                         index);
            return index;
        }

    }

    SPDLOG_TRACE("search for running time step ts: {0} no match found", timestamp);
    return -1;
}

void TaskFlowInFlowScheduler::setTaskflowFree(int time_step_index) {
    {
        std::unique_lock guard_running(running_mutex_);
        running_taskflows_[time_step_index] = false;
        running_timestamps_[time_step_index] = kTimestampZero;
    }
    free_taskflows_semaphore_.notify();
}

void TaskFlowInFlowScheduler::takeTaskflow(int taskflow_id,
                                           Timestamp &next_ts) {
    std::unique_lock guard_running(running_mutex_);
    running_taskflows_[taskflow_id] = true;
    running_timestamps_[taskflow_id] = next_ts;
    for (int source_index = 0; source_index < time_domain_buffer_->getCountSources(); ++source_index) {
        source_set_input_[source_index][time_step_latest_]->store(false, std::memory_order_relaxed);
    }
}

void TaskFlowInFlowScheduler::cancelOlderEvents(Timestamp timestamp, int component_index) {
    std::unique_lock guard_flow(flow_mutex_);
    SPDLOG_TRACE("check for events to cancel for component: {0} ts: {1}", component_index, timestamp);
    switch (config_.missing_source_event_mode) {
        case MissingSourceEventMode::WAIT_FOR_EVENT: {
            cancelSourceWaitForEvent(timestamp, component_index);
            break;
        }
        case MissingSourceEventMode::CANCEL_OLDEST: {
            cancelSourceCancelOldest(timestamp);
            break;
        }
        default: {
            SPDLOG_ERROR("Invalid type of MissingSourceEventMode: {0}",
                         static_cast<int>(config_.missing_source_event_mode));
            break;
        }
    }
}

/**
 * MissingSourceEventMode::WAIT_FOR_EVENT:
 *      Time steps are only canceled by the component itself.
 *      it is save to cancel older time steps of this component because if it could set them it would have
 *      already done so.
 *      For other components wait for their respective event, at some point they will send something
 */
void TaskFlowInFlowScheduler::cancelSourceWaitForEvent(Timestamp timestamp, int component_index) {

    // cancel running time steps
    auto min_ts = timestamp - config_.max_offset;
    for (int time_step_index = 0; time_step_index < time_step_count_; ++time_step_index) {
        auto is_not_set = !source_set_input_[component_index][time_step_index]->load(std::memory_order_relaxed);
        if (is_not_set && running_taskflows_[time_step_index]
            && running_timestamps_[time_step_index] + config_.max_offset < min_ts) {
            auto &time_step_buffer = time_domain_buffer_->getTimeStepBuffer(time_step_index);
            SPDLOG_WARN(
                "cancel source component buffer in running time step, because a newer timestamp is requested, time step: {0} component {1} buffer timestamp: {2} current timestamp {3}",
                time_step_index,
                component_index,
                time_step_buffer.getTimestamp(),
                timestamp);
            source_set_input_[component_index][time_step_index]->store(true, std::memory_order_relaxed);

            time_step_buffer.getSourceComponentBuffer(component_index)->cancel();

        }
    }
    // cancel in queue as well
    for (auto &queued_message : queued_messages_) {
        if (queued_message.event_type == EventType::DATA && queued_message.timestamp + config_.max_offset < min_ts) {
            SPDLOG_WARN(
                "cancel source component buffer in queued messages, because a newer timestamp is requested, queued ts: {0} component: {1} current timestamp: {2}",
                queued_message.timestamp,
                component_index,
                timestamp);
            queued_message.invalid_sources.emplace_back(component_index);
        }
    }

}

/**
 * MissingSourceEventMode::CANCEL_OLDEST:
 *      Only useful in live systems.
 *      Cancel any older time steps of all components, because since a new timestamp of this synchronized time domain
 *      arrived, any other sensor in this time domain should have already set its data in the time step.
 *      if not then they lag behind by at least one frame, then you have problem with your synchronized sensor setup,
 *      source component or driver. for some reason the synchronized sensor are not able to deliver their data,
 *      or at least an invalid event, within their time budget
 */
void TaskFlowInFlowScheduler::cancelSourceCancelOldest(Timestamp timestamp) {
    for (int component_index = 0; component_index < source_set_input_.size(); ++component_index) {
        cancelSourceWaitForEvent(timestamp, component_index);
    }
}

void TaskFlowInFlowScheduler::scheduleNonDataEventAndWait(EventType type, WaitForInit &init_finished) {

    {
        std::unique_lock guard(request_mutex_);
        scheduleEvent(type, kTimestampZero);
        if(type == EventType::STOP){
            stop_scheduled_ = true;
        }

    }


    while (!init_finished.tryWait()) {
        SPDLOG_WARN("waiting for event to finish: graph: {0} time domain: {1} event type: {2}",
                    graph_name_, time_domain_, type);
    }
}

bool TaskFlowInFlowScheduler::isTimestampScheduled(Timestamp timestamp) {
    std::unique_lock guard(flow_mutex_);
    SPDLOG_TRACE("is scheduled, search for {0}", timestamp);
    for (const auto &message : queued_messages_) {
        if (isWithinRange(message.timestamp, timestamp, config_.max_offset)) {
            SPDLOG_TRACE("is scheduled, search for {0}, match {1} in queued messages, diff: {2}",
                         timestamp,
                         message.timestamp,
                         timestamp - message.timestamp);
            return true;
        }

    }
    SPDLOG_TRACE("is scheduled, search for {0}, no match found", timestamp);
    return false;
}

std::future<buffer::SourceComponentBuffer *> TaskFlowInFlowScheduler::requestSourceBufferRunning(Timestamp timestamp,
                                                                                                 int component_index,
                                                                                                 int time_step_index) {
    source_set_input_[component_index][time_step_index]->store(true, std::memory_order_relaxed);
    return std::async(std::launch::deferred,
                      [&, timestamp, component_index, time_step_index]() -> buffer::SourceComponentBuffer * {
                          SPDLOG_TRACE("requestSourceBufferRunning, time step: {0} component: {1} timestamp: {2}",
                                       time_step_index, component_index, timestamp);
                          return time_domain_buffer_->getTimeStepBuffer(time_step_index).getSourceComponentBuffer(
                              component_index);
                      });
}

std::future<buffer::SourceComponentBuffer *> TaskFlowInFlowScheduler::requestSourceBufferScheduled(Timestamp timestamp,
                                                                                                   int component_index) {
    return std::async(std::launch::deferred,
                      [&, timestamp, component_index]() -> buffer::SourceComponentBuffer * {
                          latest_running_ts_.wait(timestamp);

                          auto time_step_index = isTimestampRunning(timestamp);
                          if (time_step_index < 0) {
                              SPDLOG_WARN(
                                  "requestSourceBufferScheduled, source component index {0} timestamp {1}, rejected value");
                              return nullptr;
                          } else {
                              source_set_input_[component_index][time_step_index]->store(true, std::memory_order_relaxed);
                              SPDLOG_TRACE(
                                  "requestSourceBufferScheduled, source component future, taskflow {0} component {1} timestamp {2}",
                                  time_step_index,
                                  component_index,
                                  timestamp);
                              return time_domain_buffer_->getTimeStepBuffer(time_step_index).getSourceComponentBuffer(
                                  component_index);
                          }
                      });
}

std::future<buffer::SourceComponentBuffer *> TaskFlowInFlowScheduler::requestSourceBufferNewTimestamp(Timestamp timestamp,
                                                                                                      int component_index) {
    /**
     * if the stop event is scheduled then refuse all new data events
     */
    if(stop_scheduled_){
        return requestSourceBufferInvalid();
    }
    switch (config_.source_mode) {
        case SourceMode::WAIT_FOR_BUFFER: {
            return requestSourceBufferNewTimestampWait(timestamp, component_index);
        }

        case SourceMode::IMMEDIATE_RETURN: {
            return requestSourceBufferNewTimestampImmediate(timestamp, component_index);
        }
        default: {
            SPDLOG_ERROR("Unknown SourceMode: {0}", static_cast<int>(config_.source_mode));
            throw std::invalid_argument(fmt::format("Unknown SourceMode: {0}", static_cast<int>(config_.source_mode)));
        }
    }
}

/**
 * SourceMode::WAIT_FOR_BUFFER
 *      -> schedule timestamp and wait for free time step
 *
 *      It will always return a requestSourceBufferScheduled.
 *      Since the task will wait for a free time step indefinitely (in principle),
 *      the scheduler has to take different possible scenarios into account:
 *          0: timestamp is older:
 *              0.0 already running in a time step started by a different source -> already covered by check in beginning
 *              0.1 already scheduled by different source -> already covered by check in beginning
 *          1: timestamp is next expected time step:
 *              1.0 nothing to consider -> schedule event
 *          2: timestamp is further ahead then the next expected time step:
 *          (no other source requested it, because then it would already be scheduled at least)
 *              2.0 from record: recording had completely missing events
 *                  -> other sources might request this event later
 *                  -> this source will never request those timestamps and fill the data
 *                  --> schedule data events for all missing timestamps and mark the source as immediate cancel when it is run
 *              2.1 live system:
 *                  -> this source (senor) must have had a missing event and did not even report and invalid event
 *                  -> other synchronous sources (sensor)
 *                      -> are either missing this event as well
 *                      -> are very slow for some reason and are sill in the process of delivering the data and might still request the missing timestamps
 *                  --> schedule data events for all missing timestamps and mark the source as immediate cancel when it is run
 */
std::future<buffer::SourceComponentBuffer *> TaskFlowInFlowScheduler::requestSourceBufferNewTimestampWait(Timestamp timestamp,
                                                                                                          int component_index) {
    if (isNextExpectedTimestamp(timestamp)) {
        // 1: timestamp is next expected time step
        auto expected_timestamp = time_domain_clock_.getNextExpectedTimestamp();
        SPDLOG_TRACE("Schedule event ts: {0} {1} with expected timestamp: {2}",
                     timestamp,
                     EventType::DATA,
                     expected_timestamp);
        scheduleEvent(EventType::DATA, expected_timestamp);
    } else if (isFurtherAheadThenNextExpectedTimestamp(timestamp)) {
        // 2: timestamp is further ahead then the next expected time step

        Timestamp queue_timestamp;
        do {
            queue_timestamp = getNextExpectedDataTimestamp();
            SPDLOG_WARN("scheduling padding event ts: {0}", queue_timestamp);
            schedulePaddingEvent(EventType::DATA, queue_timestamp, component_index);
        } while (!isNextExpectedTimestamp(timestamp));

        auto expected_timestamp = time_domain_clock_.getNextExpectedTimestamp();
        SPDLOG_TRACE("Schedule event ts: {0} {1} with expected timestamp: {2}",
                     timestamp,
                     EventType::DATA,
                     expected_timestamp);
        scheduleEvent(EventType::DATA, expected_timestamp);
    } else {
        throw std::invalid_argument(fmt::format(
            "trying to requestSourceBufferNewTimestampWait, but timestamp was valid (newer than any other reported by this component), not running or scheduled, not the next time step or any after that. time continuum broke for for timestamp: {0} component index: {1}",
            timestamp,
            component_index));
    }

    return requestSourceBufferScheduled(timestamp, component_index);
}

/**
 * SourceMode::IMMEDIATE_RETURN
 *      -> if time step is free: take time step and set it this timestamp.
 *      -> Otherwise: return null SourceBuffer
 *
 *      need bookkeeping about rejected timestamps so that when it is first rejected for a source,
 *      that every other source that later requests this timestamp,
 *      when a different time step might have finished and is free,
 *      gets also rejected, since a full time step will never be completed.
 *      This happens when the time step can not finish withing one frame.
 *      This in turn leads to the concurrent time steps to run full so that all are in use.
 *      It might be possible to increase the concurrent time steps to allow more parallel processing.
 *      Only works when the task graph can be run in parallel at the bottlenecks and there are still free
 *      computing resources. Otherwise increasing the concurrent time steps will only increase the delay of the
 *      whole system.
 *      Because of that: reject a complete frame since not all input data can be processed anyway no matter how big the buffers
 *      become.
 *      Maybe adaptive frame selection, to automatically reject every second, third or so frame depending on system load
 */
std::future<buffer::SourceComponentBuffer *> TaskFlowInFlowScheduler::requestSourceBufferNewTimestampImmediate(Timestamp timestamp,
                                                                                                               int component_index) {
    auto new_time_step = scheduleDataEventImmediately(timestamp, component_index);
    if (new_time_step >= 0) {
        return requestSourceBufferRunning(timestamp, component_index, new_time_step);
    } else {
        return requestSourceBufferInvalid();
    }
}
bool TaskFlowInFlowScheduler::isNextExpectedTimestamp(Timestamp timestamp) {
    return time_domain_clock_.isNextExpectedTimestamp(timestamp);
}
bool TaskFlowInFlowScheduler::isFurtherAheadThenNextExpectedTimestamp(Timestamp timestamp) {
    return time_domain_clock_.isFurtherAheadThenNextExpectedTimestamp(timestamp);
}
bool TaskFlowInFlowScheduler::isInvalidNextRequestedTimeStamp(Timestamp timestamp, int component_index) {
    if (timestamp + config_.max_offset < time_domain_clock_.getInitTimestamp() - config_.max_offset) {
        SPDLOG_ERROR("timestamp smaller then initial timestep: initial: {0} requested: {1}", time_domain_clock_.getInitTimestamp(), timestamp);
        return true;
    }

    return timestamp <= latest_scheduled_component_timestamp_.at(component_index);
}
std::future<buffer::SourceComponentBuffer *> TaskFlowInFlowScheduler::requestSourceBufferInvalid() {
    std::promise<buffer::SourceComponentBuffer *> value;
    value.set_value(nullptr);
    return value.get_future();
}
Timestamp TaskFlowInFlowScheduler::getNextExpectedDataTimestamp() {
    return time_domain_clock_.getNextExpectedTimestamp();
}

void TaskFlowInFlowScheduler::scheduleEvent(EventType message_type, Timestamp timestamp) {
    SPDLOG_TRACE("Schedule event ts: {0} {1}", timestamp, message_type);
    std::unique_lock guard_flow(flow_mutex_);

    queued_messages_.emplace_back(timestamp, message_type, std::vector<int>{});

    if (free_taskflows_semaphore_.try_wait()) {
        SPDLOG_TRACE("Schedule event ts: {0} {1}, run from queue",
                     timestamp, message_type);
        runTaskFlowFromQueue();
    }

}
void TaskFlowInFlowScheduler::schedulePaddingEvent(EventType message_type, Timestamp timestamp, int component_index) {
    std::unique_lock guard_flow(flow_mutex_);

    latest_scheduled_ts_ = timestamp;
    queued_messages_.emplace_back(timestamp, message_type, std::vector<int>{component_index});

    if (free_taskflows_semaphore_.try_wait()) {
        SPDLOG_TRACE("Schedule event ts: {0} {1}, run from queue",
                     timestamp, message_type);
        runTaskFlowFromQueue();
    }
}
int TaskFlowInFlowScheduler::scheduleDataEventImmediately(Timestamp timestamp, int component_index) {
    std::unique_lock guard_flow(flow_mutex_);

    if (free_taskflows_semaphore_.try_wait()) {
        SPDLOG_TRACE("Schedule event immediately ts: {0} {1}", timestamp, EventType::DATA);

        latest_scheduled_ts_ = time_domain_clock_.getNextExpectedTimestamp();

        time_step_latest_++;
        time_step_latest_ = time_step_latest_ % time_step_count_;

        assert(!running_taskflows_[time_step_latest_]);
        if (latest_running_ts_.currentValue() == timestamp) {
            SPDLOG_ERROR("same timestamp scheduled twice {0}", timestamp);
        }

        takeTaskflow(time_step_latest_, timestamp);
        auto &time_step_buffer = time_domain_buffer_->getTimeStepBuffer(time_step_latest_);
        time_step_buffer.setEvent(latest_scheduled_ts_, EventType::DATA);

        latest_running_ts_.notifyAll(timestamp);

        return time_step_latest_;
    } else {
        -1;
    }
}

void TaskFlowInFlowScheduler::runTaskFlowFromQueue() {

    auto next_message = queued_messages_.front();
    queued_messages_.pop_front();

    time_step_latest_++;
    time_step_latest_ = time_step_latest_ % time_step_count_;

    SPDLOG_INFO("Run task from queue, using time step: {0} ts: {1} {2}",
                time_step_latest_, next_message.timestamp, next_message.event_type);
    assert(!running_taskflows_[time_step_latest_]);
    if (next_message.event_type == EventType::DATA && latest_running_ts_.currentValue() == next_message.timestamp) {
        SPDLOG_ERROR("same timestamp scheduled twice {0}", next_message.timestamp);
    }

    takeTaskflow(time_step_latest_, next_message.timestamp);
    auto &time_step_buffer = time_domain_buffer_->getTimeStepBuffer(time_step_latest_);
    time_step_buffer.setEvent(next_message.timestamp, next_message.event_type);

    if (next_message.event_type == EventType::DATA) {
        latest_running_ts_.notifyAll(next_message.timestamp);
        for (const auto &invalid_source : next_message.invalid_sources) {
            auto *source_buffer = time_step_buffer.getSourceComponentBuffer(invalid_source);
            SPDLOG_TRACE("source for scheduled time step already marked as invalid, component index: {0} ts: {1}",
                         invalid_source, source_buffer->getTimestamp());
            source_set_input_[invalid_source][time_step_latest_]->store(true, std::memory_order_relaxed);
            source_buffer->commit(true);
        }
    } else {
        SPDLOG_TRACE("non data event, commit all source components");
        for (int source_index = 0; source_index < time_domain_buffer_->getCountSources(); ++source_index) {
            source_set_input_[source_index][time_step_latest_]->store(true,std::memory_order_relaxed);
            time_step_buffer.getSourceComponentBuffer(source_index)->commit(true);
        }

    }

}
TaskFlowInFlowScheduler::~TaskFlowInFlowScheduler() {

}

} // traact