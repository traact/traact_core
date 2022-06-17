/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "FpsCounter.h"

namespace traact {
void util::FpsCounter::event() noexcept {
    event_buffer_.push_back(nowSteady());
}
float util::FpsCounter::fps() const noexcept {
    if(event_buffer_.size() < 2 ){
        return 0;
    } else {
        std::chrono::duration<float> time_diff = event_buffer_.back() - event_buffer_.front();
        return event_buffer_.size() / time_diff.count();
    }

}
} // traact