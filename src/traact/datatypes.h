/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_INCLUDE_TRAACT_DATATYPES_H_
#define TRAACT_INCLUDE_TRAACT_DATATYPES_H_

#include <chrono>
#include <sstream>
#include <traact/traact_core_export.h>
namespace traact {

using Scalar = float;

using TimeDuration = std::chrono::duration<int64_t, std::nano>;
using Timestamp = std::chrono::time_point<std::chrono::system_clock, TimeDuration>;

static const constexpr Timestamp kTimestampZero(TimeDuration(0));

using TimestampSteady = std::chrono::time_point<std::chrono::steady_clock, TimeDuration>;
static const constexpr TimestampSteady kTimestampSteadyZero(TimeDuration(0));

static const constexpr TimeDuration kDefaultTimeout = std::chrono::milliseconds(100);
static const constexpr TimeDuration kDefaultWaitingTimeout = std::chrono::milliseconds(10000);
static const constexpr TimeDuration kDataflowStopTimeout = std::chrono::seconds(5);
static const constexpr int kDefaultTimeDomain{0};
static const constexpr char* kDefaultPortGroupName{"Default"};
static const constexpr int kDefaultPortGroupIndex{0};

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

inline static bool isWithinRange(const Timestamp& value_0, const Timestamp& value_1, const TimeDuration max_offset ){
    const auto kTs0Min = value_0 - max_offset;
    const auto kTs0Max = value_0 + max_offset;
    const auto kTs1Min = value_1 - max_offset;
    const auto kTs1Max = value_1 + max_offset;

    return kTs0Max >= kTs1Min && kTs0Min <= kTs1Max;
}

enum class MessageDataMode {
    INVALID = 0,
    STATIC,
    DYNAMIC
};

enum class EventType : uint8_t {
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

enum BaseType : int {
    UNKNOWN = 0,
    INT_8,
    UINT_8,
    INT_16,
    UINT_16,
    INT_32,
    FLOAT_16,
    FLOAT_32,
    FLOAT_64
};

static constexpr int getBytes(BaseType type) {
    switch (type) {
        case INT_8:
        case UINT_8:return 1;
        case INT_16:
        case UINT_16:
        case FLOAT_16:return 2;
        case INT_32:
        case FLOAT_32:return 4;
        case FLOAT_64:return 8;
        case UNKNOWN:
        default:return 0;
    }
}

enum class Concurrency : uint8_t { UNLIMITED = 0, SERIAL = 1 };

struct PortGroupInfo {
    int port_group_index;
    int size;
};


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
    double sensor_frequency{30};
    SourceMode source_mode{SourceMode::WAIT_FOR_BUFFER};
    MissingSourceEventMode missing_source_event_mode{MissingSourceEventMode::WAIT_FOR_EVENT};
    int cpu_count{0};
    std::string master_source{"invalid"};

};
}

}

#endif //TRAACT_INCLUDE_TRAACT_DATATYPES_H_

