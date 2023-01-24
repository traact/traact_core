/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TaskFlowTimeStep.h"
#include "traact/dataflow/graph/task/TraactTaskId.h"
namespace traact::dataflow {


TaskFlowTimeStep::TaskFlowTimeStep(TraactTaskFlowGraph &task_flow_graph) : TaskFlowGraphBuilder(task_flow_graph) {}


void TaskFlowTimeStep::buildGraph(const TraactGraph::SharedPtr &traact_graph) {
    for(int time_step_index=0; time_step_index<traact_graph->getTimeStepCount(); ++time_step_index){
        createStartStopTasks(*traact_graph, time_step_index);
    }
}


void TaskFlowTimeStep::createStartStopTasks(const TraactGraph &traact_graph, int time_step_index) {

    auto& task_flow_tasks = task_flow_graph_.task_flow_tasks.at(time_step_index);
    auto start_task = createLocalStartTask(time_step_index,  task_util::getTaskId(task_util::kControlFlow, task_util::kTimeStepStart, time_step_index));
    for (const auto &[id, traact_task] : traact_graph.tasks) {
        if(traact_task->isStartPoint()){
            auto task = task_flow_tasks.at(id);
            start_task.precede(task);
        }
    }
    task_flow_tasks.emplace(task_util::kTimeStepStart, start_task);


    auto end_task = createLocalEndTask(time_step_index, task_util::getTaskId(task_util::kControlFlow, task_util::kTimeStepEnd, time_step_index));
    for (const auto &[id, traact_task] : traact_graph.tasks) {
        if(traact_task->isEndPoint()){
            auto task = task_flow_tasks.at(id);
            end_task.succeed(task);
        }
    }
    task_flow_tasks.emplace(task_util::kTimeStepEnd, end_task);

}
tf::Task TaskFlowTimeStep::createLocalStartTask(int time_step_index, const std::string &name) {
    return task_flow_graph_.taskflow->emplace([scheduler = task_flow_graph_.scheduler, time_step_index]() mutable {
        SPDLOG_TRACE("TIME_STEP_START_{0}", time_step_index);
        scheduler->timeStepStart(time_step_index);
    }).name(name);
}
tf::Task TaskFlowTimeStep::createLocalEndTask(int time_step_index, const std::string &name) {
    return task_flow_graph_.taskflow->emplace([scheduler = task_flow_graph_.scheduler, time_step_index]() mutable {
        SPDLOG_TRACE("TIME_STEP_END_{0}", time_step_index);
        scheduler->timeStepEnded(time_step_index);

    }).name(name);
}

} // traact