/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include <traact/dataflow/state/DataflowState.h>

namespace traact::dataflow {

void DataflowState::init(int time_domain_count) {
    time_domain_states_.reserve(time_domain_count);
    for (int i = 0; i < time_domain_count; ++i) {
        time_domain_states_.emplace_back(std::make_shared<TimeDomainState>());
    }

}
const std::shared_ptr<TimeDomainState> &DataflowState::getState(int time_domain) {
    return time_domain_states_[time_domain];
}
size_t DataflowState::getTimeDomainCount() const {
    return time_domain_states_.size();
}

} // traact