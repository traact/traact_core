/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <iostream>
#include "Logging.h"

void TRAACT_CORE_EXPORT
traact::util::initLogging(spdlog::level::level_enum log_level,
                          std::string file,
                          std::chrono::seconds flush_time) {
    try {
        spdlog::set_level(log_level);

        if (flush_time > std::chrono::seconds(0))
            spdlog::flush_every(flush_time);

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(log_level);

        //console_sink->set_pattern("[%^%l%$] %v");

        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S:%e] - [%^%l%$] - %s:%# \n%v");

        std::shared_ptr<spdlog::logger> logger;
        if (!file.empty()) {
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(file);
            spdlog::sinks_init_list sinks{file_sink, console_sink};
            logger = std::make_shared<spdlog::logger>("multi_sink", sinks.begin(), sinks.end());
        } else {
            spdlog::sinks_init_list sinks{console_sink};
            logger = std::make_shared<spdlog::logger>("multi_sink", sinks.begin(), sinks.end());
        }
        logger->set_level(log_level);

        spdlog::register_logger(logger);
        spdlog::set_default_logger(spdlog::get("multi_sink"));

    }
    catch (const spdlog::spdlog_ex &ex) {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
    }
}
