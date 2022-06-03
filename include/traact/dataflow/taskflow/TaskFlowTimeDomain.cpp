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
#include <utility>

std::string TimeStepStartTaskName(int index){
    return fmt::format("TIME_STEP_START_{0}",index);
}
std::string TimeStepEndTaskName(int index){
    return fmt::format("TIME_STEP_END_{0}",index);
}


namespace traact::dataflow {

    TaskFlowTimeDomain::TaskFlowTimeDomain(std::set<buffer::BufferFactory::Ptr> genericFactoryObjects,
                                           DefaultComponentGraphPtr componentGraph, int time_domain,
                                           const component::Component::SourceFinishedCallback &callback) : generic_factory_objects_(std::move(genericFactoryObjects)),
                                                                                                           component_graph_(std::move(componentGraph)),
                                                                                                           time_domain_(time_domain),
                                                                                                           source_finished_callback(callback),
                                                                                                           free_taskflows_semaphore_(component_graph_->GetTimeDomainConfig(time_domain_).ringbuffer_size,component_graph_->GetTimeDomainConfig(time_domain_).ringbuffer_size, std::chrono::seconds(60)),
                                                                                                           time_domain_buffer_(time_domain_, generic_factory_objects_),
                                                                                                           latest_running_ts_{component_graph_->GetTimeDomainConfig(time_domain_).max_offset}
                                                                                                           //,events_scheduled_(1,0, std::chrono::milliseconds (100))
                                                                                                           {

    }

    void TaskFlowTimeDomain::Init() {




        CreateBuffer();

        Prepare();

        taskflow_.name(fmt::format("{0}_TD:{1}", component_graph_->getName(), time_domain_));

        for(int time_step_index =0; time_step_index < time_step_count_; ++time_step_index)
            CreateTimeStepTasks(time_step_index);

        CreateInterTimeStepDependencies();

        SPDLOG_TRACE("dump of task flow \n{0}", taskflow_.dump());

        for(int time_step_index =0; time_step_index < time_step_count_; ++time_step_index)
            time_domain_buffer_.GetTimeStepBuffer(time_step_index).SetEvent(TimestampType::min(), MessageType::Invalid);
    }


    void TaskFlowTimeDomain::CreateTimeStepTasks(const int time_step_index) {


        auto& time_step_data = time_step_data_[time_step_index];
        for (const auto &component : components_) {

            std::shared_ptr<pattern::instance::PatternInstance> pattern_instance = component.first;
            if (!pattern_instance) {
                continue;
            }
            CreateTask(time_step_index, time_step_data, component);

        }




        for(const auto& component_successors : component_to_successors_){
            auto& task = time_step_data.component_id_to_task.at(component_successors.first);
            for(const auto& successor : component_successors.second)
                task.precede(time_step_data.component_id_to_task.at(successor));
        }

//        auto start_task = CreateLocalStartTask(time_step_index, TimeStepStartTaskName(time_step_index));
//        for(const auto& start_point : component_start_points_){
//            start_task.precede(time_step_data.component_id_to_task.at(start_point));
//        }
//        time_step_data.component_id_to_task.emplace("TIME_STEP_START", start_task);
//
        auto end_task = CreateLocalEndTask(time_step_index, TimeStepEndTaskName(time_step_index));
        for(const auto& end_point : component_end_points_){
            end_task.succeed(time_step_data.component_id_to_task.at(end_point));
        }
        time_step_data.component_id_to_task.emplace("TIME_STEP_END", end_task);




    }

