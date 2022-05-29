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

namespace traact::dataflow {
    class TaskFlowTimeDomain {
    public:
        TaskFlowTimeDomain(std::set<buffer::BufferFactory::Ptr> genericFactoryObjects,
                           DefaultComponentGraphPtr componentGraph, int time_domain,
                           const component::Component::SourceFinishedCallback &callback);

        void Init();
        void Stop();


        std::future<buffer::SourceComponentBuffer *> RequestSourceBuffer(TimestampType ts, int component_index);
        void CommitSourceBuffer(buffer::SourceComponentBuffer *buffer, bool valid);
    private:
        std::set<buffer::BufferFactory::Ptr> generic_factory_objects_;
        DefaultComponentGraphPtr component_graph_;
        const int time_domain_;
        component::Component::SourceFinishedCallback source_finished_callback;

        // data for taskflow
        struct SourceData {
            buffer::TimeStepBuffer* buffer;
            int component_index;
        };
        struct ComponentData {
            buffer::ComponentBuffer* buffer;
            component::Component* component;
        };
        struct TimeStepData {
            std::map<std::string, SourceData> source_data;
            std::map<std::string, ComponentData> component_data;
            std::map<std::string, tf::Task> component_id_to_task;
        };

        tf::Executor executor_;
        std::vector<tf::Taskflow> taskflow_;
        std::vector<TimeStepData> time_step_data_;
        std::map<std::string, std::set<std::string>> component_to_successors_;
        std::set<std::string> component_end_points_;
        buffer::TimeDomainBuffer time_domain_buffer_;

        // time domain data
        traact::buffer::TimeDomainManagerConfig time_domain_config_;
        std::atomic_flag source_finished_{ATOMIC_FLAG_INIT};
        void MasterSourceFinished();

        // concurrent running time steps
        std::mutex flow_mutex_;
        tf::SmallVector<bool, 10> running_taskflows_;
        std::vector<tf::Future<void>> taskflow_execute_future_;
        Semaphore free_taskflows_semaphore_;
        WaitForValue<TimestampType> latest_running_ts_;
        std::set<TimestampType> queued_timestamps_;

        std::mutex running_mutex_;
        tf::SmallVector<TimestampType, 10> running_timestamps_;


        int IsTimestampRunning(const TimestampType& ts);
        void
        ScheduleEvent(MessageType message_type, TimestampType timestamp);

        void FreeTaskflow(int taskflow_id);

        void CreateBuffer();

        void Prepare();

        void CreateGraphTasks(const int concurrent_index);

        void RunTaskFlowFromQueue(int taskflow_id);

        int GetFreeTaskFlow();

        void SetTaskflowFree(int taskflow_id);

        void
        TakeTaskflow(int taskflow_id, std::chrono::time_point<std::chrono::system_clock, TimeDurationType> &next_ts);

        void PrepareComponents();

        void PrepareTaskData();
    };
}




#endif //TRAACTMULTI_TASKFLOWTIMEDOMAIN_H
