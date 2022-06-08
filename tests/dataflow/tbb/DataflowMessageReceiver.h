/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACTMULTI_DATAFLOWMESSAGERECEIVER_H
#define TRAACTMULTI_DATAFLOWMESSAGERECEIVER_H

#include <dataflow/tbb/DataflowMessage.h>
#include <tbb/flow_graph.h>
#include <tbb/queuing_mutex.h>

class DataflowMessageReceiver {
 public:
    DataflowMessageReceiver(tbb::flow::graph *graph);

    virtual ~DataflowMessageReceiver();

    void receive(const traact::dataflow::TraactMessage &msg);

    std::vector<traact::dataflow::TraactMessage> msg;
    tbb::flow::function_node<traact::dataflow::TraactMessage> *node_;

 private:
    tbb::queuing_mutex mutex_;

};

#endif //TRAACTMULTI_DATAFLOWMESSAGERECEIVER_H
