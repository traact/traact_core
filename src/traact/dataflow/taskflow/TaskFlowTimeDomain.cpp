/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TaskFlowTimeDomain.h"
#include <utility>

std::string timeStepStartTaskName(int index) {
    return fmt::format("TIME_STEP_START_{0}", index);
}
std::string timeStepEndTaskName(int index) {
    return fmt::format("TIME_STEP_END_{0}", index);
}

static const constexpr char* kTimeStepStart{"TIME_STEP_START"};
static const constexpr char* kTimeStepEnd{"TIME_STEP_END"};
static const constexpr char* kSeamEntryFormat{"{0}_SEAM_{1}"};
static const constexpr char* kSeamStartFormat{"{0}_SEAM_START_{1}"};
static const constexpr char* kSeamEndFormat{"{0}_SEAM_END_{1}"};

namespace traact::dataflow {

TaskFlowTimeDomain::TaskFlowTimeDomain(int time_domain,
                                       component::ComponentGraph::Ptr component_graph,
                                       buffer::DataBufferFactoryPtr buffer_factory,
                                       component::Component::SourceFinishedCallback callback)
    : generic_factory_objects_(std::move(buffer_factory)),
      source_finished_callback_(std::move(callback)),
      component_graph_(std::move(component_graph)),
      time_domain_(time_domain),
      time_domain_buffer_(std::make_shared<buffer::TimeDomainBuffer>(time_domain_, generic_factory_objects_)) {

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
        time_domain_buffer_->getTimeStepBuffer(time_step_index).setEvent(kTimestampZero, EventType::INVALID);
    }

    scheduler_ = std::make_unique<DefaultScheduler>(time_domain_config_, time_domain_buffer_, component_graph_->getName(), time_domain_, &taskflow_);
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

    switch (component.first->getComponentType(time_domain_)) {
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
                             component.first->getComponentType(time_domain_),
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
            SPDLOG_WARN("skipping non dataflow pattern : {0}", instance_id);
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

        switch (component.first->getComponentType(time_domain_)) {
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
                component_data.successors_valid.push_back(&input_data.valid_component_call);

            }
        }
    }

}

void TaskFlowTimeDomain::createBuffer() {

    time_domain_config_ = component_graph_->getTimeDomainConfig(time_domain_);
    components_ = component_graph_->getPatternsForTimeDomain(time_domain_);
    time_step_count_ = time_domain_config_.ringbuffer_size;
    //auto component_count = component_graph_->getPatternsForTimeDomain(time_domain_).size();
    time_domain_buffer_->init(*component_graph_);
    time_step_data_.resize(time_step_count_);


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



    return scheduler_->requestSourceBuffer(timestamp, component_index);

}


void TaskFlowTimeDomain::stop() {

    if (stop_called_) {
        return;
    }
    stop_called_ = true;
    scheduler_->stop();

    running_ = false;

}

void TaskFlowTimeDomain::start() {

    running_ = true;
    scheduler_->start();


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
    scheduler_->globalTaskFlowStart();

}

void TaskFlowTimeDomain::globalTaskflowEnd() {
    SPDLOG_TRACE("End time domain {0}", time_domain_);
}

tf::Task TaskFlowTimeDomain::createLocalStartTask(int time_step_index, const std::string &name) {
    return taskflow_.emplace([&, time_step_index]() mutable {
        SPDLOG_TRACE("TIME_STEP_START_{0}", time_step_index);
        scheduler_->timeStepStart(time_step_index);
    }).name(name);
}

tf::Task TaskFlowTimeDomain::createLocalEndTask(int time_step_index, const std::string &name) {
    return taskflow_.emplace([&, time_step_index]() mutable {
        SPDLOG_TRACE("TIME_STEP_END_{0}", time_step_index);
        scheduler_->timeStepEnded(time_step_index);


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