/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_BUFFER_COMPONENTBUFFER_H_
#define TRAACT_CORE_SRC_TRAACT_BUFFER_COMPONENTBUFFER_H_

#include <tuple>

#include <traact/datatypes.h>
#include <traact/traact_core_export.h>
#include <traact/util/Logging.h>
#include "BufferUtils.h"

template<typename ResultType, typename HeaderType, typename DataType>
ResultType getBufferAs(const HeaderType &header, const DataType &data);

template<typename ResultType, typename HeaderType, typename DataType>
ResultType getBufferAs(const HeaderType &header, DataType &data);

namespace traact::buffer {

class TRAACT_CORE_EXPORT ComponentBuffer {
 public:

    ComponentBuffer(size_t component_index,
                    LocalDataBuffer input_buffer,
                    LocalHeaderBuffer input_header,
                    LocalValidBuffer input_valid,
                    LocalTimestampBuffer input_timestamp,
                    LocalGroupBuffer input_groups,
                    LocalDataBuffer output_buffer,
                    LocalHeaderBuffer output_header,
                    LocalValidBuffer output_valid,
                    LocalTimestampBuffer output_timestamp,
                    LocalGroupBuffer output_groups,
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

    template<typename Port>
    const typename Port::Header::NativeType & getInput(int port_group_index, int port_group_instance_index) const noexcept {
        const size_t kIndex = input_groups_[port_group_index].group_offset
            + input_groups_[port_group_index].group_port_count * port_group_instance_index + Port::PortIdx;
        return *static_cast<typename Port::Header::NativeType *>(input_buffer_[kIndex]);
    }

    template<typename Port>
    const typename Port::Header &getInputHeader() const noexcept {
        return *static_cast<typename Port::Header *>(input_header_[Port::PortIdx]);
    }

//    template<typename Port, typename ReturnType>
//    ReturnType getInputAs() const noexcept {
//        auto *data = static_cast<typename Port::Header::NativeType *>(input_buffer_[Port::PortIdx]);
//        auto *header = static_cast<typename Port::Header *>(input_header_[Port::PortIdx]);
//        return getBufferAs<ReturnType, typename Port::Header, typename Port::Header::NativeType>(*header, *data);
//
//    }

    [[nodiscard]] bool isInputValid(size_t index) const noexcept;

    template<typename Port>
    [[nodiscard]] bool isInputValid() const noexcept {
        return isInputValid(Port::PortIdx);
    }

    template<typename Port>
    bool isInputValid(int port_group_index, int port_group_instance_index) const noexcept {
        const size_t kIndex = input_groups_[port_group_index].group_offset
            + input_groups_[port_group_index].group_port_count * port_group_instance_index + Port::PortIdx;
        return isInputValid(kIndex);
    }


    bool isInputGroupValid(int port_group_index, int port_group_instance_index) const noexcept;

    [[nodiscard]] bool isAllInputValid() const noexcept;

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

    template<typename HeaderType>
    typename HeaderType::NativeType &getOutput(size_t index, Timestamp timestamp) const noexcept {
        *output_valid_[index] = PortState::VALID;
        *output_timestamp_[index] = timestamp;
        return *static_cast<typename HeaderType::NativeType *>(output_buffer_[index]);
    }

    template<typename Port>
    typename Port::Header::NativeType &getOutput(int port_group_index, int port_group_instance_index) const noexcept {

        const size_t kIndex = output_groups_[port_group_index].group_offset
            + output_groups_[port_group_index].group_port_count * port_group_instance_index + Port::PortIdx;
        *output_valid_[kIndex] = PortState::VALID;
        *output_timestamp_[kIndex] = *timestamp_;
        return *static_cast<typename Port::Header::NativeType *>(output_buffer_[kIndex]);
    }

    template<typename Port>
    typename Port::Header::NativeType &getOutput() const noexcept {
        *output_valid_[Port::PortIdx] = PortState::VALID;
        *output_timestamp_[Port::PortIdx] = *timestamp_;
        return *static_cast<typename Port::Header::NativeType *>(output_buffer_[Port::PortIdx]);
    }

    template<typename Port>
    typename Port::Header::NativeType &getOutput(Timestamp timestamp) const noexcept {
        *output_valid_[Port::PortIdx] = PortState::VALID;
        *output_timestamp_[Port::PortIdx] = timestamp;
        return *static_cast<typename Port::Header::NativeType *>(output_buffer_[Port::PortIdx]);
    }

    template<typename Port>
    typename Port::Header &getOutputHeader() const noexcept {
        return *static_cast<typename Port::Header *>(output_header_[Port::PortIdx]);
    }

    template<typename Port>
    typename Port::Header &getOutputHeader(int port_group_index, int port_group_instance_index) const noexcept {
        const size_t kIndex = output_groups_[port_group_index].group_offset
            + output_groups_[port_group_index].group_port_count * port_group_instance_index + Port::PortIdx;
        return *static_cast<typename Port::Header *>(output_header_[kIndex]);
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
    const LocalHeaderBuffer input_header_;
    const LocalValidBuffer input_valid_;
    const LocalTimestampBuffer input_timestamp_;
    const LocalGroupBuffer input_groups_;

    const LocalDataBuffer output_buffer_;
    const LocalHeaderBuffer output_header_;
    const LocalValidBuffer output_valid_;
    const LocalTimestampBuffer output_timestamp_;
    const LocalGroupBuffer output_groups_;

    const size_t time_step_index_;
    const Timestamp *timestamp_;
    const EventType *event_type_;

};
}

#endif// TRAACT_CORE_SRC_TRAACT_BUFFER_COMPONENTBUFFER_H_
