/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "DataflowMessageReceiver.h"

DataflowMessageReceiver::DataflowMessageReceiver(tbb::flow::graph *graph) {
    node_ = new tbb::flow::function_node<traact::dataflow::TraactMessage>(*graph,
                                                                          1,
                                                                          std::bind(&DataflowMessageReceiver::receive,
                                                                                    this,
                                                                                    std::placeholders::_1));
}

DataflowMessageReceiver::~DataflowMessageReceiver() {
    delete node_;
}

void DataflowMessageReceiver::receive(const traact::dataflow::TraactMessage &msg) {
    tbb::queuing_mutex::scoped_lock lock(mutex_);
    this->msg.push_back(msg);
}
