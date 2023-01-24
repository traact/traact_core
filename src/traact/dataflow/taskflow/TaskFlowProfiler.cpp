/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TaskFlowProfiler.h"

#include <utility>

namespace traact::dataflow {

void TaskFlowProfiler::set_up(size_t num_workers) {
    //time_domain_state_->init(num_workers, time_domain_state_->getTimeStepCount());
}
void TaskFlowProfiler::on_entry(tf::WorkerView worker_view, tf::TaskView task_view) {
    auto task_index = task_hash_to_index_[task_view.hash_value()];
    time_domain_state_->taskStart(task_index, worker_view.id());
}
void TaskFlowProfiler::on_exit(tf::WorkerView worker_view, tf::TaskView task_view) {
    auto task_index = task_hash_to_index_[task_view.hash_value()];
    time_domain_state_->taskEnd(task_index, worker_view.id());
}
TaskFlowProfiler::TaskFlowProfiler(TimeDomainState::SharedPtr time_domain_state, const tf::Taskflow& taskflow) : time_domain_state_(std::move(
    time_domain_state)) {

    taskflow.for_each_task([this](tf::Task task){
        addTask(task);
    });
}
void TaskFlowProfiler::addTask(const tf::Task &task) {

    std::vector<std::string> predecessors;
    task.for_each_dependent([&predecessors](tf::Task predecessor) {
        predecessors.emplace_back(predecessor.name());
    });

    auto task_index = time_domain_state_->addTask(task.name(), predecessors);
    task_hash_to_index_[task.hash_value()] = task_index;
}
} // dataflow