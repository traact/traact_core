/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_BUILDER_COMPONENTGRAPHBUILDER_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_BUILDER_COMPONENTGRAPHBUILDER_H_

#include "GraphBuilder.h"
#include "traact/dataflow/taskflow/TaskFlowInFlowScheduler.h"
namespace traact::dataflow {

class ComponentGraphBuilder : public GraphBuilder{

 public:
    ComponentGraphBuilder(GraphBuilderConfig &config);
    void buildGraph(TraactGraph &traact_task_flow) override;

 private:
    TraactTask::SharedPtr findSyncSourceStartPoint(TraactGraph &traact_task_flow,
                                                   const std::pair<component::ComponentGraph::PatternPtr,
                                                                   component::ComponentGraph::ComponentPtr> &component);
    void createTasks(TraactGraph &traact_task_flow) const;
    void createDependencies(TraactGraph &traact_task_flow);
    void prepareComponents(TraactGraph &traact_task_flow);
    void setRequestCallback(const std::shared_ptr<component::Component> &component,
                            const std::string &instance_id) const;
    TraactTask::SharedPtr createTask(int time_domain,
                                     const std::shared_ptr<pattern::instance::PatternInstance> &pattern,
                                     const std::shared_ptr<component::Component> &component) const;
};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_BUILDER_COMPONENTGRAPHBUILDER_H_
