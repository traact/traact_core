/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TaskFlowTimeDomain.h"
#include <utility>

std::string timeStepStartTaskName(int index) {
    return fmt::format("TIME_STEP_START_{0}", index);
}
std::string timeStepEndTaskName(int index) {
    return fmt::format("TIME_STEP_END_{0}", index);
}

static const constexpr char *kTimeStepStart{"TIME_STEP_START"};
static const constexpr char *kTimeStepEnd{"TIME_STEP_END"};
static const constexpr char *kSeamEntryFormat{"{0}_SEAM_{1}"};
static const constexpr char *kSeamStartFormat{"{0}_SEAM_START_{1}"};
static const constexpr char *kSeamEndFormat{"{0}_SEAM_END_{1}"};

static const constexpr char *kModuleStart{"MODULE_START_{0}_{1}"};
static const constexpr char *kModuleMiddle{"MODULE_MIDDLE_{0}_{1}"};
static const constexpr char *kModuleEnd{"MODULE_END_{0}_{1}"};

static const constexpr char *kModuleStartName{"MODULE_START_{0}"};
static const constexpr char *kModuleMiddleName{"MODULE_MIDDLE_{0}"};
static const constexpr char *kModuleEndName{"MODULE_END_{0}"};

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
        createModuleConstraintTasks(time_step_index);
    }

    createInterTimeStepDependencies();

    SPDLOG_TRACE("dump of task flow \n{0}", taskflow_.dump());

    for (int time_step_index = 0; time_step_index < time_step_count_; ++time_step_index) {
        time_domain_buffer_->getTimeStepBuffer(time_step_index).setEvent(kTimestampZero, EventType::INVALID);
    }

    scheduler_ = std::make_unique<DefaultScheduler>(time_domain_config_,
                                                    time_domain_buffer_,
                                                    component_graph_->getName(),
                                                    time_domain_,
                                                    &taskflow_, [&]() {
            printState();
        });
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

