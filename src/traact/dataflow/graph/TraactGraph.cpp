/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TraactGraph.h"

namespace traact::dataflow {
GraphBuilderConfig::GraphBuilderConfig(const buffer::TimeDomainManagerConfig &t_config,
                                       const component::ComponentGraph::Ptr &t_component_graph,
                                       const std::shared_ptr<buffer::TimeDomainBuffer> &t_time_domain_buffer,
                                       const CreateBufferCallback &t_request_buffer_callback,
                                       const std::function<void(void)> &t_request_exit_callback,
                                       const DataflowState::SharedPtr &t_dataflow_state)
    : config(t_config),
      component_graph(t_component_graph),
      time_domain_buffer(t_time_domain_buffer),
      request_exit_callback(t_request_exit_callback), create_buffer_callback(t_request_buffer_callback),
      dataflow_state(t_dataflow_state) {}

size_t TraactGraph::getTimeStepCount() {
    return task_data.size();
}
} // traact