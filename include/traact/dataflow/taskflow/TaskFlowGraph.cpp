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

#include "TaskFlowGraph.h"
#include "TaskFlowTaskFunctions.h"
#include <utility>

namespace traact::dataflow {



    void TaskFlowGraph::Init() {


        for(const auto& time_domain : component_graph_->GetTimeDomains()){
            task_flow_time_domains_.emplace_back(std::make_shared<TaskFlowTimeDomain>(generic_factory_objects_, component_graph_, time_domain, source_finished_callback));

        }

        for(auto& task_flow_time_domain : task_flow_time_domains_){
            task_flow_time_domain->Init();
        }



    }



    void TaskFlowGraph::Start() {
        for(auto& task_flow_time_domain : task_flow_time_domains_){
            task_flow_time_domain->Start();
        }
    }

    void TaskFlowGraph::Stop() {
        for(auto& task_flow_time_domain : task_flow_time_domains_){
            task_flow_time_domain->Stop();
        }

    }

    void TaskFlowGraph::Teardown() {

    }

    void TaskFlowGraph::MasterSourceFinished() {
        // first finished call ends playback
        if(source_finished_.test_and_set()) {
            return;
        }

        source_finished_callback();
    }

    TaskFlowGraph::TaskFlowGraph(std::set<buffer::BufferFactory::Ptr> genericFactoryObjects,
                                 DefaultComponentGraphPtr componentGraph,
                                 const component::Component::SourceFinishedCallback &callback) : generic_factory_objects_(std::move(
            genericFactoryObjects)), component_graph_(std::move(componentGraph)), source_finished_callback(callback) {

    }


} // traact