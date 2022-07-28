/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "DataflowState.h"

namespace traact::dataflow {
TaskState &DataflowState::getTaskState(const std::string &task_id) {
    return task_states_[task_id];
}
} // traact