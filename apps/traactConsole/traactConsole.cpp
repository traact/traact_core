/*  BSD 3-Clause License
 *
 *  Copyright (c) 2020, FriederPankratz <frieder.pankratz@gmail.com>
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/

#include <iostream>
#include <signal.h>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <traact/traact.h>

#include <spdlog/sinks/stdout_color_sinks.h>

bool bStop = false;

void ctrlC(int i) {
  spdlog::info("User requested exit (Ctrl-C).");
  bStop = true;
}

int main(int argc, char **argv) {

  using namespace traact::facade;
  using namespace traact;
  using namespace traact::dataflow;

  signal(SIGINT, &ctrlC);

  //
  try {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace);
    console_sink->set_pattern("[%^%l%$] %v");

  }
  catch (const spdlog::spdlog_ex &ex) {
    std::cout << "Log initialization failed: " << ex.what() << std::endl;
  }

  // program options
  std::string dataflow_file;
  std::string log_level;
  std::string plugin_dir;
  boost::filesystem::path full_path(boost::filesystem::current_path());
  std::string default_plugin_dir = (full_path / "lib").string();
  try {
    // describe program options
    namespace po = boost::program_options;
    po::options_description poDesc("Allowed options", 80);
    poDesc.add_options()
        ("help", "print this help message")
        ("dataflow", po::value<std::string>(&dataflow_file), "Dataflow file for traact network")
        ("loglevel",
         po::value<std::string>(&log_level)->default_value("info"),
         "Set level of verbosity for logging [info|debug|trace].")
        ("plugin_dir",
         po::value<std::string>(&plugin_dir)->default_value(default_plugin_dir),
         "path to traact plugin directory");

    // specify default options
    po::positional_options_description inputOptions;
    inputOptions.add("dataflow", 1);

    // parse options from command line and environment
    po::variables_map poOptions;
    po::store(po::command_line_parser(argc, argv).options(poDesc).positional(inputOptions).run(), poOptions);
    po::store(po::parse_environment(poDesc, "TRAACT_"), poOptions);
    po::notify(poOptions);


    // print help message if nothing specified
    if (poOptions.count("help") || dataflow_file.empty()) {
      std::cout << "Syntax: traactConsole [options]  [--config] <dataflow_file.json>" << std::endl << std::endl;
      std::cout << poDesc << std::endl;
      return 1;
    }
  }
  catch (std::exception &e) {
    std::cerr << "Error parsing command line parameters : " << e.what() << std::endl;
    std::cerr << "Try traactConsole --help for help" << std::endl;
    return 1;
  }

  if (log_level == "info") {
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::info);
    spdlog::info("Set Loglevel to INFO");
  } else if (log_level == "debug") {
    spdlog::set_level(spdlog::level::debug);
    spdlog::flush_on(spdlog::level::debug);
    spdlog::info("Set Loglevel to DEBUG");
  } else if (log_level == "trace") {
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
    spdlog::info("Set Loglevel to TRACE");
    spdlog::trace("Test TRACE message");
  } else {
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
  }

  boost::filesystem::path dataflow_filepath(dataflow_file);
  if (!boost::filesystem::is_regular_file(dataflow_filepath)) {
    spdlog::error("traactConsole: error: - invalid dataflow file: {0}", dataflow_filepath.string());
    return 1;
  }

  Facade facade(plugin_dir);

  facade.loadDataflow(dataflow_file);

  facade.start();

  while (!bStop) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  facade.stop();

  spdlog::info("exit program");

  return 0;
}