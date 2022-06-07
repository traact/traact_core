/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#include "TaskFlowTimeDomain.h"
#include <utility>

std::string timeStepStartTaskName(int index) {
    return fmt::format("TIME_STEP_START_{0}", index);
}
std::string timeStepEndTaskName(int index) {
    return fmt::format("TIME_STEP_END_{0}", index);
}

static const std::string kTimeStepStart = "TIME_STEP_START";
static const std::string kTimeStepEnd = "TIME_STEP_END";
static const std::string kSeamEntryFormat = "{0}_SEAM_{1}";
static const std::string kSeamStartFormat = "{0}_SEAM_START_{1}";
static const std::string kSeamEndFormat = "{0}_SEAM_END_{1}";

namespace traact::dataflow {

TaskFlowTimeDomain::TaskFlowTimeDomain(int time_domain,
                                       DefaultComponentGraphPtr component_graph,
                                       buffer::DataBufferFactoryPtr buffer_factory,
                                       component::Component::SourceFinishedCallback callback)
    : generic_factory_objects_(std::move(buffer_factory)),
      source_finished_callback_(std::move(callback)),
      component_graph_(std::move(component_graph)),
      time_domain_(time_domain),
      free_taskflows_semaphore_(component_graph_->getTimeDomainConfig(time_domain_).ringbuffer_size,
                                component_graph_->getTimeDomainConfig(time_domain_).ringbuffer_size,
                                kFreeTaskFlowTimeout),
      time_domain_buffer_(std::make_unique<buffer::TimeDomainBuffer>(time_domain_, generic_factory_objects_)),
      latest_running_ts_{component_graph_->getTimeDomainConfig(time_domain_).max_offset} {

}

void TaskFlowTimeDomain::init() {

    createBuffer();

    prepare();

    taskflow_.name(fmt::format("{0}_TD:{1}", component_graph_->getName(), time_domain_));

    for (int time_step_index = 0; time_step_index < time_step_count_; ++time_step_index) {
        createTimeStepTasks(time_step_index);
    }

    createInterTimeStepDependencies();

    SPDLOG_TRACE("dump of task flow \n{0}", taskflow_.dump());

    for (int time_step_index = 0; time_step_index < time_step_count_; ++time_step_index) {
        time_domain_buffer_->getTimeStepBuffer(time_step_index).setEvent(Timestamp::min(), EventType::INVALID);
    }
}

void TaskFlowTimeDomain::createTimeStepTasks(const int time_step_index) {

    auto &time_step_data = time_step_data_[time_step_index];
    for (const auto &component : components_) {

        std::shared_ptr<pattern::instance::PatternInstance> pattern_instance = component.first;
        if (!pattern_instance) {
            continue;
        }
        createTask(time_step_index, time_step_data, component);

    }

    for (const auto &component_successors : component_to_successors_) {
        auto &task = time_step_data.component_id_to_task.at(component_successors.first);
        for (const auto &successor : component_successors.second) {
            task.precede(time_step_data.component_id_to_task.at(successor));
        }
    }

    auto start_task = createLocalStartTask(time_step_index, timeStepStartTaskName(time_step_index));
    for (const auto &start_point : component_start_points_) {
        start_task.precede(time_step_data.component_id_to_task.at(start_point));
    }
    time_step_data.component_id_to_task.emplace(kTimeStepStart, start_task);

    auto end_task = createLocalEndTask(time_step_index, timeStepEndTaskName(time_step_index));
    for (const auto &end_point : component_end_points_) {
        end_task.succeed(time_step_data.component_id_to_task.at(end_point));
    }
    time_step_data.component_id_to_task.emplace(kTimeStepEnd, end_task);

}

void TaskFlowTimeDomain::createTask(const int time_step_index, TimeStepData &time_step_data,
                                    const std::pair<component::ComponentGraph::PatternPtr,
                                                    component::ComponentGraph::ComponentPtr> &component) {
    auto instance_id = component.first->instance_id;

    auto task = taskflow_.placeholder();

    task.name(getTaskName(time_step_index, instance_id));

    auto local_result = time_step_data.component_data.find(instance_id);
    if (local_result == time_step_data.component_data.end()) {
        auto error_message = fmt::format("no source data for instance {0}", instance_id);
        SPDLOG_ERROR(error_message);
        throw std::range_error(error_message);
    }

    switch (component.second->getComponentType()) {
        case component::ComponentType::ASYNC_SOURCE:
        case component::ComponentType::INTERNAL_SYNC_SOURCE: {

            task.work([&local_data = local_result->second]() {
                taskSource(local_data);
            });
            break;
        }
        case component::ComponentType::SYNC_SOURCE:
        case component::ComponentType::ASYNC_SINK:
        case component::ComponentType::SYNC_FUNCTIONAL:
        case component::ComponentType::ASYNC_FUNCTIONAL:
        case component::ComponentType::SYNC_SINK: {
            task.work([&local_data = local_result->second]() {
                taskGenericComponent(local_data);
            });
            break;
        }
        case component::ComponentType::INVALID:
        default:SPDLOG_ERROR("Unsupported ComponentType {0} for component {1}",
                             component.second->getComponentType(),
                             component.second->getName());
            break;
    }

    time_step_data.component_id_to_task.emplace(instance_id, task);
}

std::string TaskFlowTimeDomain::getTaskName(const int time_step_index,
                                            const std::string &instance_id) const {
    return fmt::format("{0}_task{1}",
                       instance_id,
                       time_step_index);
}

void TaskFlowTimeDomain::prepare() {
    prepareComponents();
    prepareTaskData();
}

void TaskFlowTimeDomain::prepareComponents() {

    for (const auto &component : components_) {

        std::shared_ptr<pattern::instance::PatternInstance> pattern_instance = component.first;
        auto instance_id = component.first->instance_id;
        if (!pattern_instance) {
            spdlog::warn("skipping non dataflow pattern : {0}", instance_id);
            continue;
        }

        std::set<std::string> successors;

        bool is_endpoint = true;

        for (const auto *port : pattern_instance->getProducerPorts()) {
            for (const auto *input_port : port->connectedToPtr()) {
                successors.emplace(input_port->getID().first);
                is_endpoint = false;

            }
        }

        if (is_endpoint) {
            component_end_points_.emplace(instance_id);
        }
        if (pattern_instance->getConsumerPorts().empty()) {
            component_start_points_.emplace(instance_id);
        }

        component_to_successors_.emplace(instance_id, std::move(successors));

        switch (component.second->getComponentType()) {
            case component::ComponentType::ASYNC_SOURCE:
            case component::ComponentType::INTERNAL_SYNC_SOURCE: {

                auto component_index = time_domain_buffer_->getComponentIndex(instance_id);
                auto request_source_callback =
                    [this, component_index](auto &&timestamp) -> std::future<buffer::SourceComponentBuffer *> {
                        return requestSourceBuffer(std::forward<decltype(timestamp)>(timestamp),
                                                   component_index);
                    };
                component.second->setRequestCallback(request_source_callback);
                break;
            }
            case component::ComponentType::SYNC_SOURCE:
            case component::ComponentType::ASYNC_SINK:
            case component::ComponentType::SYNC_FUNCTIONAL:
            case component::ComponentType::ASYNC_FUNCTIONAL:
            case component::ComponentType::SYNC_SINK:
            case component::ComponentType::INVALID:
            default:

                break;
        }

    }
}

void TaskFlowTimeDomain::prepareTaskData() {

    for (int concurrent_index = 0; concurrent_index < time_step_count_; ++concurrent_index) {
        TimeStepData &time_step_data = time_step_data_[concurrent_index];

        for (const auto &component : components_) {

            std::shared_ptr<pattern::instance::PatternInstance> pattern_instance = component.first;
            if (!pattern_instance) {
                continue;
            }
            auto instance_id = component.first->instance_id;
            auto &time_step_buffer = time_domain_buffer_->getTimeStepBuffer(concurrent_index);
            auto component_index = time_step_buffer.getComponentIndex(instance_id);

            auto &component_buffer = time_step_buffer.getComponentBuffer(component_index);
            auto function_object = component.second;
            auto &parameter = component.first->local_pattern.parameter;
            time_step_data.component_data.emplace(instance_id,
                                                  ComponentData(time_step_buffer,
                                                                component_buffer,
                                                                *function_object,
                                                                component_index,
                                                                parameter,
                                                                running_));
        }
    }

    for (const auto &component : components_) {

        std::shared_ptr<pattern::instance::PatternInstance> pattern_instance = component.first;
        auto instance_id = component.first->instance_id;
        if (!pattern_instance) {
            continue;
        }

        for (int concurrent_index = 0; concurrent_index < time_step_count_; ++concurrent_index) {
            TimeStepData &time_step_data = time_step_data_[concurrent_index];
            auto &component_data = time_step_data.component_data.at(instance_id);
            for (const auto *port : pattern_instance->getConsumerPorts()) {
                auto input_id = port->connected_to.first;
                auto &input_data = time_step_data.component_data.at(input_id);
                component_data.valid_input.push_back(&input_data.valid_output);

            }
        }
    }

}

void TaskFlowTimeDomain::createBuffer() {

    time_domain_config_ = component_graph_->getTimeDomainConfig(time_domain_);
    components_ = component_graph_->getPatternsForTimeDomain(time_domain_);
    time_step_count_ = time_domain_config_.ringbuffer_size;
    time_step_latest_ = -1;
    //auto component_count = component_graph_->getPatternsForTimeDomain(time_domain_).size();
    time_domain_buffer_->init(*component_graph_);
    time_step_data_.resize(time_step_count_);
    taskflow_execute_future_.resize(time_step_count_);
    running_taskflows_.resize(time_step_count_, false);
    running_timestamps_.resize(time_step_count_);
    std::vector<bool> tmp;
    tmp.resize(time_step_count_, false);
    source_set_input_.resize(time_domain_buffer_->getCountSources(), tmp);
    latest_scheduled_ts_ = Timestamp::min();
}

void TaskFlowTimeDomain::masterSourceFinished() {
    // first finished call ends playback
    if (source_finished_.test_and_set()) {
        return;
    }

    source_finished_callback_();
}

std::future<buffer::SourceComponentBuffer *>
TaskFlowTimeDomain::requestSourceBuffer(Timestamp timestamp, int component_index) {

    SPDLOG_TRACE("RequestSource comp: {0} timestamp: {1} start", component_index, timestamp);

    // cancel potential older
    //cancelOlderEvents(timestamp, component_index);

    auto running_time_step = isTimestampRunning(timestamp);
    if (running_time_step < 0) {
        if (timestamp + time_domain_config_.max_offset < latest_scheduled_ts_) {
            SPDLOG_ERROR("New data timestamp must be monotonic increasing, latest: {0} now: {1}",
                         latest_scheduled_ts_,
                         timestamp);
            std::promise<buffer::SourceComponentBuffer *> value;
            value.set_value(nullptr);
            return value.get_future();

        }

        scheduleEvent(EventType::DATA, timestamp);

    } else {
        SPDLOG_TRACE("RequestSource comp: {0} timestamp: {1} already running",
                     component_index,
                     timestamp);
        cancelOlderEvents(timestamp, component_index);
    }

    return std::async(std::launch::deferred,
                      [&, timestamp, component_index]() mutable -> buffer::SourceComponentBuffer * {
                          latest_running_ts_.wait(timestamp);

                          auto taskflow_index = isTimestampRunning(timestamp);
                          if (taskflow_index < 0) {
                              SPDLOG_WARN("source component index {0} timestamp {1}, rejected value");
                              return nullptr;
                          } else {
                              source_set_input_[component_index][taskflow_index] = true;
                              SPDLOG_TRACE("source component future, taskflow {0} component {1} timestamp {2}",
                                           taskflow_index,
                                           component_index,
                                           timestamp);
                              return time_domain_buffer_->getTimeStepBuffer(taskflow_index).getSourceComponentBuffer(
                                  component_index);
                          }
                      });

}

void TaskFlowTimeDomain::freeTimeStep(int time_step_index) {

    std::unique_lock flow_guard(flow_mutex_);
    SPDLOG_TRACE("Free Time Step {0}, Reset Locks of ts: {1}",
                 time_step_index,
                 time_domain_buffer_->getTimeStepBuffer(time_step_index).getTimestamp());
    time_domain_buffer_->getTimeStepBuffer(time_step_index).resetLock();
    running_taskflows_[time_step_index] = false;
    if (queued_messages_.empty()) {
        setTaskflowFree(time_step_index);
    } else {
        SPDLOG_TRACE("Free Time Step {0}, run from queue, queued messages {1}", time_step_index, queued_messages_.size());
        runTaskFlowFromQueue();
    }

}

void TaskFlowTimeDomain::runTaskFlowFromQueue() {

    auto next_ts_message = queued_messages_.front();
    queued_messages_.pop();

    time_step_latest_++;
    time_step_latest_ = time_step_latest_ % time_step_count_;

    SPDLOG_INFO("Run task from queue, using time step: {0} ts: {1} {2}",
                time_step_latest_,
                next_ts_message.first,
                next_ts_message.second);
    assert(!running_taskflows_[time_step_latest_]);

    takeTaskflow(time_step_latest_, next_ts_message.first);
    auto &time_step_buffer = time_domain_buffer_->getTimeStepBuffer(time_step_latest_);
    time_step_buffer.setEvent(next_ts_message.first, next_ts_message.second);

    if (next_ts_message.second == EventType::DATA) {
        for (int source_index = 0; source_index < time_domain_buffer_->getCountSources(); ++source_index) {
            source_set_input_[source_index][time_step_latest_] = false;
        }
        latest_running_ts_.notifyAll(next_ts_message.first);
    } else {
        for (int source_index = 0; source_index < time_domain_buffer_->getCountSources(); ++source_index) {
            source_set_input_[source_index][time_step_latest_] = true;
            time_step_buffer.getSourceComponentBuffer(source_index)->commit(true);
        }

    }

}

int TaskFlowTimeDomain::isTimestampRunning(const Timestamp &timestamp) {
    std::unique_lock guard_running(running_mutex_);
    auto ts_min = timestamp - time_domain_config_.max_offset;
    auto ts_max = timestamp + time_domain_config_.max_offset;
    SPDLOG_TRACE("Search for {0} min {1} max {2}",
                 timestamp,
                 ts_min,
                 ts_max);
    for (size_t index = 0; index < running_timestamps_.size(); ++index) {
        auto latest_min = running_timestamps_[index] - time_domain_config_.max_offset;
        auto latest_max = running_timestamps_[index] + time_domain_config_.max_offset;
        if (latest_max >= ts_min && latest_min <= ts_max) {
            SPDLOG_TRACE("found match for ts {0} {1} {2}",
                         timestamp,
                         running_timestamps_[index],
                         index);
            return index;
        }

    }
    return -1;
}

void TaskFlowTimeDomain::setTaskflowFree(int time_step_index) {
    {
        std::unique_lock guard_running(running_mutex_);
        running_taskflows_[time_step_index] = false;
        running_timestamps_[time_step_index] = Timestamp::min();
    }
    free_taskflows_semaphore_.notify();
}
void TaskFlowTimeDomain::takeTaskflow(int taskflow_id,
                                      std::chrono::time_point<std::chrono::system_clock, TimeDuration> &next_ts) {
    std::unique_lock guard_running(running_mutex_);
    running_taskflows_[taskflow_id] = true;
    running_timestamps_[taskflow_id] = next_ts;
}

void TaskFlowTimeDomain::scheduleEvent(EventType message_type, Timestamp timestamp) {

    SPDLOG_TRACE("Schedule event ts: {0} {1}", timestamp, message_type);
    std::unique_lock guard_flow(flow_mutex_);

    latest_scheduled_ts_ = timestamp;

    queued_messages_.emplace(std::make_pair(timestamp, message_type));

    auto min_ts = timestamp - time_domain_config_.max_offset;
    for (int time_step_index = 0; time_step_index < time_step_count_; ++time_step_index) {
        for (int component_index = 0; component_index < time_domain_buffer_->getCountAsyncSources();
             ++component_index) {
            if (!source_set_input_[component_index][time_step_index] && running_taskflows_[time_step_index]
                && running_timestamps_[time_step_index] < min_ts) {
                auto &time_step_buffer = time_domain_buffer_->getTimeStepBuffer(time_step_index);
                SPDLOG_WARN(
                    "cancel source component buffer, because event seems to be missing, time step: {0} component {1} ts: {2}",
                    time_step_index,
                    component_index,
                    time_step_buffer.getTimestamp());
                time_step_buffer.getSourceComponentBuffer(component_index)->cancel();
            }
        }

    }

    if (free_taskflows_semaphore_.try_wait()) {
        SPDLOG_TRACE("Schedule event ts: {0} {1}, run from queue",
                     timestamp, message_type);
        runTaskFlowFromQueue();
    }

}

void TaskFlowTimeDomain::cancelOlderEvents(Timestamp timestamp, int component_index) {
    //executor_.silent_async([&, timestamp, component_index]() {
    std::unique_lock guard_flow(flow_mutex_);
    auto min_ts = timestamp - time_domain_config_.max_offset;
    for (int time_step_index = 0; time_step_index < time_step_count_; ++time_step_index) {
        if (!source_set_input_[component_index][time_step_index] && running_taskflows_[time_step_index]
            && running_timestamps_[time_step_index] + time_domain_config_.max_offset < min_ts) {
            auto &time_step_buffer = time_domain_buffer_->getTimeStepBuffer(time_step_index);
            SPDLOG_WARN(
                "cancel source component buffer, because a newer timestamp is requested, time step: {0} component {1} buffer timestamp: {2} current timestamp {3}",
                time_step_index,
                component_index,
                time_step_buffer.getTimestamp(),
                timestamp);
            time_step_buffer.getSourceComponentBuffer(component_index)->cancel();
        }
    }
    //});
}

void TaskFlowTimeDomain::stop() {

    if (stop_called_) {
        return;
    }
    stop_called_ = true;
    SPDLOG_TRACE("Call to stop dataflow");
    scheduleEvent(EventType::STOP, Timestamp::min());
    while (!stop_finished_.Wait()) {
        SPDLOG_INFO("waiting for stop message to finish: graph {0} time domain {1}",
                    component_graph_->getName(),
                    time_domain_);
    }
    scheduleEvent(EventType::TEARDOWN, Timestamp::min());
    while (!teardown_finished_.Wait()) {
        SPDLOG_INFO("waiting for teardown message to finish: graph {0} time domain {1}",
                    component_graph_->getName(),
                    time_domain_);
    }

    //scheduleEvent(EventType::DATAFLOW_STOP, Timestamp::min());
    //scheduleEvent(MessageType::DataflowNoOp, Timestamp::min());
    // stop event for every time step in the taskflow
    for (int time_step_index = 0; time_step_index < time_step_count_; ++time_step_index) {
        scheduleEvent(EventType::DATAFLOW_STOP, Timestamp::min());
    }


    //taskflow_future_.cancel();
    //time_domain_buffer_->cancelAll();

    auto status = taskflow_future_.wait_for(kDataflowStopTimeout);
    if (status != std::future_status::ready) {
        SPDLOG_WARN("could not stop task flow time domain {0} {1}", component_graph_->getName(), time_domain_);
    }
    running_ = false;

}

void TaskFlowTimeDomain::start() {

    running_ = true;
    taskflow_future_ = executor_.run(taskflow_);
    if (!taskflow_started_.wait()) {
        SPDLOG_ERROR("could not start taskflow");
        return;
    }
    scheduleEvent(EventType::CONFIGURE, Timestamp::min());
    while (!configure_finished_.Wait()) {
        SPDLOG_INFO("waiting for configure message to finish: graph {0} time domain {1}",
                    component_graph_->getName(),
                    time_domain_);
    }

    scheduleEvent(EventType::START, Timestamp::min());
    while (!start_finished_.Wait()) {
        SPDLOG_INFO("waiting for start message to finish: graph {0} time domain {1}",
                    component_graph_->getName(),
                    time_domain_);
    }

}

void TaskFlowTimeDomain::createInterTimeStepDependencies() {

    auto start_task = taskflow_.placeholder().name("start").work([&]() {
        globalTaskflowStart();
    });

    auto end_task = taskflow_.placeholder().name("end").work([&]() {
        globalTaskflowEnd();
    });

    auto start_entry = taskflow_.emplace([&]() -> tf::SmallVector<int, kStartEntries> {
        if (running_) {
            SPDLOG_TRACE("start entry is running");
            return start_entries_;
        } else {
            SPDLOG_TRACE("start entry exited");
            return {0};
        }
    }).name("start_entry");
    start_entry.succeed(start_task);
    start_entry.precede(end_task);

    auto connect_time_step_end_to_start = [&](int time_step_index) {
        auto &id_tasks = time_step_data_[time_step_index].component_id_to_task;
        auto seam_entry_name = fmt::format(kSeamEntryFormat, "TIME_STEP", time_step_index);
        auto seam_start_name = fmt::format(kSeamStartFormat, "TIME_STEP", time_step_index);
        auto seam_end_name = fmt::format(kSeamEndFormat, "TIME_STEP", time_step_index);

        auto seam_entry = createSeamEntryTask(time_step_index, seam_entry_name);
        auto seam_start = taskflow_.emplace([]() {}).name(seam_start_name);
        auto seam_end = taskflow_.emplace([]() {}).name(seam_end_name);

        auto &time_step_start = id_tasks.at(kTimeStepStart);
        auto &time_step_end = id_tasks.at(kTimeStepEnd);
        time_step_end.precede(seam_entry);
        seam_entry.precede(seam_start, seam_end);
        seam_start.precede(time_step_start);

        start_entry.precede(seam_entry);
        start_entries_.push_back(start_entries_.size() + 1);
    };

    auto inter_connect_time_steps = [&](const std::string &instance_id) {

        auto seam_entry_name = fmt::format(kSeamEntryFormat, instance_id, 0);
        auto seam_start_name = fmt::format(kSeamStartFormat, instance_id, 0);
        auto seam_end_name = fmt::format(kSeamEndFormat, instance_id, 0);

        auto seam_entry = createSeamEntryTask(0, seam_entry_name);
        auto seam_start = taskflow_.emplace([]() {}).name(seam_start_name);
        auto seam_end = taskflow_.emplace([]() {}).name(seam_end_name);


        for (int time_step_index = 0; time_step_index < time_step_count_ - 1; ++time_step_index) {
            auto &id_tasks = time_step_data_[time_step_index].component_id_to_task;
            auto &next_id_tasks = time_step_data_[time_step_index + 1].component_id_to_task;
            id_tasks.at(instance_id).precede(next_id_tasks.at(instance_id));
        }

        auto &first_tasks = time_step_data_.front().component_id_to_task;
        auto &last_tasks = time_step_data_.back().component_id_to_task;

        auto &first_time_step = first_tasks.at(instance_id);
        auto &last_time_step = last_tasks.at(instance_id);
        last_time_step.precede(seam_entry);
        seam_entry.precede(seam_start, seam_end);
        seam_start.precede(first_time_step);

        start_entry.precede(seam_entry);
        start_entries_.push_back(start_entries_.size() + 1);
    };

    auto add_start_task = [&](const std::string &instance_id) {
        auto &first_tasks = time_step_data_.front().component_id_to_task;
        auto seam_entry_name = fmt::format(kSeamEntryFormat, instance_id, "");
        start_entry.precede(first_tasks.at(seam_entry_name));
        start_entries_.push_back(start_entries_.size() + 1);
    };

    for (int time_step_index = 0; time_step_index < time_step_count_; ++time_step_index) {
        connect_time_step_end_to_start(time_step_index);

    }

    inter_connect_time_steps(kTimeStepStart);
    inter_connect_time_steps(kTimeStepEnd);

    for (const auto &component : components_) {
        std::shared_ptr<pattern::instance::PatternInstance> pattern_instance = component.first;
        if (!pattern_instance) {
            continue;
        }

        if (pattern_instance->getConcurrency() == Concurrency::SERIAL) {
            inter_connect_time_steps(pattern_instance->instance_id);
        }

    }
}

void TaskFlowTimeDomain::globalTaskflowStart() {
    SPDLOG_TRACE("start time domain: {0}", time_domain_);
    for (int time_step_index = 0; time_step_index < time_step_count_; ++time_step_index) {
        time_domain_buffer_->getTimeStepBuffer(time_step_index).resetLock();
    }
    taskflow_started_.notify();

}

void TaskFlowTimeDomain::globalTaskflowEnd() {
    SPDLOG_TRACE("End time domain {0}", time_domain_);
}

tf::Task TaskFlowTimeDomain::createLocalStartTask(int time_step_index, const std::string &name) {
    return taskflow_.emplace([&, time_step_index]() mutable {
        SPDLOG_TRACE("TIME_STEP_START_{0}", time_step_index);

    }).name(name);
}

tf::Task TaskFlowTimeDomain::createLocalEndTask(int time_step_index, const std::string &name) {
    return taskflow_.emplace([&, time_step_index]() mutable {
        SPDLOG_TRACE("TIME_STEP_END_{0}", time_step_index);
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
            default:break;
        }

    }).name(name);
}

tf::Task TaskFlowTimeDomain::createSeamEntryTask(int time_step_index, const std::string &seam_entry_name) {
    return taskflow_.emplace([&, time_step_index, seam_entry_name]() {
        SPDLOG_TRACE("seam entry: {0}", seam_entry_name);
        if (time_domain_buffer_->getTimeStepBuffer(time_step_index).getEventType() == EventType::DATAFLOW_STOP) {
            SPDLOG_TRACE("Dataflow stop in seam entry: {0}", seam_entry_name);
            return 1;
        } else {
            return 0;
        }
    }).name(seam_entry_name);
}

}