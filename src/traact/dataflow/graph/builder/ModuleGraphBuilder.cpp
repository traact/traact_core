/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include <taskflow/core/task.hpp>
#include "ModuleGraphBuilder.h"
#include "traact/dataflow/graph/task/ControlFlowTask.h"
#include "traact/dataflow/graph/task/ModuleTask.h"

namespace traact::dataflow {

static const constexpr char *kModuleStartName{"MODULE_START_{0}"};
static const constexpr char *kModuleMiddleName{"MODULE_MIDDLE_{0}"};
static const constexpr char *kModuleEndName{"MODULE_END_{0}"};

ModuleGraphBuilder::ModuleGraphBuilder(GraphBuilderConfig &config) : GraphBuilder(config) {}

void ModuleGraphBuilder::buildGraph(TraactGraph &traact_task_flow) {
    /**
    * the module instance is its own task in the task flow. depending on the configuration
    * of the module (component type of associated components) the module task is executed
     * either in between the components or at the end.
    */
    extractModules(traact_task_flow);
    createTasksAndDependencies(traact_task_flow);
}

void ModuleGraphBuilder::extractModules(TraactGraph &traact_task_flow) {

    for (const auto &component : components_) {

        auto instance_id = component.first->instance_id;

        auto module_comp_tmp =
            std::dynamic_pointer_cast<component::ModuleComponent, component::Component>(component.second);

        if (module_comp_tmp) {
            std::string module_key = module_comp_tmp->getModuleKey();
            auto &task = traact_task_flow.tasks.at(instance_id);

            switch (component.first->getComponentType(time_domain_)) {
                case component::ComponentType::ASYNC_SINK:
                case component::ComponentType::SYNC_SINK:
                case component::ComponentType::ASYNC_SOURCE:
                case component::ComponentType::INTERNAL_SYNC_SOURCE:
                case component::ComponentType::SYNC_FUNCTIONAL: {
                    component_modules_[module_key].begin_components.emplace_back(task);
                    break;
                }
                case component::ComponentType::SYNC_SOURCE: {
                    component_modules_[module_key].end_components.emplace_back(task);
                    break;
                }
                case component::ComponentType::ASYNC_FUNCTIONAL:
                case component::ComponentType::INVALID:
                default:throw std::invalid_argument(fmt::format("invalid component type of {0}", instance_id));
            }

            component::Module::Ptr module = module_map_[module_key];

            if (module) {
                SPDLOG_INFO("Graph: {0} Component: {1} module exists",
                            config_.component_graph->getName(),
                            component.first->getName());
            } else {
                SPDLOG_INFO("Graph: {0} Component: {1} create new module",
                            config_.component_graph->getName(),
                            component.first->getName());
                module = module_comp_tmp->instantiateModule();
                module_map_[module_key] = module;
            }
            module_comp_tmp->setModule(module);

        }
    }

}
void ModuleGraphBuilder::createTasksAndDependencies(TraactGraph &traact_task_flow) {
    for (const auto &[module_key, components] : component_modules_) {
        auto module_instance = module_map_.at(module_key);

        if(!module_instance->useConstraints()){
            continue;
        }

        bool use_middle = !components.begin_components.empty() && !components.end_components.empty();

        std::string start_task_name = fmt::format(kModuleStartName, module_key);
        std::string middle_task_name = fmt::format(kModuleMiddleName, module_key);
        std::string end_task_name = fmt::format(kModuleEndName, module_key);

        auto start_task = std::make_shared<ControlFlowTask>(start_task_name);
        traact_task_flow.tasks.emplace(start_task_name, start_task);

        TraactTask::SharedPtr end_task;
        TraactTask::SharedPtr begin_components_end_task;
        TraactTask::SharedPtr end_components_start_task;

        if (use_middle) {
            end_task = std::make_shared<ControlFlowTask>(end_task_name);
            traact_task_flow.tasks.emplace(end_task_name, end_task);

            auto middle_task = std::make_shared<ModuleTask>(middle_task_name, module_instance);

            traact_task_flow.tasks.emplace(middle_task_name, middle_task);

            begin_components_end_task = middle_task;
            end_components_start_task = middle_task;

        } else {
            end_task = std::make_shared<ModuleTask>(end_task_name, module_instance);
            traact_task_flow.tasks.emplace(end_task_name, end_task);

            begin_components_end_task = end_task;
            end_components_start_task = start_task;

        }

        start_task->succeedInterTimeStep(end_task);

        for (const auto &sub_task : components.begin_components) {
            start_task->precede(sub_task);
            begin_components_end_task->succeed(sub_task);
            for (const auto &successor : sub_task->successors()) {
                begin_components_end_task->precede(successor);
            }
        }

        for (const auto &sub_task : components.end_components) {
            end_components_start_task->precede(sub_task);
            end_task->succeed(sub_task);
            for (const auto &successor : sub_task->successors()) {
                end_task->precede(successor);
            }
        }
    }
}

} // traact