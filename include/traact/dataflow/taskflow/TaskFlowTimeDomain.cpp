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

#include "TaskFlowTimeDomain.h"
#include "TaskFlowTaskFunctions.h"
#include <utility>


namespace traact::dataflow {

    TaskFlowTimeDomain::TaskFlowTimeDomain(std::set<buffer::BufferFactory::Ptr> genericFactoryObjects,
                                           DefaultComponentGraphPtr componentGraph, int time_domain,
                                           const component::Component::SourceFinishedCallback &callback) : generic_factory_objects_(std::move(genericFactoryObjects)),
                                                                                                           component_graph_(std::move(componentGraph)),
                                                                                                           time_domain_(time_domain),
                                                                                                           source_finished_callback(callback),
                                                                                                           free_taskflows_semaphore_(component_graph_->GetTimeDomainConfig(time_domain_).ringbuffer_size,component_graph_->GetTimeDomainConfig(time_domain_).ringbuffer_size, std::chrono::seconds(60)),
                                                                                                           time_domain_buffer_(time_domain_, generic_factory_objects_)
                                                                                                           //,events_scheduled_(1,0, std::chrono::milliseconds (100))
                                                                                                           {

    }

    void TaskFlowTimeDomain::Init() {


        CreateBuffer();

        Prepare();

        for(int concurrent_timesteps =0; concurrent_timesteps < time_domain_config_.ringbuffer_size; ++concurrent_timesteps)
            CreateGraphTasks(concurrent_timesteps);



    }


    void TaskFlowTimeDomain::CreateGraphTasks(const int concurrent_index) {

        auto components = component_graph_->getPatternsForTimeDomain(time_domain_);
        taskflow_[concurrent_index].name(fmt::format("TimeDomain{0}_Concurrent{1}", time_domain_, concurrent_index));
        auto& time_step_data = time_step_data_[concurrent_index];
        for (const auto &component : components) {

            std::shared_ptr<pattern::instance::PatternInstance> pattern_instance = component.first;
            if (!pattern_instance) {
                continue;
            }
            auto instance_id = component.first->instance_id;

            auto task = taskflow_[concurrent_index].placeholder();

            task.name(fmt::format("{0}_task{1}",instance_id, concurrent_index));

            switch(component.second->getComponentType()){
                case component::ComponentType::AsyncSource:{
                    auto local_result = time_step_data.source_data.find(instance_id);
                    if(local_result == time_step_data.source_data.end()){
                        auto error_message = fmt::format("no source data for instance {0}", instance_id);
                        SPDLOG_ERROR(error_message);
                        throw std::range_error(error_message);
                    }

                    task.work([local_data = local_result->second]() {
                        auto buffer = local_data.buffer;
                        SPDLOG_TRACE("Source: start waiting {0} ts {1}", component_index, buffer->GetTimestamp().time_since_epoch().count());
                        auto lock = buffer->GetSourceLock(local_data.component_index);
                        lock.wait();
                        auto commit_successful = lock.get();
                        if(!commit_successful)
                            SPDLOG_ERROR("Source: finished with error {0} ts {1} {2}", local_data.component_index, buffer->GetTimestamp().time_since_epoch().count(), commit_successful);
                        SPDLOG_TRACE("Source: finished {0} ts {1}", component_index, buffer.GetTimestamp().time_since_epoch().count());
                    });
                    break;
                }
                case component::ComponentType::AsyncSink:
                case component::ComponentType::SyncSource:
                case component::ComponentType::SyncFunctional:
                case component::ComponentType::AsyncFunctional:
                case component::ComponentType::SyncSink:{
                    auto local_result = time_step_data.component_data.find(instance_id);
                    if(local_result == time_step_data.component_data.end()){
                        auto error_message = fmt::format("no component data for instance {0}", instance_id);
                        SPDLOG_ERROR(error_message);
                        throw std::range_error(error_message);
                    }
                    task.work([local_data = local_result->second]() {

                            auto buffer = local_data.buffer;
                            SPDLOG_TRACE("{0}: start ts: {1}", local_data.component->getName(), buffer->GetTimestamp().time_since_epoch().count());
                            local_data.component->processTimePoint(*buffer);
                            SPDLOG_TRACE("{0}: finished ts {1}", local_data.component->getName(), buffer->GetTimestamp().time_since_epoch().count());
                        });
                    break;
                }
                case component::ComponentType::Invalid:
                default:
                    spdlog::error("Unsupported ComponentType {0} for component {1}", component.second->getComponentType(), component.second->getName());
                    break;
            }

            time_step_data.component_id_to_task.emplace(instance_id, task);

        }

        for(const auto& component_successors : component_to_successors_){
            auto& task = time_step_data.component_id_to_task.at(component_successors.first);
            for(const auto& successor : component_successors.second)
                task.precede(time_step_data.component_id_to_task.at(successor));
        }

        //SPDLOG_INFO("dump of task flow \n{0}", taskflow_[concurrent_index].dump());


    }

