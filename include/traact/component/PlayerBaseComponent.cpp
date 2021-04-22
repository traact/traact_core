/*  BSD 3-Clause License
 *
 *  Copyright (c) 2020, FriederPankratz <frieder.pankratz@gmail.com>
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/

#include "PlayerBaseComponent.h"
std::string traact::component::PlayerBaseComponent::GetModuleKey() {
  return "GlobalPlayer";
}
traact::component::Module::Ptr traact::component::PlayerBaseComponent::InstantiateModule() {
  return std::make_shared<PlayerBaseModule>();
}

bool traact::component::PlayerBaseModule::init(traact::component::Module::ComponentPtr module_component) {
  std::lock_guard guard(component_lock_);
  PlayerBaseComponent* tmp = dynamic_cast<PlayerBaseComponent*>(module_component);
  if(!tmp) {
    SPDLOG_ERROR("Could not cast ModuleComponent to PlayerBaseComponent");
    return false;
  }
  outputs_.emplace_back(tmp);

  return true;
}
bool traact::component::PlayerBaseModule::start(traact::component::Module::ComponentPtr module_component) {
  std::lock_guard guard(component_lock_);
  if(running_)
    return true;

  running_ = true;
  SPDLOG_DEBUG("Starting Global Player Module");
  thread_ = std::make_shared<std::thread>([this]{
    threadLoop();
  });

  return true;
}
bool traact::component::PlayerBaseModule::stop(traact::component::Module::ComponentPtr module_component) {
  std::lock_guard guard(component_lock_);
  if(!running_)
    return true;
  return Module::stop(module_component);
}
bool traact::component::PlayerBaseModule::teardown(traact::component::Module::ComponentPtr module_component) {
  std::lock_guard guard(component_lock_);
  if(!initialized_)
    return true;
  return true;
}
void traact::component::PlayerBaseModule::threadLoop() {
  // find first timestamp
  TimestampType current_ts_component = TimestampType::max();
  for(auto component : outputs_) {
    auto ts = component->GetFirstTimestamp();
    current_ts_component = std::min(ts, current_ts_component);
  }

  size_t all_send_count = 0;
  size_t all_component_count = outputs_.size();
  SPDLOG_INFO("start global player in 1 second");
  TimestampType current_ts = now() + std::chrono::milliseconds(1000);
  TimeDurationType diff_current_to_first = current_ts - current_ts_component;
  // generate first events using now time + 1sec as start
  for(auto component : outputs_) {
    auto next_ts = component->GetNextTimestamp();
    auto time_till_next = next_ts - current_ts_component;
    Event e;
    e.component = component;
    e.time_at_next_event = current_ts + time_till_next;
    e.ts = next_ts;
    events_.emplace(std::move(e));
  }
  // sort events so that event with earliest timestamp is first



  while(running_ && !events_.empty()) { //all_send_count < all_component_count) {
    // get next event
    auto next_event = events_.top();
    events_.pop();
    current_ts = now();

    // check if it should be send now or if thread should wait

    bool log_once = true;

    while(next_event.time_at_next_event > current_ts+std::chrono::microseconds(100)){
      // if the time is longer than 10ms then sleep, otherwise just yield and check again
      if(log_once) {
          SPDLOG_INFO("player has to wait till next event");
          log_once = false;
      }

        TimeDurationType time_diff = next_event.time_at_next_event - current_ts;
        if(time_diff > std::chrono::milliseconds(2)){
            SPDLOG_INFO("player sleeps since time > 2ms");
          TimeDurationType sleep_time = time_diff - std::chrono::milliseconds(1);
          std::this_thread::sleep_for(sleep_time);
        } else {
          std::this_thread::yield();
        }
      current_ts = now();
    }

    if(next_event.sendAndNext()) {
      events_.emplace(std::move(next_event));
    } else {
      all_send_count++;
      // TODO make message depend on mode
      SPDLOG_INFO("Component has no more evens in buffer, reached end or buffer could not be filled fast enough for realtime replay: {0}", next_event.component->getName());
    }
  }
  SPDLOG_INFO("Global Player finished");
  running_ = false;

}
