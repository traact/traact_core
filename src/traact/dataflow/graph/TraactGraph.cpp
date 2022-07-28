/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TraactGraph.h"

namespace traact::dataflow {
GraphBuilderConfig::GraphBuilderConfig(const buffer::TimeDomainManagerConfig &config,
                                       const component::ComponentGraph::Ptr &component_graph,
                                       const std::shared_ptr<buffer::TimeDomainBuffer> &time_domain_buffer,
                                       const CreateBufferCallback &request_buffer_callback,
                                       const std::function<void(void)> &request_exit_callback)
    : config(config),
      component_graph(component_graph),
      time_domain_buffer(time_domain_buffer),
      request_exit_callback(request_exit_callback), create_buffer_callback(request_buffer_callback) {}

size_t TraactGraph::getTimeStepCount() {
    return task_data.size();
}
} // traact