    void TaskFlowTimeDomain::Prepare() {
        PrepareComponents();
        PrepareTaskData();
    }

    void TaskFlowTimeDomain::PrepareComponents() {
        auto components = component_graph_->getPatternsForTimeDomain(time_domain_);
        for (const auto &component : components) {

            std::shared_ptr<pattern::instance::PatternInstance> pattern_instance = component.first;
            if (!pattern_instance) {
                spdlog::warn("skipping non dataflow pattern : {0}", component.first->instance_id);
                continue;
            }

            std::set<std::string> successors;

            bool is_endpoint = true;

            for (auto port : pattern_instance->getProducerPorts()) {
                for (auto input_port : port->connectedToPtr()) {
                    successors.emplace(input_port->getID().first);
                    is_endpoint = false;

                }
            }
            if(is_endpoint)
                component_end_points_.emplace(component.first->instance_id);

            component_to_successors_.emplace(component.first->instance_id, std::move(successors));

            switch(component.second->getComponentType()){
                case component::ComponentType::AsyncSource:{

                    auto component_index = time_domain_buffer_.GetComponentIndex(component.first->instance_id);
                    auto request_source_callback = std::bind(&TaskFlowTimeDomain::RequestSourceBuffer, this, std::placeholders::_1, component_index);
                    component.second->setRequestCallback(request_source_callback);
                    break;
                }
                case component::ComponentType::AsyncSink:
                case component::ComponentType::SyncSource:
                case component::ComponentType::SyncFunctional:
                case component::ComponentType::AsyncFunctional:
                case component::ComponentType::SyncSink:
                case component::ComponentType::Invalid:
                default:

                    break;
            }


        }
    }

    void TaskFlowTimeDomain::PrepareTaskData() {
        auto components = component_graph_->getPatternsForTimeDomain(time_domain_);
        for(int concurrent_index =0; concurrent_index < time_domain_config_.ringbuffer_size; ++concurrent_index) {
            TimeStepData& time_step_data = time_step_data_[concurrent_index];

            for (const auto &component: components) {

                std::shared_ptr<pattern::instance::PatternInstance> pattern_instance = component.first;
                if (!pattern_instance) {
                    continue;
                }
                auto instance_id = component.first->instance_id;
                auto &time_step_buffer = time_domain_buffer_.GetTimeStepBuffer(concurrent_index);
                auto component_index = time_step_buffer.GetComponentIndex(instance_id);

                switch (component.second->getComponentType()) {
                    case component::ComponentType::AsyncSource: {


                        SourceData data;
                        data.buffer = &time_step_buffer;
                        data.component_index = component_index;
                        time_step_data.source_data.emplace(instance_id, data);

                        break;
                    }
                    case component::ComponentType::AsyncSink:
                    case component::ComponentType::SyncSource:
                    case component::ComponentType::SyncFunctional:
                    case component::ComponentType::AsyncFunctional:
                    case component::ComponentType::SyncSink: {
                        auto &component_buffer = time_step_buffer.GetComponentBuffer(component_index);
                        auto function_object = component.second;
                        ComponentData data;
                        data.buffer = &component_buffer;
                        data.component = function_object.get();
                        time_step_data.component_data.emplace(instance_id, data);
                        break;
                    }
                    case component::ComponentType::Invalid:
                    default:
                        spdlog::error("Unsupported ComponentType {0} for component {1}",
                                      component.second->getComponentType(), component.second->getName());
                        break;
                }
            }
        }
    }

    void TaskFlowTimeDomain::CreateBuffer() {


        auto commit_callback = std::bind(&TaskFlowTimeDomain::CommitSourceBuffer, this, std::placeholders::_1, std::placeholders::_2);


        time_domain_config_ = component_graph_->GetTimeDomainConfig(time_domain_);
        auto concurrent_timesteps = time_domain_config_.ringbuffer_size;
        //auto component_count = component_graph_->getPatternsForTimeDomain(time_domain_).size();
        taskflow_.resize(concurrent_timesteps);
        time_domain_buffer_ = buffer::TimeDomainBuffer(time_domain_, generic_factory_objects_);
        time_domain_buffer_.Init(*component_graph_, commit_callback);
        time_step_data_.resize(concurrent_timesteps);
        taskflow_execute_future_.resize(concurrent_timesteps);
        running_taskflows_.resize(concurrent_timesteps, false);
        running_timestamps_.resize(concurrent_timesteps);
    }



    void TaskFlowTimeDomain::MasterSourceFinished() {
        // first finished call ends playback
        if(source_finished_.test_and_set()) {
            return;
        }

        source_finished_callback();
    }



