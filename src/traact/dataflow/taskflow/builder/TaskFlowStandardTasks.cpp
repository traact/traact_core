/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TaskFlowStandardTasks.h"

#include "tasks/TaskFlowComponentTask.h"
#include "tasks/TaskFlowSourceTask.h"
#include "tasks/TaskFlowModuleTask.h"

namespace traact::dataflow {

TaskFlowStandardTasks::TaskFlowStandardTasks(TraactTaskFlowGraph &task_flow_graph) : TaskFlowGraphBuilder(
    task_flow_graph) {}
void TaskFlowStandardTasks::buildGraph(const TraactGraph::SharedPtr &traact_graph) {
    task_flow_graph_.task_flow_tasks.resize(traact_graph->getTimeStepCount());

    for(auto time_step = 0; time_step < traact_graph->getTimeStepCount(); ++time_step){
        createTimeStep(time_step, traact_graph);
    }

}
void TaskFlowStandardTasks::createTimeStep(int time_step, const TraactGraph::SharedPtr &traact_graph) {
    auto& data = traact_graph->task_data.at(time_step);
    auto& task_flow_tasks = task_flow_graph_.task_flow_tasks.at(time_step);
    for(auto& [id, traact_task] : traact_graph->tasks) {
        auto data_pointer = data.at(id);

        tf::Task task;

        auto component_task = std::dynamic_pointer_cast<ComponentTask>(traact_task);
        if(component_task){
            task = createTask(component_task, data_pointer);
        }

        auto source_task = std::dynamic_pointer_cast<SourceTask>(traact_task);
        if(source_task){
            task = createTask(source_task, data_pointer);
        }

        auto module_task = std::dynamic_pointer_cast<ModuleTask>(traact_task);
        if(module_task){
            task = createTask(module_task, data_pointer);
        }

        auto control_task = std::dynamic_pointer_cast<ControlFlowTask>(traact_task);
        if(control_task){
            task = createTask(control_task, data_pointer);
        }

        task_flow_tasks.emplace(id, task);
    }
}
tf::Task TaskFlowStandardTasks::createTask(std::shared_ptr<traact::dataflow::ComponentTask> task,
                                           std::shared_ptr<TraactTaskData> data) {
    return task_flow_graph_.taskflow->emplace( [task = taskflow::TaskFlowComponentTask(task, data)]() mutable {
        task.process();
    });
}
tf::Task TaskFlowStandardTasks::createTask(std::shared_ptr<SourceTask> task, std::shared_ptr<TraactTaskData> data) {
    return task_flow_graph_.taskflow->emplace( [task = taskflow::TaskFlowSourceTask(task, data)]() mutable {
        task.process();
    });
}
tf::Task TaskFlowStandardTasks::createTask(std::shared_ptr<ModuleTask> task, std::shared_ptr<TraactTaskData> data) {
    return task_flow_graph_.taskflow->emplace( [task = taskflow::TaskFlowModuleTask(task, data)]() mutable {
        task.process();
    });
}
tf::Task TaskFlowStandardTasks::createTask(std::shared_ptr<ControlFlowTask> task, std::shared_ptr<TraactTaskData> data) {
    return task_flow_graph_.taskflow->emplace([](){

    });
}
}