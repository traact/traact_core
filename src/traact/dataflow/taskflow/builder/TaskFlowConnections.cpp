/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TaskFlowConnections.h"
#include "TaskFlowTimeStep.h"

namespace traact::dataflow {

static const constexpr char *kSeamEntryFormat{"{0}_SEAM_{1}"};
static const constexpr char *kSeamStartFormat{"{0}_SEAM_START_{1}"};
static const constexpr char *kSeamEndFormat{"{0}_SEAM_END_{1}"};



std::string getTaskName(const int time_step_index,
                                            const std::string &instance_id) {
    return fmt::format("{0}_task{1}",
                       instance_id,
                       time_step_index);
}




TaskFlowConnections::TaskFlowConnections(TraactTaskFlowGraph &task_flow_graph)
    : TaskFlowGraphBuilder(task_flow_graph) {}


void TaskFlowConnections::buildGraph(const TraactGraph::SharedPtr &traact_graph) {
    for(auto time_step = 0; time_step < traact_graph->getTimeStepCount(); ++time_step){
        connectTimeStepComponents(time_step, traact_graph);
    }

    createInterTimeStepDependencies(traact_graph);
}

void TaskFlowConnections::connectTimeStepComponents(int time_step, const TraactGraph::SharedPtr &traact_graph) {

    auto& task_flow_tasks =  task_flow_graph_.task_flow_tasks.at(time_step);

    for (const auto &[id, task] : traact_graph->tasks) {
        auto& current_task = task_flow_tasks.at(id);
        for(const auto& successor : task->successors()){
            auto successor_task = task_flow_tasks.find(successor->getId());
            if(successor_task != task_flow_tasks.end()){
                current_task.precede(successor_task->second);
            } else{
                auto error = fmt::format("TaskFlowConnections, task {0}, could not find successor task {1}", id, successor->getId());
                SPDLOG_ERROR(error);
                throw std::logic_error(error);
            }

        }
    }

}


void TaskFlowConnections::createInterTimeStepDependencies(const TraactGraph::SharedPtr &traact_graph) {

    tf::Task start_task = createGlobalStartTask();
    tf::Task end_task = createGlobalEndTask();

    auto start_entry = task_flow_graph_.taskflow->emplace([&]() -> tf::SmallVector<int, kStartEntries> {
        SPDLOG_TRACE("start entry is running");
        return start_entries_;
    }).name("start_entry");
    start_entry.succeed(start_task);
    start_entry.precede(end_task);



    auto connect_time_step_end_to_start = [&](int time_step_index) {
        auto &id_tasks = task_flow_graph_.task_flow_tasks[time_step_index];
        auto seam_entry_name = fmt::format(kSeamEntryFormat, "TIME_STEP", time_step_index);
        auto seam_start_name = fmt::format(kSeamStartFormat, "TIME_STEP", time_step_index);
        auto seam_end_name = fmt::format(kSeamEndFormat, "TIME_STEP", time_step_index);

        auto seam_entry = createSeamEntryTask(time_step_index, seam_entry_name);
        auto seam_start = task_flow_graph_.taskflow->emplace([]() {}).name(seam_start_name);
        auto seam_end = task_flow_graph_.taskflow->emplace([]() {}).name(seam_end_name);

        auto &time_step_start = id_tasks.at(kTimeStepStart);
        auto &time_step_end = id_tasks.at(kTimeStepEnd);
        time_step_end.precede(seam_entry);
        seam_entry.precede(seam_start, seam_end);
        seam_start.precede(time_step_start);

        start_entry.precede(seam_entry);
        start_entries_.push_back(static_cast<int>(start_entries_.size()) + 1);
    };

    auto connect_end_to_next_start = [&](std::string start_name, std::string end_name) {

        auto seam_entry_name = fmt::format(kSeamEntryFormat, start_name, 0);
        auto seam_start_name = fmt::format(kSeamStartFormat, start_name, 0);
        auto seam_end_name = fmt::format(kSeamEndFormat, start_name, 0);

        auto seam_entry = createSeamEntryTask(0, seam_entry_name);
        auto seam_start = task_flow_graph_.taskflow->emplace([]() {}).name(seam_start_name);
        auto seam_end = task_flow_graph_.taskflow->emplace([]() {}).name(seam_end_name);

        for (int current_time_step_index = 0; current_time_step_index < traact_graph->getTimeStepCount() - 1;
             ++current_time_step_index) {
            int next_time_step_index = current_time_step_index + 1;
            auto &current_end_tasks = task_flow_graph_.task_flow_tasks[current_time_step_index].at(end_name);
            auto &next_start_tasks = task_flow_graph_.task_flow_tasks[current_time_step_index].at(start_name);
            current_end_tasks.precede(next_start_tasks);

            auto time_step_end_task = task_flow_graph_.task_flow_tasks[current_time_step_index].at(kTimeStepEnd);
            current_end_tasks.precede(time_step_end_task);
        }

        auto &last_end_task = task_flow_graph_.task_flow_tasks.back().at(end_name);
        auto &fist_start_task = task_flow_graph_.task_flow_tasks.front().at(start_name);
        auto last_time_step_end_task = task_flow_graph_.task_flow_tasks.back().at(kTimeStepEnd);

        last_end_task.precede(seam_entry);
        seam_entry.precede(seam_start, seam_end);
        seam_start.precede(fist_start_task);

        last_end_task.precede(last_time_step_end_task);

        start_entry.precede(seam_entry);
        start_entries_.push_back(static_cast<int>(start_entries_.size()) + 1);
    };

    auto inter_connect_time_steps = [&](const std::string &instance_id) {

        auto seam_entry_name = fmt::format(kSeamEntryFormat, instance_id, 0);
        auto seam_start_name = fmt::format(kSeamStartFormat, instance_id, 0);
        auto seam_end_name = fmt::format(kSeamEndFormat, instance_id, 0);

        auto seam_entry = createSeamEntryTask(0, seam_entry_name);
        auto seam_start = task_flow_graph_.taskflow->emplace([]() {}).name(seam_start_name);
        auto seam_end = task_flow_graph_.taskflow->emplace([]() {}).name(seam_end_name);

        for (int time_step_index = 0; time_step_index < traact_graph->getTimeStepCount()-1; ++time_step_index) {
            auto &id_tasks = task_flow_graph_.task_flow_tasks[time_step_index];
            auto &next_id_tasks = task_flow_graph_.task_flow_tasks[time_step_index + 1];
            id_tasks.at(instance_id).precede(next_id_tasks.at(instance_id));
        }

        auto &first_tasks = task_flow_graph_.task_flow_tasks.front();
        auto &last_tasks = task_flow_graph_.task_flow_tasks.back();

        auto &first_time_step = first_tasks.at(instance_id);
        auto &last_time_step = last_tasks.at(instance_id);
        last_time_step.precede(seam_entry);
        seam_entry.precede(seam_start, seam_end);
        seam_start.precede(first_time_step);

        start_entry.precede(seam_entry);
        start_entries_.push_back(static_cast<int>(start_entries_.size()) + 1);
    };

    for (int time_step_index = 0; time_step_index < traact_graph->getTimeStepCount(); ++time_step_index) {
        connect_time_step_end_to_start(time_step_index);

    }

    inter_connect_time_steps(kTimeStepStart);
    inter_connect_time_steps(kTimeStepEnd);

    for (const auto &[id, current_task] : traact_graph->tasks) {

        for(const auto& successor_task : current_task->successorsInterTimeStep()) {
            auto current_id = current_task->getId();
            auto successor_id = successor_task->getId();
            if(current_id == successor_id){
                inter_connect_time_steps(current_id);
            } else {
                connect_end_to_next_start(current_id, successor_id);
            }
        }
    }


}

tf::Task TaskFlowConnections::createSeamEntryTask(int time_step_index, const std::string &seam_entry_name) {
    auto* buffer = &task_flow_graph_.time_domain_buffer->getTimeStepBuffer(time_step_index);
    return task_flow_graph_.taskflow->emplace([buffer, seam_entry_name]() {
        SPDLOG_TRACE("seam entry: {0}", seam_entry_name);
        if (buffer->getEventType() == EventType::DATAFLOW_STOP) {
            SPDLOG_TRACE("Dataflow stop in seam entry: {0}", seam_entry_name);
            return 1;
        } else {
            return 0;
        }
    }).name(seam_entry_name);
}

tf::Task TaskFlowConnections::createGlobalStartTask() {
    return task_flow_graph_.taskflow->emplace([scheduler = task_flow_graph_.scheduler, time_domain = task_flow_graph_.time_domain]() mutable {
        SPDLOG_TRACE("start time domain: {0}", time_domain);
        scheduler->globalTaskFlowStart();
    }).name("start");


}
tf::Task TaskFlowConnections::createGlobalEndTask() {
    return task_flow_graph_.taskflow->emplace([scheduler = task_flow_graph_.scheduler, time_domain = task_flow_graph_.time_domain]() mutable {
        SPDLOG_TRACE("End time domain {0}", time_domain);
    }).name("end");
}

} // traact