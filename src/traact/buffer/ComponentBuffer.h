/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACT_CORE_SRC_TRAACT_BUFFER_COMPONENTBUFFER_H_
#define TRAACT_CORE_SRC_TRAACT_BUFFER_COMPONENTBUFFER_H_

#include <tuple>

#include <traact/datatypes.h>
#include <traact/traact_core_export.h>
#include <traact/util/Logging.h>
#include "BufferUtils.h"
namespace traact::buffer {

class TRAACT_CORE_EXPORT ComponentBuffer {
 public:

    ComponentBuffer(size_t component_index,
                    LocalDataBuffer input_buffer,
                    LocalValidBuffer input_valid,
                    LocalTimestampBuffer input_timestamp,
                    LocalDataBuffer output_buffer,
                    LocalValidBuffer output_valid,
                    LocalTimestampBuffer output_timestamp,
                    size_t time_step_index,
                    const Timestamp *time_step_ts,
                    const EventType *message_type);
    ComponentBuffer() = delete;

    [[nodiscard]] size_t getInputCount() const noexcept;

    template<typename HeaderType>
    const typename HeaderType::NativeType &getInput(size_t index) const noexcept {
        return *static_cast<typename HeaderType::NativeType *>(input_buffer_[index]);
    }

    template<typename Port>
    const typename Port::Header::NativeType &getInput() const noexcept {
        return *static_cast<typename Port::Header::NativeType *>(input_buffer_[Port::PortIdx]);

    }

    [[nodiscard]] bool isInputValid(size_t index) const noexcept;

    template<typename Port>
    [[nodiscard]] bool isInputValid() const noexcept {
        return isInputValid(Port::PortIdx);
    }

    [[nodiscard]] Timestamp getInputTimestamp(size_t index) const noexcept;

    template<typename Port>
    [[nodiscard]] Timestamp getInputTimestamp() const noexcept {
        return getInputTimestamp(Port::PortIdx);

    }

    [[nodiscard]] size_t getOutputCount() const noexcept;

    template<typename HeaderType>
    typename HeaderType::NativeType &getOutput(size_t index) const noexcept {
        *output_valid_[index] = PortState::VALID;
        *output_timestamp_[index] = *timestamp_;
        return *static_cast<typename HeaderType::NativeType *>(output_buffer_[index]);
    }

    template<typename Port>
    typename Port::Header::NativeType &getOutput() const noexcept {
        *output_valid_[Port::PortIdx] = PortState::VALID;
        *output_timestamp_[Port::PortIdx] = *timestamp_;
        return *static_cast<typename Port::Header::NativeType *>(output_buffer_[Port::PortIdx]);
    }

    template<typename HeaderType>
    typename HeaderType::NativeType &getOutput(size_t index, Timestamp timestamp) const noexcept {
        *output_valid_[index] = PortState::VALID;
        *output_timestamp_[index] = timestamp;
        return *static_cast<typename HeaderType::NativeType *>(output_buffer_[index]);
    }

    template<typename Port>
    typename Port::Header::NativeType &getOutput(Timestamp timestamp) const noexcept {
        *output_valid_[Port::PortIdx] = PortState::VALID;
        *output_timestamp_[Port::PortIdx] = timestamp;
        return *static_cast<typename Port::Header::NativeType *>(output_buffer_[Port::PortIdx]);
    }

    Timestamp getOutputTimestamp(size_t index) const noexcept;

    template<typename Port>
    typename Port::Header::NativeType &getOutputTimestamp() const noexcept {
        return *getOutputTimestamp(output_buffer_[Port::PortIdx]);
    }

    [[nodiscard]] bool isOutputValid(size_t index) const noexcept;

    template<typename Port>
    [[nodiscard]] bool isOutputValid() const noexcept {
        return isOutputValid(Port::PortIdx);
    }

    void setOutputInvalid(size_t index) const noexcept;

    template<typename Port>
    void setOutputInvalid() const noexcept {
        setOutputInvalid(Port::PortIdx);
    }

    [[nodiscard]] Timestamp getTimestamp() const noexcept;

    [[nodiscard]] size_t getComponentIndex() const noexcept;

    [[nodiscard]] size_t getTimeStepIndex() const noexcept;

    [[nodiscard]] EventType getEventType() const noexcept;


 private:
    const size_t component_index_;
    const LocalDataBuffer input_buffer_;
    const LocalValidBuffer input_valid_;
    const LocalTimestampBuffer input_timestamp_;
    const LocalDataBuffer output_buffer_;
    const LocalValidBuffer output_valid_;
    const LocalTimestampBuffer output_timestamp_;
    const size_t time_step_index_;
    const Timestamp* timestamp_;
    const EventType* event_type_;
};
}

#endif// TRAACT_CORE_SRC_TRAACT_BUFFER_COMPONENTBUFFER_H_
