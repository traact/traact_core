/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACTMULTI_TBBNETWORK_H
#define TRAACTMULTI_TBBNETWORK_H

#include <traact/dataflow/Network.h>
#include <atomic>
#include <thread>

namespace traact::dataflow {
class NetworkGraph;
}

namespace traact::dataflow {

class TBBNetwork : public Network {
 public:
    TBBNetwork();
    ~TBBNetwork();

    bool start() override;

    bool stop() override;
 private:
    std::set<std::shared_ptr<NetworkGraph> > network_graphs_;
    std::atomic<int> finished_count_{0};
    std::thread stop_signal_thread_;
    void MasterSourceFinished();

};
}

#endif //TRAACTMULTI_TBBNETWORK_H
