/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TIMEDOMAINCLOCK_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TIMEDOMAINCLOCK_H_

#include <vector>
#include "traact/datatypes.h"

namespace traact::dataflow {
    class TimeDomainClock {
     public:

        TimeDomainClock(double expected_sensor_frequency, const TimeDuration &max_offset, double alpha_0);

        void newTimestamp(Timestamp timestamp);

        void advanceTimeStep();


        bool isNextExpectedTimestamp(Timestamp next_timestamp) const;
        bool isNextExpectedTimestampOrBigger(Timestamp next_timestamp) const;
        bool isFurtherAheadThenNextExpectedTimestamp(Timestamp next_timestamp) const;

        Timestamp getNextExpectedTimestamp();
        Timestamp getInitTimestamp();
     private:
        double expected_sensor_frequency_;
        TimeDuration max_offset_;
        double alpha_0_;
        double alpha_1_;

        TimeDuration sensor_delta_mean_;

        size_t current_time_step_{0};
        Timestamp initial_timestamp_mean_{kTimestampZero};
        Timestamp current_timestamp_{kTimestampZero};
        Timestamp expected_next_timestamp_{kTimestampZero};
        std::vector<Timestamp> received_next_timestamps_;

    };
}


#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TIMEDOMAINCLOCK_H_
