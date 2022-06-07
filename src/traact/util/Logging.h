/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACT_CORE_SRC_TRAACT_UTIL_LOGGING_H_
#define TRAACT_CORE_SRC_TRAACT_UTIL_LOGGING_H_

#include <spdlog/spdlog.h>
#include <spdlog/formatter.h>
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
#endif //TRAACT_CORE_SRC_TRAACT_UTIL_LOGGING_H_
