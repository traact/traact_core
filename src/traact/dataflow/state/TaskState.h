/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_STATE_TASKSTATE_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_STATE_TASKSTATE_H_

#include "traact/datatypes.h"

namespace traact::dataflow {

class TaskState {

 public:

    void beginWaiting();
    void begin();
    void process(EventType event_type);
    void end();
    void error();
    void error(std::string error);
};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_STATE_TASKSTATE_H_
