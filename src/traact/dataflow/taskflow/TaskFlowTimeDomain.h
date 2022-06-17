/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWTIMEDOMAIN_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWTIMEDOMAIN_H_

#include "traact/component/Component.h"
#include "traact/buffer/DataFactory.h"
#include <taskflow/taskflow.hpp>
#include "traact/buffer/TimeDomainBuffer.h"
#include "traact/util/Semaphore.h"
#include <taskflow/algorithm/pipeline.hpp>
#include "TaskFlowTaskFunctions.h"
#include <traact/buffer/DataBufferFactory.h>
#include "TaskFlowUtils.h"
#include "TaskFlowAsyncScheduler.h"
#include "TaskFlowInFlowScheduler.h"



namespace traact::dataflow {

using DefaultScheduler = TaskFlowInFlowScheduler;

class TaskFlowTimeDomain {
 public:
    TaskFlowTimeDomain(int time_domain,
                       component::ComponentGraph::Ptr component_graph,
                       buffer::DataBufferFactoryPtr buffer_factory,
                       component::Component::SourceFinishedCallback callback);

    void init();
    void start();
    void stop();

    std::future<buffer::SourceComponentBuffer *> requestSourceBuffer(Timestamp timestamp, int component_index);
 private:
    const int time_domain_;
    component::ComponentGraph::Ptr  component_graph_;
    buffer::DataBufferFactoryPtr generic_factory_objects_;

    component::Component::SourceFinishedCallback source_finished_callback_;

    // data for taskflow

    tf::Taskflow taskflow_;

    std::vector<TimeStepData> time_step_data_;
    std::map<std::string, tf::Task> inter_time_step_tasks_;
    std::map<std::string, std::set<std::string>> component_to_successors_;
    std::set<std::string> component_end_points_;
    std::set<std::string> component_start_points_;
    std::shared_ptr<buffer::TimeDomainBuffer> time_domain_buffer_;
    std::atomic<bool> running_{};

    int time_step_count_{0};
    tf::SmallVector<int, kStartEntries> start_entries_;
    bool stop_called_{false};
    // time domain data
    std::set<component::ComponentGraph::PatternComponentPair> components_;
    traact::buffer::TimeDomainManagerConfig time_domain_config_;
    std::atomic_flag source_finished_ = ATOMIC_FLAG_INIT;
    void masterSourceFinished();
    std::map<std::string, ModuleTask> component_modules_;

    std::unique_ptr<DefaultScheduler> scheduler_;





    void createBuffer();

    void prepare();

    void createTimeStepTasks(const int time_step_index);




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



    tf::Task createSeamEntryTask(int time_step_index, const std::string &seam_entry_name);

    void createModuleConstraintTasks(int time_step_index);
};
}

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWTIMEDOMAIN_H_
