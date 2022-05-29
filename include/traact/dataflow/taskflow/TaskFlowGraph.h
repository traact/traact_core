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

#ifndef TRAACTMULTI_TRAACT_DATAFLOW_TASKFLOWGRAPH_H
#define TRAACTMULTI_TRAACT_DATAFLOW_TASKFLOWGRAPH_H

#include <traact/component/Component.h>
#include <traact/buffer/BufferFactory.h>
#include <traact/traact.h>
#include <taskflow/taskflow.hpp>
#include "TaskFlowTimeDomain.h"
#include "traact/util/Semaphore.h"

namespace traact::dataflow {

    class TaskFlowGraph {
    public:
        TaskFlowGraph(std::set<buffer::BufferFactory::Ptr> genericFactoryObjects,
                      DefaultComponentGraphPtr componentGraph,
                      const component::Component::SourceFinishedCallback &callback);

        void Init();
        void Start();
        void Stop();
        void Teardown();

    private:

        std::set<buffer::BufferFactory::Ptr> generic_factory_objects_;
        DefaultComponentGraphPtr component_graph_;

        std::atomic_flag source_finished_{ATOMIC_FLAG_INIT};
        component::Component::SourceFinishedCallback source_finished_callback;
        void MasterSourceFinished();

        tf::Executor executor_;
        std::vector<std::shared_ptr<TaskFlowTimeDomain>> task_flow_time_domains_;

    };

} // traact

#endif //TRAACTMULTI_TRAACT_DATAFLOW_TASKFLOWGRAPH_H
