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

#ifndef TRAACTMULTI_TRAACT_FACADE_INCLUDE_TRAACT_COMPONENT_PLAYERBASECOMPONENT_H_
#define TRAACTMULTI_TRAACT_FACADE_INCLUDE_TRAACT_COMPONENT_PLAYERBASECOMPONENT_H_

#include <traact/traact.h>
#include <queue>
#include <rttr/type>
#include <thread>

namespace traact::component {


class PlayerBaseComponent : public ModuleComponent {
 public:
  PlayerBaseComponent(std::string name)
      : ModuleComponent(name,
                        traact::component::ComponentType::AsyncSource, ModuleType::Global) {
  };


  std::string GetModuleKey() override;
  Module::Ptr InstantiateModule() override;

  virtual TimestampType GetFirstTimestamp() = 0;
  virtual bool HasNext() = 0;
  virtual TimestampType GetNextTimestamp() = 0;
  virtual void SendCurrent(TimestampType ts) = 0;


 protected:

    RTTR_ENABLE(ModuleComponent)

};

class PlayerBaseModule : public Module {
 public:
  PlayerBaseModule() = default;
  bool init(ComponentPtr module_component) override;
  bool start(ComponentPtr module_component) override;
  bool stop(ComponentPtr module_component) override;
  bool teardown(ComponentPtr module_component) override;

  template<typename T> std::vector<T*> GetComponentsOfClass() {
    std::vector<T*> result;

    for(PlayerBaseComponent* component : outputs_) {
        T* tmp = dynamic_cast<T*>(component);
        if(tmp)
          result.emplace_back(tmp);
    }

    return std::move(result);
  }


 private:
  struct Event {
      TimestampType time_at_next_event;
      TimestampType ts;
      PlayerBaseComponent* component;

    bool operator==(const Event &rhs) const {
      return ts == rhs.ts;
    }
    bool operator!=(const Event &rhs) const {
      return !(rhs == *this);
    }
    bool operator<(const Event &rhs) const {
      return ts < rhs.ts;
    }
    bool operator>(const Event &rhs) const {
      return rhs < *this;
    }
    bool operator<=(const Event &rhs) const {
      return !(rhs < *this);
    }
    bool operator>=(const Event &rhs) const {
      return !(*this < rhs);
    }

    bool sendAndNext() {
        component->SendCurrent(time_at_next_event);
        if(!component->HasNext())
          return false;

        auto next_ts = component->GetNextTimestamp();
        auto time_till_next = next_ts - ts;
        time_at_next_event = time_at_next_event + time_till_next;
        ts = next_ts;
        return true;
      }
  };
  std::vector<PlayerBaseComponent*> outputs_;
  std::priority_queue<Event, std::vector<Event>, std::greater<Event> > events_;

  std::shared_ptr<std::thread> thread_;
  bool initialized_{false};
  bool running_{false};
  std::mutex component_lock_;

  void threadLoop();

    RTTR_ENABLE(Module)
};
}


#endif //TRAACTMULTI_TRAACT_FACADE_INCLUDE_TRAACT_COMPONENT_PLAYERBASECOMPONENT_H_
