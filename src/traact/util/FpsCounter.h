/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_UTIL_FPSCOUNTER_H_
#define TRAACT_CORE_SRC_TRAACT_UTIL_FPSCOUNTER_H_

#include "traact/datatypes.h"
#include <traact/util/CircularBuffer.h>

namespace traact::util {

class FpsCounter {
 public:
    void event() noexcept;
    float fps() const noexcept;

 private:
    static const constexpr uint32_t k_max_events_{60};
    float fps_{0.0f};
    traact::util::CircularBuffer<TimestampSteady, k_max_events_> event_buffer_;

};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_UTIL_FPSCOUNTER_H_
