/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TaskFlowGraphBuilder.h"

namespace traact::dataflow {
TaskFlowGraphBuilder::TaskFlowGraphBuilder(TraactTaskFlowGraph &task_flow_graph) : task_flow_graph_(task_flow_graph) {}
} // traact