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
#include "TaskFlowInFlowScheduler.h"

// GPU
#include "traact/component/CudaComponent.h"
//

#include "traact/dataflow/graph/TraactGraphBuilder.h"
#include "TraactTaskFlowBuilder.h"

namespace traact::dataflow {

using DefaultScheduler = TaskFlowInFlowScheduler;

class TaskFlowTimeDomain {
 public:
    TaskFlowTimeDomain(int time_domain,
                       component::ComponentGraph::Ptr component_graph,
                       buffer::DataBufferFactoryPtr buffer_factory,
                       component::Component::SourceFinishedCallback callback,
                       DataflowState::SharedPtr dataflow_state);



    void init();
    void start();
    void stop();
    void parameterChanged(const std::string &instance_id);

    void printState();

 private:
    const int time_domain_;
    component::ComponentGraph::Ptr  component_graph_;
    buffer::DataBufferFactoryPtr generic_factory_objects_;
    component::Component::SourceFinishedCallback source_finished_callback_;


    // data for taskflow
    std::unique_ptr<TraactGraphBuilder> graph_builder_;
    std::unique_ptr<TraactTaskFlowBuilder> task_flow_builder_;


    tf::Taskflow taskflow_;

    std::shared_ptr<buffer::TimeDomainBuffer> time_domain_buffer_;
    std::atomic<bool> running_{};

    int time_step_count_{0};

    bool stop_called_{false};
    // time domain data
    std::set<component::ComponentGraph::PatternComponentPair> components_;
    traact::buffer::TimeDomainManagerConfig time_domain_config_;

    std::unique_ptr<DefaultScheduler> scheduler_;


    void createBuffer();

};
}

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWTIMEDOMAIN_H_
