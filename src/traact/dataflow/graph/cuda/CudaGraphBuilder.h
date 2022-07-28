/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_CUDA_CUDAGRAPHBUILDER_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_CUDA_CUDAGRAPHBUILDER_H_

#include "traact/dataflow/graph/builder/GraphBuilder.h"
#include "traact/component/CudaComponent.h"

namespace traact::dataflow {

class CudaGraphBuilder : public GraphBuilder{
 public:
    CudaGraphBuilder(GraphBuilderConfig &config);
    void buildGraph(TraactGraph &traact_task_flow) override;

 private:



    void updateComponentSuccessorsForGpu(TraactGraph &traact_task_flow, const std::string& instance_id);

    std::optional<std::string> getCudaGraph(TraactGraph &traact_task_flow,
                                            const std::basic_string<char> &instance_id);
    bool isCudaGraph(const TraactGraph &traact_task_flow, const std::basic_string<char> &instance_id);
    bool isCudaComponent(TraactGraph &traact_task_flow, const std::string &instance_id);

    void createGpuTimeStepTasks(const int time_step_index);


    void createTasks(TraactGraph &traact_task_flow);
    void createDependencies(TraactGraph &traact_task_flow);
};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_CUDA_CUDAGRAPHBUILDER_H_
