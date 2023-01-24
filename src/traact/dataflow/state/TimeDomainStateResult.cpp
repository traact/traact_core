/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TimeDomainStateResult.h"

#include <utility>
#include <cmath>
#include <spdlog/spdlog.h>
#include <re2/re2.h>
namespace traact::dataflow {

static constexpr const size_t kMaxResultBuffer{50};
static constexpr const TimeDuration kOldestData{std::chrono::seconds(5)};

TimeDomainStateProcessing::TimeDomainStateProcessing(std::shared_ptr<TimeDomainState> time_domain_state)
    : time_domain_state_(std::move(
    time_domain_state)), profile_result_(time_domain_state_->getTaskCount()) {

    re2::RE2 re_exp("([^_]+)_(\\d+)_(.+)");
    const auto &task_info = time_domain_state_->getTaskInfo();
    auto find_task_index = [&task_info](const std::string &task_id) -> int {
        auto result = std::find_if(task_info.cbegin(), task_info.cend(), [&task_id](const auto &item) {
            return item.task_id == task_id;
        });
        if (result == task_info.cend()) {
            return -1;
        } else {
            return std::distance(task_info.begin(), result);
        }
    };

    time_step_to_tasks_.resize(time_domain_state_->getTimeStepCount());
    start_task_index_.resize(time_domain_state_->getTimeStepCount());
    end_task_index_.resize(time_domain_state_->getTimeStepCount());
    std::map<std::string, std::vector<std::shared_ptr<ProfileTimeResult>>> task_to_time_steps;


    for (int task_index = 0; task_index < task_info.size(); ++task_index) {
        const auto &task = task_info[task_index];
        const auto &task_id = task.task_id;

        std::string task_type_name;
        int time_step_index{0};
        std::string instance_id;
        if(re2::RE2::FullMatch(task_id, re_exp, &task_type_name, &time_step_index, &instance_id)) {
            std::vector<size_t> predecessors;
            for (const auto &pred : task.predecessors) {
                auto pred_index = find_task_index(pred);
                if (pred_index >= 0) {
                    predecessors.emplace_back(pred_index);
                } else {
                    SPDLOG_ERROR("task {0}: predecessor {1} not found", task_id, pred);
                }
            }

            time_step_to_tasks_[time_step_index].emplace_back(task_index);

            auto type_instance_id = fmt::format("{0}_{1}", task_type_name, instance_id);

            auto& task_to_time_step = task_to_time_steps[type_instance_id];
            if(task_to_time_step.empty()){
                task_to_time_step.reserve(time_domain_state_->getTimeStepCount());
                // init with new ProfileTimeResult and override with the ones found in the loop
                // not nice but easy, control and seam tasks only exist in time step 0
                // they could be the only ProfileTimeResult but the order of the other ProfileTimeResult
                // for each time step must be correct in the vector
                for (int i = 0; i < time_domain_state_->getTimeStepCount(); ++i) {
                    task_to_time_step.emplace_back(std::make_shared<ProfileTimeResult>());
                }
            }
            task_to_time_step[time_step_index] = profile_result_.tasks[task_index];
            auto task_type = task_util::taskTypeNameToEnum(task_type_name.c_str());

            info_.emplace_back(task_index, task_id,
                               task_type,
                               time_step_index,
                               instance_id,
                               predecessors);

            if(task_type == dataflow::task_util::TaskType::CONTROL_FLOW){
                if(instance_id == dataflow::task_util::kTimeStepStart){
                    start_task_index_[time_step_index] = task_index;
                } else if(instance_id == dataflow::task_util::kTimeStepEnd){
                    end_task_index_[time_step_index] = task_index;
                }

            }

        } else {
            SPDLOG_ERROR("parsing of task id not successful: {0}", task_id);
        }


    }

    for (const auto& id_tasks : task_to_time_steps) {
        profile_result_.time_step_tasks.emplace(id_tasks.first, std::make_shared<TimeStepProfileTimeResult>(id_tasks.second));
    }

    current_state_.init(time_domain_state_->getTimeStepCount(), time_domain_state_->getTaskCount());

}
void TimeDomainStateProcessing::update(TimeDuration max_offset) {

    auto profile_data = time_domain_state_->getData();
    std::map<TimestampSteady, std::deque<TaskStateData>> time_sorted_data;



    profile_result_.last_update = nowSteady();
    for (const auto &worker_data : profile_data->task_data) {
        for (const auto &data : worker_data) {
            time_sorted_data[data.event_time].emplace_back(data);
        }
    }



    auto isTimeStepStart = [&start_task_index = this->start_task_index_](size_t task_index) -> int{
        auto is_start = std::find(start_task_index.begin(), start_task_index.end(), task_index);
        if(is_start != start_task_index.end()){
            return std::distance(start_task_index.begin(), is_start);
        } else {
            return -1;
        }


    };

    for(const auto& time_data : time_sorted_data){
        for(const auto& data : time_data.second){
            auto time_step_start = isTimeStepStart(data.task_index);
            if(time_step_start >= 0 && data.is_start){
                resetTimeStep(time_step_start);
            }
            profile_result_.tasks[data.task_index]->addMeasurement(data);
        }
    }



    auto latest_valid = profile_result_.last_update - max_offset;
    for (auto &task : profile_result_.tasks) {
        task->update(latest_valid);
    }

    for(auto &task : profile_result_.time_step_tasks) {
        task.second->update(latest_valid);
    }

}
const std::vector<ProcessingTaskInfo> &TimeDomainStateProcessing::getInfo() {
    return info_;
}
const TimeDomainProfileResult &TimeDomainStateProcessing::getProfileResult() {
    return profile_result_;
}
const TimeDomainStateResult &TimeDomainStateProcessing::getCurrentState() const {
    return current_state_;
}
void TimeDomainStateProcessing::resetTimeStep(int time_step) {
    for(auto& task_index : time_step_to_tasks_[time_step]){
        profile_result_.tasks[task_index]->current_state = TimeDomainTaskState::IDLE;
    }

}
size_t TimeDomainStateProcessing::getTimeStepCount() const {
    return time_step_to_tasks_.size();
}

TimeDomainProfileResult::TimeDomainProfileResult(size_t task_count) {
    tasks.reserve(task_count);
    for (auto i = 0; i < task_count; ++i) {
        tasks.emplace_back(std::make_shared<ProfileTimeResult>());
    }
}
void TimeDomainStateResult::init(size_t time_step_count, size_t task_count) {
    task_finished.resize(time_step_count);
    for (auto &tasks : task_finished) {
        tasks.resize(task_count, TimeDomainTaskState::IDLE);
    }
}

ProcessingTaskInfo::ProcessingTaskInfo(size_t task_index,
                                       std::string task_id,
                                       task_util::TaskType task_type,
                                       size_t time_step,
                                       std::string pattern_instance_id,
                                       std::vector<size_t> predecessors)
    : task_index(task_index),
      task_id(std::move(task_id)),
      task_type(task_type),
      time_step(time_step),
      pattern_instance_id(std::move(pattern_instance_id)),
      predecessors(std::move(predecessors)) {}

void ProfileTimeResult::update(TimestampSteady latest_valid) {
    measurement_count = 0;
    TimeDuration mean = TimeDuration::zero();

    TimestampSteady global_start = TimestampSteady(TimeDuration(std::numeric_limits<int64_t>::max()));
    TimestampSteady global_end = kTimestampSteadyZero;

    for (const auto &time : measurements) {
        //if (time.start >= latest_valid) {
        mean += time.end - time.start;
            measurement_count++;
            global_start = std::min(global_start, time.start);
            global_end = std::max(global_end, time.end);
        //}

    }

    if (measurement_count == 0) {
        return;
    }

    mean_duration = mean / measurement_count;

    double std_dev_value{0};
    for (auto time : measurements) {
        //if (time.start >= latest_valid) {
            auto diff = static_cast<double>((mean_duration - (time.end - time.start)).count());
        std_dev_value += diff * diff;
        //}
    }

    float measurement_count_float =static_cast<float>(measurement_count);
    std_dev_value /= measurement_count_float;
    std_dev_value = std::sqrt(std_dev_value);
    std_dev = TimeDuration(static_cast<uint64_t>(std_dev_value));

    auto total_time = std::chrono::duration_cast<TimeDurationFloatSecond>(global_end - global_start);
    events_per_second = measurement_count_float / total_time.count();

}
void ProfileTimeResult::addMeasurement(const TaskStateData &measurement) {
    if (measurement.is_start) {
        if (current_state != TimeDomainTaskState::FINISHED && current_state != TimeDomainTaskState::IDLE) {
            SPDLOG_ERROR("start of task in profile data while the task was not finished or idle, something wrong, {0}",
                         current_state);
            current_state = TimeDomainTaskState::INVALID;
            return;
        }
        current_state = TimeDomainTaskState::STARTED;
        current_measurement.event = current_event++;
        current_measurement.start = measurement.event_time;
    } else {
        if (current_state != TimeDomainTaskState::STARTED) {
            SPDLOG_ERROR("end of task in profile data while the task was not started, something wrong, {0}",
                         current_state);
            current_state = TimeDomainTaskState::INVALID;
            return;
        }
        current_state = TimeDomainTaskState::FINISHED;
        current_measurement.end = measurement.event_time;
        measurements.push_back(current_measurement);
    }
    //measurements.emplace_back();
}
TimeStepProfileTimeResult::TimeStepProfileTimeResult(std::vector<std::shared_ptr<ProfileTimeResult>> time_steps)
    : time_steps(std::move(time_steps)) {}
void TimeStepProfileTimeResult::update(TimestampSteady latest_valid) {
    measurement_count = 0;
    TimeDuration mean = TimeDuration::zero();

    TimestampSteady global_start = TimestampSteady(TimeDuration(std::numeric_limits<int64_t>::max()));
    TimestampSteady global_end = kTimestampSteadyZero;

    current_event = 0;
    for(const auto& task : time_steps){
        current_event += task->current_event;
        for (const auto &time : task->measurements) {
            //if (time.start >= latest_valid) {
            mean += time.end - time.start;
            measurement_count++;
            global_start = std::min(global_start, time.start);
            global_end = std::max(global_end, time.end);

            //}

        }
    }


    if (measurement_count == 0) {
        return;
    }

    mean_duration = mean / measurement_count;

    double std_dev_value{0};
    for(const auto& task : time_steps) {
        for (auto time : task->measurements) {
            //if (time.start >= latest_valid) {
            auto diff = static_cast<double>((mean_duration - (time.end - time.start)).count());
            std_dev_value += diff * diff;
            //}
        }
    }

    float measurement_count_float =static_cast<float>(measurement_count);
    std_dev_value /= measurement_count_float;
    std_dev_value = std::sqrt(std_dev_value);
    std_dev = TimeDuration(static_cast<uint64_t>(std_dev_value));

    auto total_time = std::chrono::duration_cast<TimeDurationFloatSecond>(global_end - global_start);
    events_per_second = measurement_count_float / total_time.count();
}
} // dataflow