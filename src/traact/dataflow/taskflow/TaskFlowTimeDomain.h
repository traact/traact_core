/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWTIMEDOMAIN_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWTIMEDOMAIN_H_

#include "traact/component/Component.h"
#include "traact/buffer/DataFactory.h"
#include "traact/traact.h"
#include <taskflow/taskflow.hpp>
#include "traact/buffer/TimeDomainBuffer.h"
#include "traact/util/Semaphore.h"
#include <taskflow/algorithm/pipeline.hpp>
#include "TaskFlowTaskFunctions.h"
#include <traact/buffer/DataBufferFactory.h>
#include "TaskFlowUtils.h"
namespace traact::dataflow {
class TaskFlowTimeDomain {
 public:
    TaskFlowTimeDomain(int time_domain,
                       DefaultComponentGraphPtr component_graph,
                       buffer::DataBufferFactoryPtr buffer_factory,
                       component::Component::SourceFinishedCallback callback);

    void init();
    void start();
    void stop();

    std::future<buffer::SourceComponentBuffer *> requestSourceBuffer(Timestamp timestamp, int component_index);
 private:
    const int time_domain_;
    DefaultComponentGraphPtr component_graph_;
    buffer::DataBufferFactoryPtr generic_factory_objects_;

    component::Component::SourceFinishedCallback source_finished_callback_;

    // data for taskflow
    tf::Executor executor_;
    tf::Taskflow taskflow_;
    tf::Future<void> taskflow_future_;
    std::vector<TimeStepData> time_step_data_;
    std::map<std::string, tf::Task> inter_time_step_tasks_;
    std::map<std::string, std::set<std::string>> component_to_successors_;
    std::set<std::string> component_end_points_;
    std::set<std::string> component_start_points_;
    std::unique_ptr<buffer::TimeDomainBuffer> time_domain_buffer_;
    std::atomic<bool> running_{};
    std::vector<std::vector<bool>> source_set_input_;
    Semaphore teardown_wait_{1, 0, std::chrono::duration_cast<std::chrono::milliseconds>(kDataflowStopTimeout)};
    int time_step_count_{};
    int time_step_latest_{};
    tf::SmallVector<int, kStartEntries> start_entries_;
    bool stop_called_{false};
    // time domain data
    std::set<component::ComponentGraph::PatternComponentPair> components_;
    traact::buffer::TimeDomainManagerConfig time_domain_config_;
    std::atomic_flag source_finished_ = ATOMIC_FLAG_INIT;
    void masterSourceFinished();
    traact::WaitForInit configure_finished_;
    traact::WaitForInit start_finished_;
    traact::WaitForInit stop_finished_;
    traact::WaitForInit teardown_finished_;

    // concurrent running time steps
    std::mutex flow_mutex_;
    //tf::SmallVector<bool, 10> running_taskflows_;
    std::vector<bool> running_taskflows_;
    std::vector<tf::Future<void>> taskflow_execute_future_;
    Semaphore free_taskflows_semaphore_;
    WaitForTimestamp latest_running_ts_;
    std::queue<std::pair<Timestamp, EventType>> queued_messages_;
    Timestamp latest_scheduled_ts_;

    std::mutex running_mutex_;
    //tf::SmallVector<Timestamp, 10> running_timestamps_;
    std::vector<Timestamp> running_timestamps_;
    traact::Semaphore taskflow_started_{1, 0, kFreeTaskFlowTimeout};

    int isTimestampRunning(const Timestamp &timestamp);
    void
    scheduleEvent(EventType message_type, Timestamp timestamp);

    void freeTimeStep(int time_step_index);

    void createBuffer();

    void prepare();

    void createTimeStepTasks(const int time_step_index);

    void runTaskFlowFromQueue();

    void setTaskflowFree(int time_step_index);

    void
    takeTaskflow(int taskflow_id, std::chrono::time_point<std::chrono::system_clock, TimeDuration> &next_ts);

    void prepareComponents();

    void prepareTaskData();

    void createTask(const int time_step_index, TimeStepData &time_step_data,
                    const std::pair<component::ComponentGraph::PatternPtr,
                                    component::ComponentGraph::ComponentPtr> &component);

    void createInterTimeStepDependencies();

    void globalTaskflowStart();

    void globalTaskflowEnd();

    tf::Task createLocalStartTask(int time_step_index, const std::string& name);

    tf::Task createLocalEndTask(int time_step_index, const std::string& name);

    std::string getTaskName(const int time_step_index, const std::string &instance_id) const;

    void cancelOlderEvents(Timestamp timestamp, int component_index);

    tf::Task createSeamEntryTask(int time_step_index, const std::string &seam_entry_name);
};
}

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWTIMEDOMAIN_H_
