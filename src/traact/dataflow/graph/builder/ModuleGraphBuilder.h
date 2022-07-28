/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_BUILDER_MODULEGRAPHBUILDER_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_BUILDER_MODULEGRAPHBUILDER_H_

#include "GraphBuilder.h"
#include "traact/component/ModuleComponent.h"

namespace traact::dataflow {


class ModuleGraphBuilder : public GraphBuilder {
 public:
    ModuleGraphBuilder(GraphBuilderConfig &config);
    void buildGraph(TraactGraph &traact_task_flow) override;
 private:
    struct ModuleTaskData {
        std::string module_key;
        std::vector<TraactTask::SharedPtr> begin_components;
        std::vector<TraactTask::SharedPtr> end_components;
    };
    void extractModules(TraactGraph &traact_task_flow);
    void createTasksAndDependencies(TraactGraph &traact_task_flow);

    std::map<std::string, component::Module::Ptr> module_map_;
    std::map<std::string, ModuleTaskData> component_modules_;
};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_BUILDER_MODULEGRAPHBUILDER_H_
