/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TimeDomainState.h"
#include <spdlog/spdlog.h>

#include <utility>
namespace traact::dataflow {

void TimeDomainState::init(size_t worker_count, size_t time_step_count) {
    worker_count_ = worker_count;
    time_step_count_ = time_step_count;
    std::vector<std::mutex> list(worker_count);
    data_mutex_.swap(list);

    buffer_data_ = std::make_shared<TimeDomainStateData>(worker_count_);
}

size_t TimeDomainState::addTask(const std::string &task_id, std::vector<std::string> predecessors) {

    if (task_id.empty()) {
        throw std::invalid_argument("task name of taskflow task is empty");
    }

    auto task_present = std::find_if(task_info_.begin(), task_info_.end(), [&task_id](const auto &item) {
        return item.task_id == task_id;
    });
    if (task_present == task_info_.end()) {
        size_t task_index = task_info_.size();
        task_info_.emplace_back(task_id, std::move(predecessors));
        return task_index;
    } else {
        SPDLOG_ERROR("task_id already present in TimeDomainState {0}", task_id);
        return std::distance(task_info_.begin(), task_present);
    }
}

TimeDomainState::TimeDomainStateDataPtr TimeDomainState::getData() {
    auto result_data = std::make_shared<TimeDomainStateData>(worker_count_);
    for (auto i = 0; i < data_mutex_.size(); ++i) {
        std::scoped_lock guard(data_mutex_[i]);
        result_data->task_data[i].swap(buffer_data_->task_data[i]);
    }
    return result_data;
}
const std::vector<TaskInfo> &TimeDomainState::getTaskInfo() const {
    return task_info_;
}
void TimeDomainState::taskStart(size_t task_index, size_t worker_id) {
    std::scoped_lock guard(data_mutex_[worker_id]);
    buffer_data_->task_data[worker_id].emplace_back(task_index, true);

}
void TimeDomainState::taskEnd(size_t task_index, size_t worker_id) {
    std::scoped_lock guard(data_mutex_[worker_id]);
    buffer_data_->task_data[worker_id].emplace_back(task_index, false);
}
size_t TimeDomainState::getTaskCount() const {
    return task_info_.size();
}
size_t TimeDomainState::getTimeStepCount() const {
    return time_step_count_;
}

TimeDomainStateData::TimeDomainStateData(size_t worker_count) : task_data(worker_count) {

}
TaskInfo::TaskInfo(std::string task_id, std::vector<std::string> predecessors)
    : task_id(std::move(task_id)), predecessors(std::move(predecessors)) {}

TaskStateData::TaskStateData(size_t index, bool is_start) : task_index(index), is_start(is_start), event_time(nowSteady()) {

}
} // traact