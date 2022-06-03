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

#ifndef TRAACTMULTI_TASKFLOWTASKFUNCTIONS_H
#define TRAACTMULTI_TASKFLOWTASKFUNCTIONS_H

#include <traact/buffer/ComponentBuffer.h>
#include "traact/component/Component.h"

namespace traact::dataflow {

    struct ComponentData {

        ComponentData(buffer::TimeStepBuffer &time_step_buffer, buffer::ComponentBuffer &buffer,
                      component::Component &component, int component_index, nlohmann::json &component_parameter,
                      std::atomic<bool> &running) : time_step_buffer(time_step_buffer), buffer(buffer),
                                                    component(component), component_index(component_index),
                                                    component_parameter(component_parameter), running(running) {}

        buffer::TimeStepBuffer& time_step_buffer;
        buffer::ComponentBuffer& buffer;
        component::Component& component;
        int component_index;
        nlohmann::json& component_parameter;
        std::atomic<bool>& running;

        //tf::SmallVector<bool*> valid_input{};
        std::vector<bool*> valid_input{};
        bool valid_output{false};


    };

    struct TimeStepData {
        std::map<std::string, ComponentData> component_data;
        std::map<std::string, tf::Task> component_id_to_task;
    };

    inline void TaskSource(ComponentData & local_data) {

        SPDLOG_TRACE("TaskSource: start waiting, Component {0}", local_data.component_index);
        auto lock = local_data.time_step_buffer.GetSourceLock(local_data.component_index);
        auto status = lock.wait_for(std::chrono::milliseconds(100));
        while (status != std::future_status::ready && local_data.running){
            status = lock.wait_for(std::chrono::milliseconds(100));
        }
        if(status != std::future_status::ready || !local_data.running){
            local_data.valid_output = false;
            return;
        }

        local_data.valid_output = lock.get();

        switch(local_data.time_step_buffer.GetEventType()){

            case MessageType::Configure:
                local_data.component.configure(local_data.component_parameter, nullptr);
                break;
            case MessageType::Start:
                local_data.component.start();
                break;
            case MessageType::Data:
                break;
            case MessageType::Stop:
                local_data.component.stop();
                break;
            case MessageType::Teardown:
                local_data.component.teardown();
                break;
            case MessageType::DataflowNoOp:
            case MessageType::DataflowStop:
                break;
            default:
            case MessageType::Invalid:
                assert(!"Invalid MessageType");
                break;
        }



        SPDLOG_TRACE("TaskSource: finished, {0} ts {1}", local_data.component_index, local_data.time_step_buffer.GetTimestamp().time_since_epoch().count());
    }


    inline void TaskGenericComponent(ComponentData& local_data){
        SPDLOG_TRACE("Component {0}: start ts: {1}", local_data.component.getName(), local_data.buffer.GetTimestamp().time_since_epoch().count());



        switch(local_data.time_step_buffer.GetEventType()){
            case MessageType::Configure:
                local_data.component.configure(local_data.component_parameter, nullptr);
                break;
            case MessageType::Start:
                local_data.component.start();
                break;
            case MessageType::Data:{
                bool valid_input = true;
                for(auto vp : local_data.valid_input)
                    valid_input &= *vp;

                if(valid_input)
                    local_data.valid_output = local_data.component.processTimePoint(local_data.buffer);
                else{
                    local_data.component.invalidTimePoint(local_data.buffer.GetTimestamp(),0);
                    local_data.valid_output = false;
                }

                break;
            }
            case MessageType::Stop:
                local_data.component.stop();
                break;
            case MessageType::Teardown:
                local_data.component.teardown();
                break;
            case MessageType::DataflowNoOp:
            case MessageType::DataflowStop:
                 break;
            default:
            case MessageType::Invalid:
                assert(!"Invalid MessageType");
                break;
        }



        SPDLOG_TRACE("{0}: finished ts {1}", local_data.component.getName(), local_data.buffer.GetTimestamp().time_since_epoch().count());
    }
}



#endif //TRAACTMULTI_TASKFLOWTASKFUNCTIONS_H
