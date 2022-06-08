/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "PlayerBaseComponent.h"
std::string traact::component::PlayerBaseComponent::getModuleKey() {
    return "GlobalPlayer";
}
traact::component::Module::Ptr traact::component::PlayerBaseComponent::instantiateModule() {
    return std::make_shared<PlayerBaseModule>();
}

bool traact::component::PlayerBaseModule::init(traact::component::Module::ComponentPtr module_component) {
    std::lock_guard guard(component_lock_);
    auto *tmp = dynamic_cast<PlayerBaseComponent *>(module_component);
    if (tmp == nullptr) {
        SPDLOG_ERROR("Could not cast ModuleComponent to PlayerBaseComponent");
        return false;
    }
    outputs_.emplace_back(tmp);
    initialized_ = true;

    return true;
}
bool traact::component::PlayerBaseModule::start(traact::component::Module::ComponentPtr module_component) {
    std::lock_guard guard(component_lock_);
    if (running_) {
        return true;
    }

    running_ = true;
    SPDLOG_DEBUG("Starting Global Player Module");
    thread_ = std::make_shared<std::thread>([this] {
        threadLoop();
    });

    return true;
}
bool traact::component::PlayerBaseModule::stop(traact::component::Module::ComponentPtr module_component) {
    std::lock_guard guard(component_lock_);
    if (!running_) {
        return true;
    }
    return Module::stop(module_component);
}
bool traact::component::PlayerBaseModule::teardown(traact::component::Module::ComponentPtr module_component) {
    std::lock_guard guard(component_lock_);
    if (!initialized_) {
        return true;
    }
    return true;
}
void traact::component::PlayerBaseModule::threadLoop() {
    // find first timestamp
    Timestamp current_ts_component = Timestamp::max();
    for (auto *component : outputs_) {
        auto first_timestamp = component->getFirstTimestamp();
        current_ts_component = std::min(first_timestamp, current_ts_component);
    }

    size_t all_send_count = 0;
    SPDLOG_INFO("start global player in 1 second");
    Timestamp current_ts = now() + std::chrono::milliseconds(1000);
    TimeDuration diff_current_to_first = current_ts - current_ts_component;
    // generate first events using now time + 1sec as start
    for (auto *component : outputs_) {
        auto next_ts = component->getNextTimestamp();
        auto time_till_next = next_ts - current_ts_component;
        Event event;
        event.component = component;
        event.time_at_next_event = current_ts + time_till_next;
        event.ts = next_ts;
        events_.emplace(event);
    }
    // sort events so that event with the earliest timestamp is first



    while (running_ && !events_.empty()) {
        // get next event
        auto next_event = events_.top();
        events_.pop();
        current_ts = now();

        // check if it should be sent now or if thread should wait

        bool log_once = true;

        while (next_event.time_at_next_event > current_ts + std::chrono::microseconds(100)) {
            // if the time is longer than 10ms then sleep, otherwise just yield and check again
            if (log_once) {
                SPDLOG_INFO("player has to wait till next event");
                log_once = false;
            }

            TimeDuration time_diff = next_event.time_at_next_event - current_ts;
            if (time_diff > std::chrono::milliseconds(2)) {
                SPDLOG_INFO("player sleeps since time > 2ms");
                TimeDuration sleep_time = time_diff - std::chrono::milliseconds(1);
                std::this_thread::sleep_for(sleep_time);
            } else {
                std::this_thread::yield();
            }
            current_ts = now();
        }

        if (next_event.sendAndNext()) {
            events_.emplace(next_event);
        } else {
            all_send_count++;
            // TODO make message depend on mode
            SPDLOG_INFO(
                "Component has no more evens in buffer, reached end or buffer could not be filled fast enough for realtime replay: {0}",
                next_event.component->getName());
        }
    }
    SPDLOG_INFO("Global Player finished");
    running_ = false;

}
