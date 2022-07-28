/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "CudaGraphBuilder.h"
#include "traact/component/CudaComponent.h"
#include "TraactCudaTask.h"
namespace traact::dataflow {
CudaGraphBuilder::CudaGraphBuilder(GraphBuilderConfig &config) : GraphBuilder(config) {}

void CudaGraphBuilder::buildGraph(TraactGraph &traact_task_flow) {
    createTasks(traact_task_flow);

    createDependencies(traact_task_flow);
}
void CudaGraphBuilder::createTasks(TraactGraph &traact_task_flow) {

    std::map<std::string, std::map<std::string, std::shared_ptr<component::CudaComponent > > > cuda_graph_to_component;

    for (const auto &component : components_) {

        std::shared_ptr<pattern::instance::PatternInstance> pattern_instance = component.first;
        auto instance_id = component.first->instance_id;

        auto gpu_component =
            std::dynamic_pointer_cast<component::CudaComponent, component::Component>(component.second);

        if (gpu_component) {
            std::string cuda_graph = gpu_component->getCudaGraphName();
            cuda_graph_to_component[cuda_graph].emplace(instance_id, gpu_component);
        }
    }

    for(auto& [graph_name, components] : cuda_graph_to_component) {
        traact_task_flow.tasks.emplace(graph_name, std::make_shared<TraactCudaTask>(graph_name, components));
    }
}
void CudaGraphBuilder::createDependencies(TraactGraph &traact_task_flow) {
    for (const auto &component : components_) {
        auto instance_id = component.first->instance_id;
        updateComponentSuccessorsForGpu(traact_task_flow, instance_id);
    }
}

void CudaGraphBuilder::updateComponentSuccessorsForGpu(TraactGraph &traact_task_flow, const std::string& instance_id) {
    auto self_graph = getCudaGraph(traact_task_flow, instance_id);

    if (self_graph.has_value()) {
        auto cuda_graph_name = self_graph.value();
        auto cuda_graph_task = std::dynamic_pointer_cast<TraactCudaTask>(traact_task_flow.tasks.at(cuda_graph_name));

        auto component_task = std::dynamic_pointer_cast<TraactCudaTask>(traact_task_flow.tasks.at(instance_id));


        for (const auto &successor : component_task->successors()) {
            auto successor_cuda_graph = getCudaGraph(traact_task_flow, successor->getId());
            if (successor_cuda_graph.has_value()) {
                if (successor_cuda_graph.value() == cuda_graph_name) {
                    component_task->addInternalSuccessor(instance_id, successor->getId());
                } else {
                    cuda_graph_task->precede(successor);
                }
            } else {
                cuda_graph_task->precede(successor);
            }
        }

        cuda_graph_task->succeed(component_task);
    }
}

std::optional<std::string> CudaGraphBuilder::getCudaGraph(TraactGraph &traact_task_flow,
                                                          const std::basic_string<char> &instance_id) {
    auto component = std::find_if(components_.begin(), components_.end(), [&instance_id](const auto &value) {
        return value.first->instance_id == instance_id;
    });
    if (component != components_.end()) {
        auto gpu_comp_tmp =
            std::dynamic_pointer_cast<component::CudaComponent, component::Component>(component->second);

        if (gpu_comp_tmp) {
            return gpu_comp_tmp->getCudaGraphName();
        } else {
            return {};
        }

    } else {

        if (isCudaGraph(traact_task_flow, instance_id)) {
            // component is cuda graph
            return {instance_id};
        } else {
            throw std::invalid_argument(fmt::format("unknown component {0} in components list", instance_id));
        }

    }
}

bool CudaGraphBuilder::isCudaComponent(TraactGraph &traact_task_flow, const std::string &instance_id) {
    auto component = std::find_if(components_.begin(), components_.end(), [&instance_id](const auto &value) {
        return value.first->instance_id == instance_id;
    });
    if (component != components_.end()) {
        auto gpu_comp_tmp =
            std::dynamic_pointer_cast<component::CudaComponent, component::Component>(component->second);

        return gpu_comp_tmp != nullptr;


    } else {

        if (isCudaGraph(traact_task_flow, instance_id)) {
            // component is cuda graph
            return false;
        } else {
            throw std::invalid_argument(fmt::format("unknown component {0} in components list", instance_id));
        }

    }
}
bool CudaGraphBuilder::isCudaGraph(const TraactGraph &traact_task_flow, const std::basic_string<char> &instance_id) {
    auto component_find = traact_task_flow.tasks.find(instance_id);
    if(component_find != traact_task_flow.tasks.end()){
        auto cuda_graph = std::dynamic_pointer_cast<TraactCudaTask>(component_find->second);
        return cuda_graph != nullptr;
    } else {
        return false;
    }

}




} // traact