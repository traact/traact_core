/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWGRAPH_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWGRAPH_H_

#include "traact/component/Component.h"
#include "traact/buffer/DataBufferFactory.h"
#include "traact/traact.h"
#include <taskflow/taskflow.hpp>
#include "TaskFlowTimeDomain.h"
#include "traact/util/Semaphore.h"

namespace traact::dataflow {

class TaskFlowGraph {
 public:
    TaskFlowGraph(buffer::DataBufferFactoryPtr generic_factory_objects,
                  DefaultComponentGraphPtr component_graph,
                  component::Component::SourceFinishedCallback callback);

    void init();
    void start();
    void stop();
    void teardown();

 private:

    buffer::DataBufferFactoryPtr generic_factory_objects_;
    DefaultComponentGraphPtr component_graph_;

    std::atomic_flag source_finished_ = ATOMIC_FLAG_INIT;
    component::Component::SourceFinishedCallback source_finished_callback_;
    void masterSourceFinished();

    tf::Executor executor_;
    std::vector<std::unique_ptr<TaskFlowTimeDomain>> task_flow_time_domains_;

};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWGRAPH_H_
