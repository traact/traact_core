/*  BSD 3-Clause License
 *
 *  Copyright (c) 2020, FriederPankratz <frieder.pankratz@gmail.com>
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/

#ifndef TRAACTMULTI_TASKFLOWTIMEDOMAIN_H
#define TRAACTMULTI_TASKFLOWTIMEDOMAIN_H

#include <traact/component/Component.h>
#include <traact/buffer/BufferFactory.h>
#include <traact/traact.h>
#include <taskflow/taskflow.hpp>
#include <traact/buffer/TimeDomainBuffer.h>
#include "traact/util/Semaphore.h"
#include <taskflow/algorithm/pipeline.hpp>
#include "TaskFlowTaskFunctions.h"

namespace traact::dataflow {
    class TaskFlowTimeDomain {
    public:
        TaskFlowTimeDomain(std::set<buffer::BufferFactory::Ptr> genericFactoryObjects,
                           DefaultComponentGraphPtr componentGraph, int time_domain,
                           const component::Component::SourceFinishedCallback &callback);

        void Init();
        void Start();
        void Stop();


        std::future<buffer::SourceComponentBuffer *> RequestSourceBuffer(TimestampType ts, int component_index);
        void CommitSourceBuffer(buffer::SourceComponentBuffer *buffer, bool valid);
    private:
        std::set<buffer::BufferFactory::Ptr> generic_factory_objects_;
        DefaultComponentGraphPtr component_graph_;
        const int time_domain_;
        component::Component::SourceFinishedCallback source_finished_callback;

        // data for taskflow
        tf::Executor executor_;
        tf::Taskflow taskflow_;
        tf::Future<void> taskflow_future_;
        std::vector<TimeStepData> time_step_data_;
        std::map<std::string, tf::Task> inter_time_step_tasks_;
        std::map<std::string, std::set<std::string>> component_to_successors_;
        std::set<std::string> component_end_points_;
        std::set<std::string> component_start_points_;
        buffer::TimeDomainBuffer time_domain_buffer_;
        std::atomic<bool> running_;
        std::vector<std::vector<bool>> source_set_input;
        Semaphore teardown_wait_{1, 0, std::chrono::seconds(10)};
        int time_step_count_;
        int time_step_latest_;
        tf::SmallVector<int,10> start_entries_;
        bool stop_called{false};
        // time domain data
        std::set<component::ComponentGraph::PatternComponentPair> components_;
        traact::buffer::TimeDomainManagerConfig time_domain_config_;
        std::atomic_flag source_finished_{ATOMIC_FLAG_INIT};
        void MasterSourceFinished();
        traact::WaitForInit configure_finished_;
        traact::WaitForInit start_finished_;

        // concurrent running time steps
        std::mutex flow_mutex_;
        //tf::SmallVector<bool, 10> running_taskflows_;
        std::vector<bool> running_taskflows_;
        std::vector<tf::Future<void>> taskflow_execute_future_;
        Semaphore free_taskflows_semaphore_;
        WaitForTimestamp latest_running_ts_;
        std::queue<std::pair<TimestampType, MessageType>> queued_messages_;
        TimestampType latest_scheduled_ts_;

        std::mutex running_mutex_;
        //tf::SmallVector<TimestampType, 10> running_timestamps_;
        std::vector<TimestampType> running_timestamps_;
        traact::Semaphore taskflow_started_{1,0,std::chrono::seconds(10)};


        int IsTimestampRunning(const TimestampType& ts);
        void
        ScheduleEvent(MessageType message_type, TimestampType timestamp);

        void FreeTimeStep(int time_step_index);

        void CreateBuffer();

        void Prepare();

        void CreateTimeStepTasks(const int time_step_index);

        void RunTaskFlowFromQueue();

        void SetTaskflowFree(int time_step_index);

        void
        TakeTaskflow(int taskflow_id, std::chrono::time_point<std::chrono::system_clock, TimeDurationType> &next_ts);

        void PrepareComponents();

        void PrepareTaskData();

        void CreateTask(const int time_step_index, TimeStepData &time_step_data,
                        const std::pair<component::ComponentGraph::PatternPtr, component::ComponentGraph::ComponentPtr> &component);

        void CreateInterTimeStepDependencies();

        void GlobalTaskflowStart();

        void GlobalTaskflowEnd();

        tf::Task CreateLocalStartTask(int time_step_index, std::string name);

        tf::Task CreateLocalEndTask(int time_step_index, std::string name);

        std::string GetTaskName(const int time_step_index, const std::string &instance_id) const;

        void CancelOlderEvents(TimestampType ts, int component_index);

        tf::Task CreateSeamEntryTask(int time_step_index, const std::string &seam_entry_name);
    };
}




#endif //TRAACTMULTI_TASKFLOWTIMEDOMAIN_H
