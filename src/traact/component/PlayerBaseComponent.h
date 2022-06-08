/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_COMPONENT_PLAYERBASECOMPONENT_H_
#define TRAACT_CORE_SRC_TRAACT_COMPONENT_PLAYERBASECOMPONENT_H_

#include "traact/traact.h"
#include <queue>
#include <rttr/type>
#include <thread>

namespace traact::component {

class TRAACT_CORE_EXPORT PlayerBaseComponent : public ModuleComponent {
 public:
    PlayerBaseComponent(std::string name)
        : ModuleComponent(name,
                          traact::component::ComponentType::ASYNC_SOURCE, ModuleType::GLOBAL) {
    };

    std::string getModuleKey() override;
    Module::Ptr instantiateModule() override;

    virtual Timestamp getFirstTimestamp() = 0;
    virtual bool hasNext() = 0;
    virtual Timestamp getNextTimestamp() = 0;
    virtual void sendCurrent(Timestamp ts) = 0;

 protected:

 RTTR_ENABLE(ModuleComponent)

};

class TRAACT_CORE_EXPORT PlayerBaseModule : public Module {
 public:
    PlayerBaseModule() = default;
    bool init(ComponentPtr module_component) override;
    bool start(ComponentPtr module_component) override;
    bool stop(ComponentPtr module_component) override;
    bool teardown(ComponentPtr module_component) override;

    template<typename T>
    std::vector<T *> getComponentsOfClass() {
        std::vector<T *> result;

        for (PlayerBaseComponent *component : outputs_) {
            T *tmp = dynamic_cast<T *>(component);
            if (tmp)
                result.emplace_back(tmp);
        }

        return std::move(result);
    }

 private:
    struct Event {
        Timestamp time_at_next_event;
        Timestamp ts;
        PlayerBaseComponent *component;

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
            component->sendCurrent(time_at_next_event);
            if (!component->hasNext())
                return false;

            auto next_ts = component->getNextTimestamp();
            auto time_till_next = next_ts - ts;
            time_at_next_event = time_at_next_event + time_till_next;
            ts = next_ts;
            return true;
        }
    };
    std::vector<PlayerBaseComponent *> outputs_;
    std::priority_queue<Event, std::vector<Event>, std::greater<Event> > events_;

    std::shared_ptr<std::thread> thread_;
    bool initialized_{false};
    bool running_{false};
    std::mutex component_lock_;

    void threadLoop();

 RTTR_ENABLE(Module)
};
}

#endif //TRAACT_CORE_SRC_TRAACT_COMPONENT_PLAYERBASECOMPONENT_H_
