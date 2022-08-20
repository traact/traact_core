/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TRAACTGRAPH_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TRAACTGRAPH_H_

#include <memory>
#include "task/TraactTask.h"
#include "traact/buffer/TimeDomainBuffer.h"
#include "traact/dataflow/state/DataflowState.h"

namespace traact::dataflow {

struct GraphBuilderConfig {
    using BufferCallback = std::function<std::future<buffer::SourceComponentBuffer *>(Timestamp)>;
    using CreateBufferCallback = std::function<BufferCallback(int)>;


    GraphBuilderConfig(const buffer::TimeDomainManagerConfig &t_config,
                       const component::ComponentGraph::Ptr &t_component_graph,
                       const std::shared_ptr<buffer::TimeDomainBuffer> &t_time_domain_buffer,
                       const CreateBufferCallback &t_request_buffer_callback,
                       const std::function<void(void)> &t_request_exit_callback,
                       const DataflowState::SharedPtr &t_dataflow_state);
    buffer::TimeDomainManagerConfig config;
    component::ComponentGraph::Ptr  component_graph;
    std::shared_ptr<buffer::TimeDomainBuffer> time_domain_buffer;

    CreateBufferCallback create_buffer_callback;
    std::function<void(void)> request_exit_callback;
    DataflowState::SharedPtr dataflow_state;
};

struct TraactGraph {
 public:
    using SharedPtr = std::shared_ptr<TraactGraph>;

    std::map<std::string, TraactTask::SharedPtr> tasks;
    std::vector<std::map<std::string, TraactTaskData::SharedPtr>> task_data;
    std::shared_ptr<DataflowState> dataflow_state;
    size_t getTimeStepCount();

};
}



#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TRAACTGRAPH_H_
