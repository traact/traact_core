/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWPROFILER_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWPROFILER_H_

#include <taskflow/taskflow.hpp>
#include "traact/dataflow/state/TimeDomainState.h"
#include <unordered_map>
namespace traact::dataflow {

 class TaskFlowProfiler : public tf::ObserverInterface {

  public:
     explicit TaskFlowProfiler(TimeDomainState::SharedPtr time_domain_state, const tf::Taskflow& taskflow);
     void set_up(size_t num_workers) final;
     void on_entry(tf::WorkerView worker_view, tf::TaskView task_view) final;
     void on_exit(tf::WorkerView worker_view, tf::TaskView task_view) final;

  private:
     TimeDomainState::SharedPtr time_domain_state_;
     std::unordered_map<size_t, size_t > task_hash_to_index_;

     void addTask(const tf::Task &task);
 };

} // dataflow

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWPROFILER_H_
