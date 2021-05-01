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
//#include <cppfs/FileHandle.h>
#include <traact/traact.h>
#include <traact/facade/DefaultFacade.h>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>
#include "spdlog/sinks/basic_file_sink.h"
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
      auto filesink = std::make_shared<spdlog::sinks::basic_file_sink_mt >("traact_log.txt");

      spdlog::sinks_init_list sinks{filesink, console_sink};
      auto logger = std::make_shared<spdlog::logger>("console", sinks);
      spdlog::set_default_logger(logger);
      traact::util::setup_logger(logger);

  }
  catch (const spdlog::spdlog_ex &ex) {
    std::cout << "Log initialization failed: " << ex.what() << std::endl;
  }

  // program options
  std::string dataflow_file;
  std::string log_level = "info";


  try {
      if(argc < 2){
          std::cout << "Syntax: traactConsole [--loglevel info|debug|trace] <dataflow_file.json>" << std::endl << std::endl;
          return 1;
      }
      if(argc == 4) {
          log_level = std::string(argv[2]);
          dataflow_file = std::string(argv[3]);
      } else {
          dataflow_file = std::string(argv[1]);
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





  DefaultFacade facade;

  facade.loadDataflow(dataflow_file);

  facade.start();

  while (!bStop) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  facade.stop();

  spdlog::info("exit program");

  return 0;
}