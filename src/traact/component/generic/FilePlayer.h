/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_COMPONENT_FILEPLAYER_H_
#define TRAACT_CORE_SRC_TRAACT_COMPONENT_FILEPLAYER_H_

#include "traact/component/Component.h"
#include "traact/traact_plugins.h"
#include <spdlog/spdlog.h>
#include <traact/pattern/ParameterUtils.h>

namespace traact::component {

template<class T>
class FilePlayer : public Component {
 public:
    explicit FilePlayer(const std::string &name, std::string serializer_name) : Component(name),
                                                                                serializer_name_(std::move(
                                                                                    serializer_name)) {
    }

    [[nodiscard]] static traact::pattern::Pattern::Ptr GetBasePattern(const std::string& serializer_name) {

        std::string pattern_name = fmt::format("FilePlayer_{0}_{1}", serializer_name, T::NativeTypeName);

        traact::pattern::Pattern::Ptr
            pattern =
            std::make_shared<traact::pattern::Pattern>(pattern_name, Concurrency::SERIAL, ComponentType::ASYNC_SOURCE);

        pattern->addProducerPort("output", T::NativeTypeName);
        pattern->addStringParameter("file", "file.json");

        pattern->addCoordinateSystem("A")
            .addCoordinateSystem("B")
            .addEdge("A", "B", "output");

        return pattern;
    }

    bool configure(const pattern::instance::PatternInstance &pattern_instance, buffer::ComponentBufferConfig *data) override {
        bool result = pattern::setValueFromParameter(pattern_instance, "file", filename_, "");
        if (!result) {
            return false;
        }

        return openFile();
    }

    bool start() override {
        running_ = true;
        thread_ = std::make_shared<std::thread>(std::bind(&FilePlayer<T>::threadLoop, this));
        return Component::start();
    }

    bool stop() override {
        if (running_) {
            running_ = false;
            thread_->join();
        }
        return true;
    }

    bool teardown() override {
        return closeFile();
    }

    virtual bool openFile() = 0;
    virtual bool closeFile() = 0;
    virtual bool readTimestamp(Timestamp &timestamp) = 0;
    virtual bool readValue(typename T::NativeType &data) = 0;

 protected:
    std::string filename_;
    const std::string serializer_name_;
    std::shared_ptr<std::thread> thread_;

    std::atomic<bool> running_{false};

    void threadLoop() {

        Timestamp ts;

        while (running_) {

            if (!readTimestamp(ts)) {
                SPDLOG_INFO("{0}: End of file or error when trying to read next data, exit thread");
                running_ = false;
                return;
            }

            auto buffer = request_callback_(ts);
            buffer.wait();
            auto buffer_p = buffer.get();
            if (buffer_p != nullptr) {
                auto &new_data = buffer_p->template getOutput<T>(0);
                readValue(new_data);
                buffer_p->commit(true);
            } else {
                SPDLOG_ERROR("request to get next buffer failed");
                std::this_thread::yield();
            }

        }
    }


};

}

#endif //TRAACT_CORE_SRC_TRAACT_COMPONENT_FILEPLAYER_H_
