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

    [[nodiscard]] size_t getInputCount() const;

    template<typename HeaderType>
    const typename HeaderType::NativeType &getInput(size_t index) const {
        return *static_cast<typename HeaderType::NativeType *>(input_buffer_[index]);
    }

    template<typename Port>
    const typename Port::Header::NativeType &getInput() const {
        return *static_cast<typename Port::Header::NativeType *>(input_buffer_[Port::PortIdx]);
    }

    template<typename Port>
    const typename Port::Header::NativeType &getInput(int port_group_index, int port_group_instance_index) const {
        const size_t kIndex = input_groups_[port_group_index].group_offset
            + input_groups_[port_group_index].group_port_count * port_group_instance_index + Port::PortIdx;
        return *static_cast<typename Port::Header::NativeType *>(input_buffer_[kIndex]);
    }

    template<typename Port>
    const typename Port::Header &getInputHeader() const {
        return *static_cast<typename Port::Header *>(input_header_[Port::PortIdx]);
    }

    template<typename HeaderType>
    const HeaderType &getInputHeader(int index) const {
        return *static_cast<HeaderType *>(input_header_[index]);
    }

    [[nodiscard]] bool isInputValid(size_t index) const;

    template<typename Port>
    [[nodiscard]] bool isInputValid() const {
        return isInputValid(Port::PortIdx);
    }

    template<typename Port>
    bool isInputValid(int port_group_index, int port_group_instance_index) const {
        const size_t kIndex = input_groups_[port_group_index].group_offset
            + input_groups_[port_group_index].group_port_count * port_group_instance_index + Port::PortIdx;
        return isInputValid(kIndex);
    }

    bool isInputGroupValid(int port_group_index, int port_group_instance_index) const;

    [[nodiscard]] bool isAllInputValid() const;

    [[nodiscard]] Timestamp getInputTimestamp(size_t index) const;

    template<typename Port>
    [[nodiscard]] Timestamp getInputTimestamp() const {
        return getInputTimestamp(Port::PortIdx);

    }

    [[nodiscard]] size_t getOutputCount() const;

    template<typename HeaderType>
    typename HeaderType::NativeType &getOutput(size_t index, Timestamp timestamp) const {
        *output_valid_[index] = PortState::VALID;
        *output_timestamp_[index] = timestamp;
        return *static_cast<typename HeaderType::NativeType *>(output_buffer_[index]);
    }

    template<typename HeaderType>
    typename HeaderType::NativeType &getOutput(size_t index) const {
        return getOutput<HeaderType>(index, *timestamp_);
    }

    template<typename Port>
    typename Port::Header::NativeType &getOutput(int port_group_index, int port_group_instance_index) const {
        const size_t kIndex = output_groups_[port_group_index].group_offset
            + output_groups_[port_group_index].group_port_count * port_group_instance_index + Port::PortIdx;
        return getOutput<typename Port::Header>(kIndex, *timestamp_);
    }

    template<typename Port>
    typename Port::Header::NativeType &getOutput() const {
        return getOutput<typename Port::Header>(Port::PortIdx);
    }

    template<typename Port>
    typename Port::Header::NativeType &getOutput(Timestamp timestamp) const {
        return getOutput<typename Port::Header>(Port::PortIdx, timestamp);
    }

    template<typename Port>
    typename Port::Header &getOutputHeader() const {
        return *static_cast<typename Port::Header *>(output_header_[Port::PortIdx]);
    }

    template<typename Port>
    typename Port::Header &getOutputHeader(int port_group_index, int port_group_instance_index) const {
        const size_t kIndex = output_groups_[port_group_index].group_offset
            + output_groups_[port_group_index].group_port_count * port_group_instance_index + Port::PortIdx;
        return *static_cast<typename Port::Header *>(output_header_[kIndex]);
    }

    Timestamp getOutputTimestamp(size_t index) const;

    template<typename Port>
    typename Port::Header::NativeType &getOutputTimestamp() const {
        return *getOutputTimestamp(output_buffer_[Port::PortIdx]);
    }

    [[nodiscard]] bool isOutputValid(size_t index) const;

    template<typename Port>
    [[nodiscard]] bool isOutputValid() const {
        return isOutputValid(Port::PortIdx);
    }

    void setOutputInvalid(size_t index) const;

    template<typename Port>
    void setOutputInvalid() const {
        setOutputInvalid(Port::PortIdx);
    }

    [[nodiscard]] Timestamp getTimestamp() const;

    [[nodiscard]] size_t getComponentIndex() const;

    [[nodiscard]] size_t getTimeStepIndex() const;

    [[nodiscard]] EventType getEventType() const;

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
