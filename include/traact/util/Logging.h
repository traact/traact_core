/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_UTIL_LOGGING_H_
#define TRAACT_CORE_SRC_TRAACT_UTIL_LOGGING_H_

#include <spdlog/spdlog.h>
#include <spdlog/formatter.h>
#include <fmt/chrono.h>

#include <traact/traact_core_export.h>
#include <traact/datatypes.h>
#include <traact/component/ComponentTypes.h>
#include "EnumToString.h"

namespace traact::util {
void TRAACT_CORE_EXPORT initLogging(spdlog::level::level_enum log_level,
                                    std::string file = std::string(),
                                    std::chrono::seconds flush_time = std::chrono::seconds(1));

}



template <> struct fmt::formatter<traact::Timestamp> {

    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.end();
    }
    template <typename FormatContext>
    auto format(const traact::Timestamp & value, FormatContext& ctx) -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{}", value.time_since_epoch().count());
    }
};

template <> struct fmt::formatter<std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<uint64_t, std::nano>>> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.end();
    }
    template <typename FormatContext>
    auto format(const std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<uint64_t, std::nano>> & value, FormatContext& ctx) -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{}", value.time_since_epoch().count());
    }
};

template <> struct fmt::formatter<traact::TimeDuration> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.end();
    }
    template <typename FormatContext>
    auto format(const traact::TimeDuration & value, FormatContext& ctx) -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{}", value.count());
    }
};

template <> struct fmt::formatter<traact::TimestampSteady> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.end();
    }
    template <typename FormatContext>
    auto format(const traact::TimestampSteady & value, FormatContext& ctx) -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{}", value.time_since_epoch().count());
    }
};

template <> struct fmt::formatter<traact::EventType> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.end();
    }
    template <typename FormatContext>
    auto format(const traact::EventType & value, FormatContext& ctx) -> decltype(ctx.out()) {

        return format_to(ctx.out(), "{}", traact::toString(value));
    }
};

template <> struct fmt::formatter<traact::Concurrency> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.end();
    }
    template <typename FormatContext>
    auto format(const traact::Concurrency & value, FormatContext& ctx) -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{}", traact::toString(value));
    }
};

template <> struct fmt::formatter<traact::component::ComponentType> {

    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const traact::component::ComponentType & value, FormatContext& ctx) -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{}", traact::toString(value));
    }
};

inline std::string formatDuration(traact::TimeDuration duration){
    if(duration > std::chrono::seconds(1)){
        return fmt::format("{0:>3.2}", std::chrono::duration_cast<traact::TimeDurationFloatSecond>(duration));
    } else if(duration > std::chrono::milliseconds(1)){
        return fmt::format("{0:>3.2}", std::chrono::duration_cast<traact::TimeDurationFloatMilli>(duration));
    } else if(duration > std::chrono::microseconds (1)){
        return fmt::format("{0:>3.2}", std::chrono::duration_cast<traact::TimeDurationFloatMicro>(duration));
    } else {
        return fmt::format("{0:>3.2}", std::chrono::duration_cast<traact::TimeDurationFloatNano>(duration));
    }
}

#endif //TRAACT_CORE_SRC_TRAACT_UTIL_LOGGING_H_