void TaskFlowTimeDomain::createModuleConstraintTasks(int time_step_index) {
    auto &time_step_data = time_step_data_[time_step_index];

    for (const auto &[module_key, components] : component_modules_) {
        auto module_instance = component_graph_->getModule(module_key);
        bool use_middle = !components.begin_components.empty() && !components.end_components.empty();

        std::string start_task_name = fmt::format(kModuleStart, module_key, time_step_index);
        std::string middle_task_name = fmt::format(kModuleMiddle, module_key, time_step_index);
        std::string end_task_name = fmt::format(kModuleEnd, module_key, time_step_index);
        auto start_task = taskflow_.emplace([]() {}).name(start_task_name);
        time_step_data.component_id_to_task.emplace(fmt::format(kModuleStartName, module_key), start_task);
        tf::Task end_task;

        tf::Task begin_components_end_task;
        tf::Task end_components_start_task;

        if (use_middle) {
            end_task = taskflow_.emplace([]() {}).name(end_task_name);
            auto middle_task = taskflow_.emplace([module_instance]() {
                module_instance->processTimePoint();
            }).name(middle_task_name);

            time_step_data.component_id_to_task.emplace(fmt::format(kModuleEndName, module_key), end_task);
            time_step_data.component_id_to_task.emplace(fmt::format(kModuleMiddleName, module_key), middle_task);

            begin_components_end_task = middle_task;
            end_components_start_task = middle_task;

        } else {
            end_task = taskflow_.emplace([module_instance]() {
                module_instance->processTimePoint();
            }).name(end_task_name);
            time_step_data.component_id_to_task.emplace(fmt::format(kModuleEndName, module_key), end_task);

            begin_components_end_task = end_task;
            end_components_start_task = start_task;

        }
        for (const auto &instance_id : components.begin_components) {
            start_task.precede(time_step_data.component_id_to_task.at(instance_id));
            begin_components_end_task.succeed(time_step_data.component_id_to_task.at(instance_id));
            for (const auto &successor : component_to_successors_[instance_id]) {
                begin_components_end_task.precede(time_step_data.component_id_to_task.at(successor));
            }
        }
        for (const auto &instance_id : components.end_components) {
            end_components_start_task.precede(time_step_data.component_id_to_task.at(instance_id));
            end_task.succeed(time_step_data.component_id_to_task.at(instance_id));
            for (const auto &successor : component_to_successors_[instance_id]) {
                end_task.precede(time_step_data.component_id_to_task.at(successor));
            }
        }

    }
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

        for (const auto *port : pattern_instance->getProducerPorts(time_domain_)) {
            for (const auto *input_port : port->connectedToPtr()) {
                successors.emplace(input_port->getId().first);
                is_endpoint = false;

            }
        }

        if (is_endpoint) {
            component_end_points_.emplace(instance_id);
        }
        if (pattern_instance->getConsumerPorts(time_domain_).empty()
            && pattern_instance->getComponentType(time_domain_) != component::ComponentType::SYNC_SOURCE) {
            component_start_points_.emplace(instance_id);
        }

        component_to_successors_.emplace(instance_id, std::move(successors));

        component.second->setSourceFinishedCallback([&]() {
            masterSourceFinished();
        });

        switch (component.first->getComponentType(time_domain_)) {
            case component::ComponentType::ASYNC_SOURCE:
            case component::ComponentType::INTERNAL_SYNC_SOURCE: {

                auto component_index = time_domain_buffer_->getComponentIndex(instance_id);
                auto request_source_callback =
                    [this, component_index](Timestamp timestamp) -> std::future<buffer::SourceComponentBuffer *> {
                        return requestSourceBuffer(timestamp, component_index);
                    };
                component.second->setRequestCallback(request_source_callback);
                break;
            }
            case component::ComponentType::SYNC_SOURCE: {
                std::optional<std::string> sync_source_start = findSyncSourceStartPoint(component, 0);
                if (!sync_source_start.has_value()) {
                    auto error = fmt::format(
                        "sync source {0} has no start point (other non sync source, there should be at least one async source)",
                        pattern_instance->instance_id);
                    SPDLOG_ERROR(error);
                    throw std::invalid_argument(error);
                }
                component_to_successors_[sync_source_start.value()].emplace(pattern_instance->instance_id);
                break;
            }
            case component::ComponentType::ASYNC_SINK:
            case component::ComponentType::SYNC_FUNCTIONAL:
            case component::ComponentType::ASYNC_FUNCTIONAL:
            case component::ComponentType::SYNC_SINK:
            case component::ComponentType::INVALID:
            default:break;
        }

        auto module_comp_tmp =
            std::dynamic_pointer_cast<component::ModuleComponent, component::Component>(component.second);

        if (module_comp_tmp) {
            std::string module_key = module_comp_tmp->getModuleKey();

            switch (component.first->getComponentType(time_domain_)) {
                case component::ComponentType::ASYNC_SINK:
                case component::ComponentType::SYNC_SINK:
                case component::ComponentType::ASYNC_SOURCE:
                case component::ComponentType::INTERNAL_SYNC_SOURCE:
                case component::ComponentType::SYNC_FUNCTIONAL: {
                    component_modules_[module_key].begin_components.emplace_back(instance_id);
                    break;
                }
                case component::ComponentType::SYNC_SOURCE: {
                    component_modules_[module_key].end_components.emplace_back(instance_id);
                    break;
                }
                case component::ComponentType::ASYNC_FUNCTIONAL:
                case component::ComponentType::INVALID:
                default:throw std::invalid_argument(fmt::format("invalid component type of {0}", instance_id));
                    break;
            }
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
            auto &function_object = component.second;

            time_step_data.component_data.emplace(instance_id,
                                                  ComponentData(time_step_buffer, component_buffer, *function_object,
                                                                component_index, *pattern_instance, running_));
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
            for (const auto *port : pattern_instance->getConsumerPorts(time_domain_)) {
                auto input_id = port->connected_to.first;
                if (input_id.empty()) {
                    throw std::invalid_argument(fmt::format("component input not connected {0} {1}",
                                                            port->getId().first,
                                                            port->getId().second));
                }
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

    auto connect_end_to_next_start = [&](std::string start_name, std::string end_name) {

        auto seam_entry_name = fmt::format(kSeamEntryFormat, start_name, 0);
        auto seam_start_name = fmt::format(kSeamStartFormat, start_name, 0);
        auto seam_end_name = fmt::format(kSeamEndFormat, start_name, 0);

        auto seam_entry = createSeamEntryTask(0, seam_entry_name);
        auto seam_start = taskflow_.emplace([]() {}).name(seam_start_name);
        auto seam_end = taskflow_.emplace([]() {}).name(seam_end_name);

        for (int current_time_step_index = 0; current_time_step_index < time_step_count_ - 1;
             ++current_time_step_index) {
            int next_time_step_index = current_time_step_index + 1;
            auto &current_end_tasks = time_step_data_[current_time_step_index].component_id_to_task.at(end_name);
            auto &next_start_tasks = time_step_data_[next_time_step_index].component_id_to_task.at(start_name);
            current_end_tasks.precede(next_start_tasks);

            auto time_step_end_task = time_step_data_[current_time_step_index].component_id_to_task.at(kTimeStepEnd);
            current_end_tasks.precede(time_step_end_task);
        }

        auto &last_end_task = time_step_data_.back().component_id_to_task.at(end_name);
        auto &fist_start_task = time_step_data_.front().component_id_to_task.at(start_name);
        auto last_time_step_end_task = time_step_data_.back().component_id_to_task.at(kTimeStepEnd);

        last_end_task.precede(seam_entry);
        seam_entry.precede(seam_start, seam_end);
        seam_start.precede(fist_start_task);

        last_end_task.precede(last_time_step_end_task);

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

        if (pattern_instance->getConcurrency(time_domain_) == Concurrency::SERIAL) {
            inter_connect_time_steps(pattern_instance->instance_id);
        }

    }

    for (const auto &[module_key, module_components] : component_modules_) {
        auto module_start_task = fmt::format(kModuleStartName, module_key);
        auto module_end_task = fmt::format(kModuleEndName, module_key);
        connect_end_to_next_start(module_start_task, module_end_task);
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
std::optional<std::string> TaskFlowTimeDomain::findSyncSourceStartPoint(const std::pair<component::ComponentGraph::PatternPtr,
                                                                                        component::ComponentGraph::ComponentPtr> &pair,
                                                                        int time_step) {
    // find the latest point in dataflow where the sync source needs to be triggered to allow maximum wait for
    // time domain syncing components
    // for now use first async source
    for (const auto &pattern : component_graph_->getPatternsForTimeDomain(time_domain_)) {
        if (pattern.first->getComponentType(time_domain_) == component::ComponentType::ASYNC_SOURCE) {
            return pattern.first->instance_id;
        }
    }
    return {};
}
void TaskFlowTimeDomain::printState() {
    std::stringstream ss;
    ss << "State of dataflow network\n";
    int time_step_buffer = 0;
    for (const auto &data : time_step_data_) {
        ss << "time step buffer " << time_step_buffer << ":\n";
        for (const auto &component_data : data.component_data) {
            ss << fmt::format("{0} last call: {1} position: {2} event type: {3} error: {4}\n",
                              component_data.first,
                              component_data.second.state_last_call,
                              component_data.second.state_last_event_position,
                              component_data.second.state_last_event_type,
                              component_data.second.state_had_error);
        }
        ++time_step_buffer;
    }

    SPDLOG_WARN(ss.str());
}

}