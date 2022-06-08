/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include <iostream>
#include <signal.h>
#include <traact/traact.h>
#include <traact/facade/DefaultFacade.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>
#include <traact/util/FileUtil.h>
#include "spdlog/sinks/basic_file_sink.h"
#include <tclap/CmdLine.h>

traact::facade::Facade *myfacade{nullptr};

void ctrlC(int i) {
    SPDLOG_INFO("User requested exit (Ctrl-C).");
    if (myfacade)
        myfacade->stop();
}

int main(int argc, char **argv) {

    using namespace traact::facade;
    using namespace traact;
    using namespace traact::dataflow;

    signal(SIGINT, &ctrlC);

    traact::util::initLogging(spdlog::level::trace, "traact_log.txt");


    std::string dataflow_file;

    std::string plugin_dirs{};
    try {
        // TODO take version from conan package
        TCLAP::CmdLine cmd("Command description message", ' ', "0.1.0");
        TCLAP::ValueArg<std::string>
            dataflow_arg("d", "dataflow_file", "Dataflow file", true, "dataflow.json", "Dataflow File");
        cmd.add(dataflow_arg);
        TCLAP::ValueArg<std::string> log_level_arg
            ("l", "log_level", "Log Level [TRACE, DEBUG, INFO, WARN, ERROR]", false, "dataflow.json", "Log Level");
        cmd.add(log_level_arg);

        cmd.parse( argc, argv );

        dataflow_file = dataflow_arg.getValue();
        if(log_level_arg.isSet()) {
            std::string log_level = log_level_arg.getValue();
            if (log_level == "info") {
                spdlog::set_level(spdlog::level::info);
                spdlog::flush_on(spdlog::level::info);
                SPDLOG_INFO("Set Loglevel to INFO");
            } else if (log_level == "debug") {
                spdlog::set_level(spdlog::level::debug);
                spdlog::flush_on(spdlog::level::debug);
                SPDLOG_INFO("Set Loglevel to DEBUG");
            } else if (log_level == "trace") {
                spdlog::set_level(spdlog::level::trace);
                spdlog::flush_on(spdlog::level::trace);
                SPDLOG_INFO("Set Loglevel to TRACE");
                SPDLOG_TRACE("Test TRACE message");
            } else {
                spdlog::set_level(spdlog::level::trace);
                spdlog::flush_on(spdlog::level::trace);
            }
        }

    }
    catch (std::exception &e) {
        std::cerr << "Error parsing command line parameters : " << e.what() << std::endl;
        std::cerr << "Try traactConsole --help for help" << std::endl;
        return 1;
    }

    if(plugin_dirs.empty()){
        SPDLOG_INFO("no user defined plugin paths, try finding environment variable TRAACT_PLUGIN_PATHS");
        auto traact_env = std::getenv("TRAACT_PLUGIN_PATHS");
        if (traact_env){
            plugin_dirs = traact_env;
            SPDLOG_INFO("Environment plugin paths: {0}", plugin_dirs);
        } else {
            SPDLOG_INFO("No environment plugin paths");
        }

    }


    if (!util::fileExists(dataflow_file, "Facade")) {
        SPDLOG_ERROR("traactConsole: error: - invalid dataflow file: {0}", dataflow_file);
        return 1;
    }

    SPDLOG_INFO("using plugin directories: {0}", plugin_dirs);

    myfacade = new DefaultFacade(plugin_dirs);

    myfacade->loadDataflow(dataflow_file);

    myfacade->blockingStart();

    delete myfacade;

    SPDLOG_INFO("exit program");

    return 0;
}