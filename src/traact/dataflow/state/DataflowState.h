/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_STATE_DATAFLOWSTATE_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_STATE_DATAFLOWSTATE_H_

#include <memory>
#include <map>
#include "TimeDomainState.h"
namespace traact::dataflow {

class DataflowState {
 public:
    using SharedPtr = std::shared_ptr<DataflowState>;
    using ConstSharedPtr = std::shared_ptr<const DataflowState>;


    void init(int time_domain_count);

    size_t getTimeDomainCount() const;
    const std::shared_ptr<TimeDomainState>& getState(int time_domain);

 private:
    
    std::vector<std::shared_ptr<TimeDomainState>> time_domain_states_;


};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_STATE_DATAFLOWSTATE_H_
