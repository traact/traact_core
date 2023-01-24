/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_STATE_TIMEDOMAINSTATE_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_STATE_TIMEDOMAINSTATE_H_

#include <memory>
#include <vector>
#include <deque>
#include <mutex>
#include <traact/datatypes.h>

namespace traact::dataflow {

struct TaskStateData {
    explicit TaskStateData(size_t index, bool is_start);
    size_t task_index;
    bool is_start;
    TimestampSteady event_time;
};

struct TimeDomainStateData {
    TimeDomainStateData(size_t worker_count);
    std::vector<std::deque<TaskStateData>> task_data;
};

struct TaskInfo {
    TaskInfo(std::string task_id, std::vector<std::string> predecessors);
    std::string task_id;
    std::vector<std::string> predecessors;
};

class TimeDomainState {
 public:
    using SharedPtr = std::shared_ptr<TimeDomainState>;
    using TimeDomainStateDataPtr = std::shared_ptr<TimeDomainStateData>;


    void init(size_t worker_count, size_t time_step_count);
    size_t addTask(const std::string &task_id, std::vector<std::string> predecessors);
    void taskStart(size_t task_index, size_t worker_id);
    void taskEnd(size_t task_index, size_t worker_id);


    TimeDomainStateDataPtr getData();
    [[nodiscard]] const std::vector<TaskInfo> &getTaskInfo() const;
    size_t getTaskCount() const;
    size_t getTimeStepCount() const;


 private:
    size_t worker_count_{0};
    size_t time_step_count_{1};
    std::vector<TaskInfo> task_info_;

    std::vector<std::mutex> data_mutex_;
    TimeDomainStateDataPtr buffer_data_;

};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_STATE_TIMEDOMAINSTATE_H_