    std::future<buffer::SourceComponentBuffer *>
    TaskFlowTimeDomain::RequestSourceBuffer(TimestampType ts, int component_index) {

        SPDLOG_TRACE("RequestSource comp: {0} ts: {1} start", component_index, ts.time_since_epoch().count());

        ScheduleEvent(MessageType::Data, ts);

        return std::async(std::launch::deferred, [&, ts, component_index]() mutable -> buffer::SourceComponentBuffer*{
            auto wait_value = latest_running_ts_.waitValue(ts);

            auto taskflow_index = IsTimestampRunning(ts);
            if(taskflow_index < 0){
                SPDLOG_ERROR("source component future, no valid taskflow index");
                return nullptr;
            }else {
                SPDLOG_TRACE("source component future, taskflow {0} component {3} ts {1} wait_ts {2}", taskflow_index, ts.time_since_epoch().count(), wait_value.time_since_epoch().count(), component_index);
                return time_domain_buffer_.GetTimeStepBuffer(taskflow_index).GetSourceComponentBuffer(component_index);
            }
        });
    }

    void TaskFlowTimeDomain::CommitSourceBuffer(buffer::SourceComponentBuffer *buffer, bool valid) {

    }

    void TaskFlowTimeDomain::FreeTaskflow(int taskflow_id) {

        std::unique_lock flow_guard(flow_mutex_);
        if(queued_timestamps_.empty())
            SetTaskflowFree(taskflow_id);
        else
            RunTaskFlowFromQueue(taskflow_id);

    }



    void TaskFlowTimeDomain::RunTaskFlowFromQueue(int taskflow_id) {


        auto next = queued_timestamps_.begin();
        auto next_ts = *next;
        queued_timestamps_.erase(next_ts);

        SPDLOG_TRACE("run task from queue {0} waiting timestamps, start {1}", taskflow_id, next_ts.time_since_epoch().count());

        TakeTaskflow(taskflow_id, next_ts);


        auto& time_step_buffer = time_domain_buffer_.GetTimeStepBuffer(taskflow_id);
        time_step_buffer.ResetForTimestamp(next_ts);
        latest_running_ts_.notifyAll(next_ts);


        executor_.silent_async([&, taskflow_id, next_ts]()mutable {
            SPDLOG_TRACE("run task from queue execute {0} {1}", taskflow_id, next_ts.time_since_epoch().count());
            auto running = executor_.run(taskflow_[taskflow_id]);
            running.wait();
            FreeTaskflow(taskflow_id);
        });


    }




    int TaskFlowTimeDomain::IsTimestampRunning(const TimestampType &ts) {
        std::unique_lock guard_running(running_mutex_);
        for(int index =0;index < running_timestamps_.size();++index){
            if(running_timestamps_[index] == ts)
                return index;
        }
        return -1;
    }

    int TaskFlowTimeDomain::GetFreeTaskFlow() {
        std::unique_lock guard_running(running_mutex_);
        for(int index =0; index < running_taskflows_.size(); ++index){
            if(!running_taskflows_[index]){
                return index;
            }
        }
        assert(false);
    }

    void TaskFlowTimeDomain::SetTaskflowFree(int taskflow_id) {
        {
            std::unique_lock guard_running(running_mutex_);
            running_taskflows_[taskflow_id] = false;
            running_timestamps_[taskflow_id] = TimestampType::min();
        }
        free_taskflows_semaphore_.notify();
    }
    void TaskFlowTimeDomain::TakeTaskflow(int taskflow_id,
                                          std::chrono::time_point<std::chrono::system_clock, TimeDurationType> &next_ts) {
        std::unique_lock guard_running(running_mutex_);
        running_taskflows_[taskflow_id] = true;
        running_timestamps_[taskflow_id] = next_ts;
    }

    void
    TaskFlowTimeDomain::ScheduleEvent(MessageType message_type, TimestampType timestamp) {

        SPDLOG_TRACE("schedule timestamp {0}", timestamp.time_since_epoch().count());
        std::unique_lock guard_flow(flow_mutex_);
        int running_index = IsTimestampRunning(timestamp);
        SPDLOG_TRACE("schedule timestamp {0} is running {1}", timestamp.time_since_epoch().count(), running_index);
        if(running_index >= 0)
            return;

        SPDLOG_TRACE("schedule timestamp start {0} ", timestamp.time_since_epoch().count());
        queued_timestamps_.emplace(timestamp);
        //events_scheduled_.notify();
        //queued_timestamps_.emplace_hint(queued_timestamps_.cend(), timestamp);
        if(free_taskflows_semaphore_.try_wait()){
            RunTaskFlowFromQueue(GetFreeTaskFlow());
        }

        SPDLOG_TRACE("schedule timestamp done {0} ", timestamp.time_since_epoch().count());

    }

    void TaskFlowTimeDomain::Stop() {
//        run_scheduler_ =false;
//        auto status = scheduler_.wait_for(std::chrono::seconds(1));
//        if(status != std::future_status::ready){
//
//            SPDLOG_WARN("scheduler did not quit on time, attempting to cancel scheduler");
//            scheduler_.cancel();
//
//        }
        //scheduler_.cancel();
        executor_.wait_for_all();
    }




}