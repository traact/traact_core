/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACTMULTI_DATAFLOWMESSAGE_H
#define TRAACTMULTI_DATAFLOWMESSAGE_H

#include <traact/datatypes.h>
#include <traact/buffer/TimeStepBuffer.h>
#include <tbb/tbb_stddef.h>

namespace traact::dataflow {

struct TraactMessage {
    MessageType message_type = MessageType::Invalid;
    size_t event_idx = 0;
    buffer::TimeStepBuffer *domain_buffer = nullptr;
    bool valid_data = false;

    uint64_t key() const {
        return event_idx;
    }

    inline void merge(const TraactMessage &msg) {
        __TBB_ASSERT(event_idx == msg.event_idx, "event index of sync input differ");
        __TBB_ASSERT(domain_buffer == msg.domain_buffer, "domain_buffer of sync input differ");

        valid_data = valid_data && msg.valid_data;

        switch (msg.message_type) {
            case MessageType::Invalid: {
                message_type = MessageType::Invalid;
                break;
            }
            default:break;

        }
    }

    std::string toString() const {
        std::stringstream ss;
        ss << "TraactMessage MeaIndex: " << event_idx << " message type: " << static_cast<int>(message_type)
           << std::endl;
        return ss.str();
    }

    bool operator==(const TraactMessage &rhs) const {
        return message_type == rhs.message_type &&
            event_idx == rhs.event_idx &&
            domain_buffer == rhs.domain_buffer &&
            valid_data == rhs.valid_data;
    }

    bool operator!=(const TraactMessage &rhs) const {
        return !(rhs == *this);
    }
};
}

#endif //TRAACTMULTI_DATAFLOWMESSAGE_H
