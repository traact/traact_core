/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TimeDomainClock.h"
#include <numeric>

namespace traact::dataflow {


bool TimeDomainClock::isNextExpectedTimestamp(Timestamp next_timestamp) const {
    return isWithinRange(expected_next_timestamp_, next_timestamp, max_offset_);
}
bool TimeDomainClock::isFurtherAheadThenNextExpectedTimestamp(Timestamp next_timestamp) const {
    return expected_next_timestamp_+max_offset_ < next_timestamp-max_offset_;
}

Timestamp TimeDomainClock::getNextExpectedTimestamp() {
    auto result = expected_next_timestamp_;

    advanceTimeStep();

    return result;
}
void TimeDomainClock::newTimestamp(Timestamp timestamp) {
    // first call initializes time domain clock
    if(initial_timestamp_mean_ == kTimestampZero) {
        initial_timestamp_mean_ = timestamp;
        expected_next_timestamp_ = timestamp;
        current_timestamp_ = timestamp;
        current_time_step_ = 0;
    } else {
        // every next call is either from a different source of the same time domain or the next call from the same source
        if(isWithinRange(initial_timestamp_mean_, timestamp, max_offset_)){
            // same timestamp, different source, running average over all initial timestamps
            uint64_t ts_init = initial_timestamp_mean_.time_since_epoch().count() * alpha_0_;
            uint64_t ts_other = timestamp.time_since_epoch().count() * alpha_1_;
            initial_timestamp_mean_ = Timestamp(TimeDuration(ts_init+ts_other));
            current_timestamp_ = initial_timestamp_mean_;
        } else if(isNextExpectedTimestamp(timestamp)) {
            // next timestamp of any source
            received_next_timestamps_.emplace_back(timestamp);
        }
        //else: timestamp is further ahead then next, do nothing, getNextExpectedTimestamp should be called until the timestamp is the next timestamp
    }


}
void TimeDomainClock::advanceTimeStep() {
    ++current_time_step_;
    auto get_mean_timestamp = [](const std::vector<Timestamp> &values){
        if(values.empty()){
            return kTimestampZero;
        }
        uint64_t mean_ts(0);
        for (const auto& timestamp : values) {
            mean_ts += timestamp.time_since_epoch().count();
        }
        mean_ts = mean_ts / values.size();
        return Timestamp(TimeDuration (mean_ts));
    };
    if(!received_next_timestamps_.empty()){
        auto mean_next = get_mean_timestamp(received_next_timestamps_);
        auto time_diff = mean_next - current_timestamp_;

        sensor_delta_mean_ = TimeDuration(static_cast<uint64_t>(sensor_delta_mean_.count()*alpha_0_ + time_diff.count()*alpha_1_));
        received_next_timestamps_.clear();
    }
    current_timestamp_ = expected_next_timestamp_;
    expected_next_timestamp_ = current_timestamp_ + sensor_delta_mean_;
}
TimeDomainClock::TimeDomainClock(double expected_sensor_frequency, const TimeDuration &max_offset, double alpha_0)
    : expected_sensor_frequency_(expected_sensor_frequency), max_offset_(max_offset), alpha_0_(alpha_0), alpha_1_(1.0 - alpha_0) {

    TimeDuration one_second (std::chrono::seconds(1));
    auto delta = one_second.count() / expected_sensor_frequency_;
    sensor_delta_mean_ =  TimeDuration (static_cast<uint64_t>(delta));
}
Timestamp TimeDomainClock::getInitTimestamp() {
    return initial_timestamp_mean_;
}
}