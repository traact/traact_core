/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_STATE_TIMEDOMAINSTATERESULT_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_STATE_TIMEDOMAINSTATERESULT_H_

#include <unordered_map>
#include <vector>
#include <string>
#include "traact/datatypes.h"
#include "DataflowState.h"
#include "traact/util/CircularBuffer.h"
#include "traact/dataflow/graph/task/TraactTaskId.h"
namespace traact::dataflow {

enum TimeDomainTaskState {
    IDLE = 0,
    STARTED,
    FINISHED,
    INVALID
};

struct ProfileTaskResult {
    TimestampSteady start;
    TimestampSteady end;
    size_t event{0};
};

struct ProfileTimeResult {
    TimeDuration mean_duration{0};
    TimeDuration std_dev{0};
    size_t measurement_count;
    TimeDomainTaskState current_state{TimeDomainTaskState::IDLE};
    size_t current_event;
    float events_per_second;
    util::CircularBuffer<ProfileTaskResult, 30> measurements;
    ProfileTaskResult current_measurement;

    void update(TimestampSteady latest_valid);
    void addMeasurement(const TaskStateData& measurement);
};

struct TimeStepProfileTimeResult {
    explicit TimeStepProfileTimeResult(std::vector<std::shared_ptr<ProfileTimeResult>> time_steps);
    TimeDuration mean_duration{0};
    TimeDuration std_dev{0};
    size_t measurement_count{0};
    TimeDomainTaskState current_state{TimeDomainTaskState::IDLE};
    size_t current_event{0};
    float events_per_second{0};
    std::vector<std::shared_ptr<ProfileTimeResult>> time_steps;

    void update(TimestampSteady latest_valid);
};

struct TimeDomainProfileResult {
    explicit TimeDomainProfileResult(size_t task_count);
    TimestampSteady last_update{kTimestampSteadyZero};
    size_t total_time_steps{0};
    ProfileTimeResult nano_seconds_per_frame;
    ProfileTimeResult delay;
    std::vector<std::shared_ptr<ProfileTimeResult>> tasks;
    std::map<std::string, std::shared_ptr<TimeStepProfileTimeResult>> time_step_tasks;
};



struct TimeDomainStateResult {
    std::vector<std::vector<TimeDomainTaskState>> task_finished;

    void init(size_t time_step_count, size_t task_count);
};

struct ProcessingTaskInfo {
    ProcessingTaskInfo(size_t task_index,
                       std::string task_id,
                       task_util::TaskType task_type,
                       size_t time_step,
                       std::string pattern_instance_id,
                       std::vector<size_t> predecessors);
    size_t task_index;
    std::string task_id;
    dataflow::task_util::TaskType task_type;
    size_t time_step;
    std::string pattern_instance_id;
    std::vector<size_t> predecessors;

};

class TimeDomainStateProcessing {
 public:
    explicit TimeDomainStateProcessing(std::shared_ptr<TimeDomainState> time_domain_state);
    void update(TimeDuration max_offset);
    const std::vector<ProcessingTaskInfo>& getInfo();
    const TimeDomainProfileResult& getProfileResult();
    const TimeDomainStateResult &getCurrentState() const;
    size_t getTimeStepCount() const;

 private:
    std::shared_ptr<TimeDomainState> time_domain_state_;
    std::vector<ProcessingTaskInfo> info_;
    TimeDomainProfileResult profile_result_;
    TimeDomainStateResult current_state_;
    std::vector<std::vector<size_t>> time_step_to_tasks_;
    std::vector<size_t> start_task_index_;
    std::vector<size_t> end_task_index_;

    util::CircularDynamicBuffer<TimestampSteady> start_time_;
    util::CircularDynamicBuffer<TimeDuration> total_time_;

    std::vector<util::CircularDynamicBuffer<TimeDuration>> task_time_waiting_;
    std::vector<util::CircularDynamicBuffer<TimeDuration>> task_time_processing_;

    void resetTimeStep(int time_step);
};

} // dataflow

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_STATE_TIMEDOMAINSTATERESULT_H_