    void TaskFlowTimeDomain::CreateTask(const int time_step_index, TimeStepData &time_step_data,
                                        const std::pair<component::ComponentGraph::PatternPtr, component::ComponentGraph::ComponentPtr> &component) {
        auto instance_id = component.first->instance_id;

        auto task = taskflow_.placeholder();

        task.name(GetTaskName(time_step_index, instance_id));

        auto local_result = time_step_data.component_data.find(instance_id);
        if(local_result == time_step_data.component_data.end()){
            auto error_message = fmt::format("no source data for instance {0}", instance_id);
            SPDLOG_ERROR(error_message);
            throw std::range_error(error_message);
        }

        switch(component.second->getComponentType()){
            case component::ComponentType::AsyncSource:
            case component::ComponentType::SyncSource:{

                task.work([&local_data = local_result->second]() {
                    TaskSource(local_data);
                });
                break;
            }
            case component::ComponentType::AsyncSink:
            case component::ComponentType::SyncFunctional:
            case component::ComponentType::AsyncFunctional:
            case component::ComponentType::SyncSink:{
                task.work([&local_data = local_result->second]() {
                    TaskGenericComponent(local_data);
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

    std::string TaskFlowTimeDomain::GetTaskName(const int time_step_index,
                                                const std::string &instance_id) const { return fmt::format("{0}_task{1}", instance_id, time_step_index); }

    void TaskFlowTimeDomain::Prepare() {
        PrepareComponents();
        PrepareTaskData();
    }

    void TaskFlowTimeDomain::PrepareComponents() {

        for (const auto &component : components_) {

            std::shared_ptr<pattern::instance::PatternInstance> pattern_instance = component.first;
            auto instance_id = component.first->instance_id;
            if (!pattern_instance) {
                spdlog::warn("skipping non dataflow pattern : {0}", instance_id);
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
                component_end_points_.emplace(instance_id);
            if(pattern_instance->getConsumerPorts().empty())
                component_start_points_.emplace(instance_id);

            component_to_successors_.emplace(instance_id, std::move(successors));

            switch(component.second->getComponentType()){
                case component::ComponentType::AsyncSource:
                case component::ComponentType::SyncSource:{

                    auto component_index = time_domain_buffer_.GetComponentIndex(instance_id);
                    auto request_source_callback = std::bind(&TaskFlowTimeDomain::RequestSourceBuffer, this, std::placeholders::_1, component_index);
                    component.second->setRequestCallback(request_source_callback);
                    break;
                }
                case component::ComponentType::AsyncSink:

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

        for(int concurrent_index =0; concurrent_index < time_step_count_; ++concurrent_index) {
            TimeStepData& time_step_data = time_step_data_[concurrent_index];

            for (const auto &component: components_) {

                std::shared_ptr<pattern::instance::PatternInstance> pattern_instance = component.first;
                if (!pattern_instance) {
                    continue;
                }
                auto instance_id = component.first->instance_id;
                auto &time_step_buffer = time_domain_buffer_.GetTimeStepBuffer(concurrent_index);
                auto component_index = time_step_buffer.GetComponentIndex(instance_id);



                auto &component_buffer = time_step_buffer.GetComponentBuffer(component_index);
                auto function_object = component.second;
                auto& parameter = component.first->local_pattern.parameter;
                time_step_data.component_data.emplace(instance_id, ComponentData(time_step_buffer, component_buffer, *function_object.get(), component_index, parameter, running_));
            }
        }

        for (const auto &component : components_) {

            std::shared_ptr<pattern::instance::PatternInstance> pattern_instance = component.first;
            auto instance_id = component.first->instance_id;
            if (!pattern_instance) {
                continue;
            }

            for(int concurrent_index =0; concurrent_index < time_step_count_; ++concurrent_index) {
                TimeStepData& time_step_data = time_step_data_[concurrent_index];
                auto& component_data = time_step_data.component_data.at(instance_id);
                for (auto port: pattern_instance->getConsumerPorts()) {
                        auto input_id = port->connected_to.first;
                        auto& input_data = time_step_data.component_data.at(input_id);
                        component_data.valid_input.push_back(&input_data.valid_output);

                    }
                }
        }

    }

    void TaskFlowTimeDomain::CreateBuffer() {

        auto commit_callback = std::bind(&TaskFlowTimeDomain::CommitSourceBuffer, this, std::placeholders::_1, std::placeholders::_2);

        time_domain_config_ = component_graph_->GetTimeDomainConfig(time_domain_);
        components_ = component_graph_->getPatternsForTimeDomain(time_domain_);
        time_step_count_ = time_domain_config_.ringbuffer_size;
        time_step_latest_ = -1;
        //auto component_count = component_graph_->getPatternsForTimeDomain(time_domain_).size();
        time_domain_buffer_ = buffer::TimeDomainBuffer(time_domain_, generic_factory_objects_);
        time_domain_buffer_.Init(*component_graph_, commit_callback);
        time_step_data_.resize(time_step_count_);
        taskflow_execute_future_.resize(time_step_count_);
        running_taskflows_.resize(time_step_count_, false);
        running_timestamps_.resize(time_step_count_);
        std::vector<bool> tmp;
        tmp.resize(time_step_count_,false);
        source_set_input.resize(time_domain_buffer_.GetCountSources(), tmp);
        latest_scheduled_ts_ = TimestampType::min();
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

        // cancel potential older
        //CancelOlderEvents(ts, component_index);

        auto running_time_step =IsTimestampRunning(ts);
        if(running_time_step < 0){
            if(ts+time_domain_config_.max_offset < latest_scheduled_ts_){
                SPDLOG_ERROR("New data timestamp must be monotonic increasing, latest: {0} now: {1}", latest_scheduled_ts_.time_since_epoch().count(), ts.time_since_epoch().count() );
                std::promise<buffer::SourceComponentBuffer*> value;
                value.set_value(nullptr);
                return value.get_future();

            }

            ScheduleEvent(MessageType::Data, ts);


        } else {
            SPDLOG_TRACE("RequestSource comp: {0} ts: {1} already running", component_index, ts.time_since_epoch().count());
            CancelOlderEvents(ts, component_index);
        }

        return std::async(std::launch::deferred, [&, ts, component_index]() mutable -> buffer::SourceComponentBuffer*{
            latest_running_ts_.wait(ts);

            auto taskflow_index = IsTimestampRunning(ts);
            if(taskflow_index < 0){
                SPDLOG_WARN("source component index {0} ts {1}, rejected value");
                return nullptr;
            }else {
                source_set_input[component_index][taskflow_index] = true;
                SPDLOG_TRACE("source component future, taskflow {0} component {1} ts {2}", taskflow_index, component_index, ts.time_since_epoch().count() );
                return time_domain_buffer_.GetTimeStepBuffer(taskflow_index).GetSourceComponentBuffer(component_index);
            }
        });





    }

    void TaskFlowTimeDomain::CommitSourceBuffer(buffer::SourceComponentBuffer *buffer, bool valid) {

    }

    void TaskFlowTimeDomain::FreeTimeStep(int time_step_index) {

        std::unique_lock flow_guard(flow_mutex_);
        SPDLOG_TRACE("Free Time Step {0}, Reset Locks of ts: {1}", time_step_index, time_domain_buffer_.GetTimeStepBuffer(time_step_index).GetTimestamp().time_since_epoch().count());
        time_domain_buffer_.GetTimeStepBuffer(time_step_index).ResetLock();
        if(queued_messages_.empty())
            SetTaskflowFree(time_step_index);
        else{
            SPDLOG_TRACE("Free Time Step {0}, run from queue", time_step_index);
            RunTaskFlowFromQueue();
        }


    }



    void TaskFlowTimeDomain::RunTaskFlowFromQueue() {


        auto next_ts_message = queued_messages_.front();
        queued_messages_.pop();


        time_step_latest_++;
        time_step_latest_ = time_step_latest_ % time_step_count_;

        SPDLOG_TRACE("Run task from queue, using time step: {0} ts: {1} message: {2}", time_step_latest_, next_ts_message.first.time_since_epoch().count(), next_ts_message.second);

        TakeTaskflow(time_step_latest_, next_ts_message.first);
        auto& time_step_buffer = time_domain_buffer_.GetTimeStepBuffer(time_step_latest_);
        time_step_buffer.SetEvent(next_ts_message.first, next_ts_message.second);


        if(next_ts_message.second == MessageType::Data){
            for (int source_index = 0; source_index < time_domain_buffer_.GetCountSources(); ++source_index) {
                source_set_input[source_index][time_step_latest_] = false;
            }
            latest_running_ts_.notifyAll(next_ts_message.first);
        }
        else {
            for (int source_index = 0; source_index < time_domain_buffer_.GetCountSources(); ++source_index) {
                source_set_input[source_index][time_step_latest_] = true;
                time_step_buffer.GetSourceComponentBuffer(source_index)->Commit(true);
            }

        }

    }




    int TaskFlowTimeDomain::IsTimestampRunning(const TimestampType &ts) {
        std::unique_lock guard_running(running_mutex_);
        auto ts_min = ts - time_domain_config_.max_offset;
        auto ts_max = ts + time_domain_config_.max_offset;
        SPDLOG_TRACE("Search for {0} min {1} max {2}", ts.time_since_epoch().count(), ts_min.time_since_epoch().count(), ts_max.time_since_epoch().count());
        for(int index =0;index < running_timestamps_.size();++index){
            auto latest_min = running_timestamps_[index] - time_domain_config_.max_offset;
            auto latest_max = running_timestamps_[index] + time_domain_config_.max_offset;
            if(latest_max >= ts_min && latest_min <= ts_max){
                SPDLOG_TRACE("found match for ts {0} {1} {2}", ts.time_since_epoch().count(), running_timestamps_[index].time_since_epoch().count(),index);
                return index;
            }

        }
        return -1;
    }

    void TaskFlowTimeDomain::SetTaskflowFree(int time_step_index) {
        {
            std::unique_lock guard_running(running_mutex_);
            running_taskflows_[time_step_index] = false;
            running_timestamps_[time_step_index] = TimestampType::min();
        }
        free_taskflows_semaphore_.notify();
    }
    void TaskFlowTimeDomain::TakeTaskflow(int taskflow_id,
                                          std::chrono::time_point<std::chrono::system_clock, TimeDurationType> &next_ts) {
        std::unique_lock guard_running(running_mutex_);
        running_taskflows_[taskflow_id] = true;
        running_timestamps_[taskflow_id] = next_ts;
    }

    void TaskFlowTimeDomain::ScheduleEvent(MessageType message_type, TimestampType timestamp) {

        SPDLOG_TRACE("Schedule event {0} {1}", timestamp.time_since_epoch().count(), message_type);
        std::unique_lock guard_flow(flow_mutex_);

//        if(message_type == MessageType::Data){
//            int running_index = IsTimestampRunning(timestamp);
//            SPDLOG_TRACE("Schedule event {0} is running {1}", timestamp.time_since_epoch().count(), running_index);
//            if(running_index >= 0)
//                return;
//
//
//        }

        latest_scheduled_ts_ = timestamp;

        if(message_type == MessageType::Teardown){
            int foo = 3;
        }


        queued_messages_.emplace(std::make_pair(timestamp, message_type));

        auto min_ts = timestamp - time_domain_config_.max_offset;
        for (int time_step_index = 0; time_step_index < time_step_count_; ++time_step_index) {
            for (int component_index = 0; component_index < time_domain_buffer_.GetCountAsyncSources(); ++component_index) {
                if(!source_set_input[component_index][time_step_index] && running_taskflows_[time_step_index] && running_timestamps_[time_step_index] < min_ts){
                    auto& time_step_buffer = time_domain_buffer_.GetTimeStepBuffer(time_step_index);
                    SPDLOG_WARN("Cancel source component buffer, because event seems to be missing, time step: {0} component {1} ts: {2}", time_step_index,component_index,  time_step_buffer.GetTimestamp().time_since_epoch().count());
                    time_step_buffer.GetSourceComponentBuffer(component_index)->Cancel();
                }
            }

        }



        if(free_taskflows_semaphore_.try_wait()){
            SPDLOG_TRACE("Schedule event {0} {1}, run from queue", timestamp.time_since_epoch().count(), message_type);
            RunTaskFlowFromQueue();
        }


    }

    void TaskFlowTimeDomain::CancelOlderEvents(TimestampType ts, int component_index) {
        //executor_.silent_async([&, ts, component_index]() {
            std::unique_lock guard_flow(flow_mutex_);
            auto min_ts = ts - time_domain_config_.max_offset;
            for (int time_step_index = 0; time_step_index < time_step_count_; ++time_step_index) {
                if(!source_set_input[component_index][time_step_index] && running_taskflows_[time_step_index] && running_timestamps_[time_step_index]+time_domain_config_.max_offset < min_ts){
                    auto& time_step_buffer = time_domain_buffer_.GetTimeStepBuffer(time_step_index);
                    SPDLOG_WARN("Cancel source component buffer, because a newer timestamp is requested, time step: {0} component {1} buffer ts: {2} current ts {3}", time_step_index,component_index,  time_step_buffer.GetTimestamp().time_since_epoch().count(),ts.time_since_epoch().count());
                    time_step_buffer.GetSourceComponentBuffer(component_index)->Cancel();
                }
            }
        //});
    }

    void TaskFlowTimeDomain::Stop() {

        if(stop_called)
            return;
        stop_called = true;
        SPDLOG_TRACE("Call to Stop dataflow");

        ScheduleEvent(MessageType::Stop, TimestampType::min());
        ScheduleEvent(MessageType::Teardown, TimestampType::min());
        //ScheduleEvent(MessageType::DataflowNoOp, TimestampType::min());
        // stop event for every time step in the taskflow
        for (int time_step_index = 0; time_step_index < time_step_count_; ++time_step_index) {
            ScheduleEvent(MessageType::DataflowStop, TimestampType::min());
        }



        //teardown_wait_.wait();
        //time_domain_buffer_.CancelAll();

        auto status = taskflow_future_.wait_for(std::chrono::seconds(10));
        if(status != std::future_status::ready){
            SPDLOG_WARN("could not stop task flow time domain {0} {1}", component_graph_->getName(), time_domain_);
        }
        running_ = false;

    }

    void TaskFlowTimeDomain::Start() {

        running_ = true;
        taskflow_future_ = executor_.run(taskflow_);
        if(!taskflow_started_.wait()){
            SPDLOG_ERROR("could not start taskflow");
            return;
        }
        ScheduleEvent(MessageType::Configure, TimestampType::min());
        configure_finished_.Wait();
        ScheduleEvent(MessageType::Start, TimestampType::min());
        start_finished_.Wait();

    }

    void TaskFlowTimeDomain::CreateInterTimeStepDependencies() {

        const std::string seam_entry_format = "seam_entry_{0}_{1}";
        const std::string seam_start_format = "seam_start_{0}_{1}";
        const std::string seam_end_format = "seam_end_{0}_{1}";

        auto start_task = taskflow_.placeholder().name("start").work([&]() {
            GlobalTaskflowStart();
        });

        auto end_task = taskflow_.placeholder().name("end").work([&]() {
            GlobalTaskflowEnd();
        });



        auto start_entry = taskflow_.emplace([&]() -> tf::SmallVector<int,10> {
            if(running_)
                return start_entries_;
            else
                return {0};
        }).name("start_entry");
        start_entry.succeed(start_task);
        start_entry.precede(end_task);



        auto ConnectTimeStepEndToStart = [&] (int time_step_index) {
            auto& id_tasks = time_step_data_[time_step_index].component_id_to_task;
            auto seam_entry_name = fmt::format(seam_entry_format, "TS", time_step_index);
            auto seam_start_name = fmt::format(seam_start_format, "TS", time_step_index);
            auto seam_end_name = fmt::format(seam_end_format, "TS", time_step_index);

            auto seam_entry = CreateSeamEntryTask(time_step_index, seam_entry_name);
            auto seam_start = CreateLocalStartTask(time_step_index, seam_start_name);
            auto seam_end = taskflow_.emplace([&, time_step_index](){
//                if(time_domain_buffer_.GetTimeStepBuffer(time_step_index).GetEventType() == MessageType::Teardown){
//                    running_ = false;
//                }
            }).name( seam_end_name);

            seam_entry.precede(seam_start, end_task);
            seam_end.precede(seam_entry);
            for(auto& id_task : id_tasks ){
                if(id_task.second.num_dependents() == 0) {
                    seam_start.precede(id_task.second);
                }
                if(id_task.second.num_successors() == 0) {
                    seam_end.succeed(id_task.second);
                }
            }

            id_tasks.emplace(fmt::format(seam_start_format, "TS", ""), seam_start);
            id_tasks.emplace(fmt::format(seam_end_format, "TS", ""), seam_end);
            id_tasks.emplace(fmt::format(seam_entry_format, "TS", ""), seam_entry);
        };

        auto InterConnectTimeSteps = [&](const std::string& instance_id) {


            auto seam_entry_name = fmt::format(seam_entry_format, instance_id, "");
            auto seam_start_name = fmt::format(seam_start_format, instance_id, "");

            auto seam_entry = CreateSeamEntryTask(0, seam_entry_name);
            auto seam_start = taskflow_.emplace([]() {}).name(seam_start_name);


            for (int time_step_index = 0; time_step_index < time_step_count_-1; ++time_step_index) {
                auto& id_tasks = time_step_data_[time_step_index].component_id_to_task;
                auto& next_id_tasks = time_step_data_[time_step_index+1].component_id_to_task;
                id_tasks.at( instance_id).precede(next_id_tasks.at( instance_id));
            }

            auto& first_tasks = time_step_data_.front().component_id_to_task;
            auto& last_tasks = time_step_data_.back().component_id_to_task;

            seam_entry.precede(seam_start, end_task);
            seam_start.precede(first_tasks.at(instance_id));
            last_tasks.at( instance_id).precede(seam_entry);
            first_tasks.emplace(seam_start_name, seam_start);
            first_tasks.emplace(seam_entry_name, seam_entry);
        };

        auto AddStartTask = [&](std::string instance_id) {
            auto& first_tasks = time_step_data_.front().component_id_to_task;
            auto seam_entry_name = fmt::format(seam_entry_format, instance_id, "");
            start_entry.precede(first_tasks.at(seam_entry_name));
            start_entries_.push_back(start_entries_.size()+1);
        };

        for (int time_step_index = 0; time_step_index < time_step_count_; ++time_step_index) {
            ConnectTimeStepEndToStart(time_step_index);

            auto& id_tasks = time_step_data_[time_step_index].component_id_to_task;
            //AddStartTask("TS");
            auto seam_entry_name = fmt::format(seam_entry_format, "TS", "");
            start_entry.precede(id_tasks.at(seam_entry_name));
            start_entries_.push_back(start_entries_.size()+1);
        }

        for (const auto& component : components_) {
            std::shared_ptr<pattern::instance::PatternInstance> pattern_instance = component.first;
            if (!pattern_instance) {
                continue;
            }

            if(pattern_instance->getConcurrency() == 1){
                InterConnectTimeSteps(pattern_instance->instance_id);
                AddStartTask(pattern_instance->instance_id);
            }

        }
        InterConnectTimeSteps("TIME_STEP_END");
        AddStartTask("TIME_STEP_END");

    }

    void TaskFlowTimeDomain::GlobalTaskflowStart() {
        SPDLOG_TRACE("Start time domain {0}", time_domain_);
        for (int time_step_index = 0; time_step_index < time_step_count_;++time_step_index ) {
            time_domain_buffer_.GetTimeStepBuffer(time_step_index).ResetLock();
        }
        taskflow_started_.notify();
    }

    void TaskFlowTimeDomain::GlobalTaskflowEnd() {
        SPDLOG_TRACE("End time domain {0}", time_domain_);
    }

    tf::Task TaskFlowTimeDomain::CreateLocalStartTask(int time_step_index, std::string name) {
        return taskflow_.emplace([&, time_step_index]() mutable {
            SPDLOG_TRACE("Start Time Step {0}", time_step_index);

//            for (int i = 0; i < time_step_count_; ++i) {
//                SPDLOG_TRACE("running : ts   {0} : {1}", running_taskflows_[i], running_timestamps_[i].time_since_epoch().count());
//            }

            //auto& time_step_buffer = time_domain_buffer_.GetTimeStepBuffer(time_step_index);

        }).name(name);
    }

    tf::Task TaskFlowTimeDomain::CreateLocalEndTask(int time_step_index, std::string name) {
        return taskflow_.emplace([&, time_step_index]() mutable {
            SPDLOG_TRACE("Finished Time Step: {0}", time_step_index);
            for (int i = 0; i < time_step_count_; ++i) {
                SPDLOG_TRACE("running : ts   {0} : {1}", running_taskflows_[i], running_timestamps_[i].time_since_epoch().count());
            }
            switch (time_domain_buffer_.GetTimeStepBuffer(time_step_index).GetEventType()) {
                case MessageType::Configure:
                    configure_finished_.SetInit(true);
                    break;
                case MessageType::Start:
                    start_finished_.SetInit(true);
                    break;
                default:
                    break;
            }
            FreeTimeStep(time_step_index);
        }).name(name);
    }

    tf::Task TaskFlowTimeDomain::CreateSeamEntryTask(int time_step_index, const std::string &seam_entry_name) {
        return taskflow_.emplace([&,time_step_index, seam_entry_name]() {
            if(time_domain_buffer_.GetTimeStepBuffer(time_step_index).GetEventType() == MessageType::DataflowStop){
                SPDLOG_TRACE("Dataflow stop in seam entry: {0}", seam_entry_name);
                return 1;
            } else {
                return 0;
            }
//                if(running_)
//                    return 0;
//                else
//                    return 1;
        }).name(seam_entry_name);
    }


}