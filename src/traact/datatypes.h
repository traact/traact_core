/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACT_INCLUDE_TRAACT_DATATYPES_H_
#define TRAACT_INCLUDE_TRAACT_DATATYPES_H_

#include <chrono>
#include <sstream>
#include <traact/traact_core_export.h>
namespace traact {

using TimeDuration = std::chrono::duration<uint64_t, std::nano>;
using Timestamp = std::chrono::time_point<std::chrono::system_clock, TimeDuration>;

using TimestampSteady = std::chrono::time_point<std::chrono::steady_clock, TimeDuration>;

static constexpr  TimeDuration kDefaultTimeout = std::chrono::milliseconds(100);
static constexpr  TimeDuration kDataflowStopTimeout = std::chrono::seconds(5);

struct TRAACT_CORE_EXPORT TimestampHashCompare {
    static size_t hash(const Timestamp &x) {
        return std::hash<size_t>{}(x.time_since_epoch().count());
    }
    static bool equal(const Timestamp &x, const Timestamp &y) {
        return x == y;
    }
};

inline static Timestamp now() {
    return std::chrono::system_clock::now();
}
inline static TimestampSteady nowSteady() {
    return std::chrono::steady_clock::now();
}
enum class MessageDataMode {
    INVALID = 0,
    STATIC,
    DYNAMIC
};

enum class EventType : uint8_t{
    INVALID = 0,
    CONFIGURE,
    START,
    DATA,
    STOP,
    TEARDOWN,
    DATAFLOW_NO_OP,
    DATAFLOW_STOP

};

enum class SourceMode {
    WAIT_FOR_BUFFER = 0,
    IMMEDIATE_RETURN
};

enum class MissingSourceEventMode {
    WAIT_FOR_EVENT = 0,
    CANCEL_OLDEST
};

enum class Concurrency : uint8_t { UNLIMITED = 0, SERIAL = 1 };




namespace buffer {

template<typename HeaderType, size_t PortValue>
class PortConfig {
 public:
    using Header = HeaderType;
    enum { PortIdx = PortValue };
};

struct TRAACT_CORE_EXPORT TimeDomainManagerConfig {
    int time_domain{-1};
    size_t ringbuffer_size{0};
    TimeDuration max_offset{TimeDuration::min()};
    TimeDuration max_delay{TimeDuration::min()};
    TimeDuration measurement_delta{TimeDuration::min()};
    SourceMode source_mode{SourceMode::WAIT_FOR_BUFFER};
    MissingSourceEventMode missing_source_event_mode{MissingSourceEventMode::WAIT_FOR_EVENT};
    std::string master_source{"invalid"};

};
}

}

#endif //TRAACT_INCLUDE_TRAACT_DATATYPES_H_
