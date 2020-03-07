#pragma clang diagnostic push
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

#include <spdlog/spdlog.h>

#include <traact/traact.h>
#include <thread>
#include <traact/util/PerformanceMonitor.h>

#include <Eigen/Geometry>

#include <traact/DoubleType.h>

class SimpleSourceComponent : public traact::DefaultComponent {
 public:
  explicit SimpleSourceComponent(const std::string &name) : traact::DefaultComponent(name,
                                                                                     traact::component::ComponentType::AsyncSource) {
    internal_data_ = 0;
    running_ = false;
  }

  static traact::pattern::Pattern::Ptr getPattern() {
    traact::pattern::spatial::SpatialPattern::Ptr
        pattern =
        std::make_shared<traact::pattern::spatial::SpatialPattern>("SimpleSourceComponent", tbb::flow::unlimited);

    pattern->addProducerPort("output", traact::type_name::TestDouble);

    return pattern;
  }

  bool start() override {
    running_ = true;
    spdlog::info("starting simple source");
    thread_.reset(new std::thread(std::bind(&SimpleSourceComponent::threadLoop, this)));
  }
  bool stop() override {
    spdlog::info("stopping simple source");
    if (running_) {
      running_ = false;
      thread_->join();
    }

  }

  void waitForFinish() {
    thread_->join();
  }

 private:
  double internal_data_;
  std::shared_ptr<std::thread> thread_;
  bool running_;

  void threadLoop() {
    traact::TimestampType ts = traact::TimestampType::min();
    //TimestampType ts = now();
    traact::TimeDurationType deltaTs = std::chrono::milliseconds(5);

    int output_count = 0;
    internal_data_ = 1;

    while (running_ && output_count < 1000) {
      std::this_thread::sleep_for(deltaTs);
      ts += std::chrono::nanoseconds(1);

      spdlog::trace("request buffer");

      if (request_callback_(ts) != 0)
        continue;

      auto &buffer = acquire_callback_(ts);
      auto &newData = buffer.getOutput<double, traact::DoubleHeader>(0);

      newData = internal_data_;

      spdlog::trace("commit data");
      commit_callback_(ts);

      internal_data_++;
      output_count++;

      spdlog::trace("done");
    }
    spdlog::trace("source quit loop");
    running_ = false;
  }
};

class PrintSinkComponent : public traact::DefaultComponent {
 public:
  explicit PrintSinkComponent(const std::string &name) : traact::DefaultComponent(name,
                                                                                  traact::component::ComponentType::SyncSink) {
    lastTimestamp = traact::TimestampType::min();
  }

  static traact::pattern::Pattern::Ptr getPattern() {
    traact::pattern::spatial::SpatialPattern::Ptr
        pattern =
        std::make_shared<traact::pattern::spatial::SpatialPattern>("PrintSinkComponent", tbb::flow::unlimited);

    pattern->addConsumerPort("input", traact::type_name::TestDouble);

    return pattern;
  }

  void processTimePoint(traact::DefaultComponentBuffer &data) override {

    const auto &input = data.getInput<double, traact::DoubleHeader>(0);

    traact::TimestampType ts = data.getTimestamp();
    if (ts < lastTimestamp) {
      spdlog::warn("current ts: {0} < lastTs: {1}",
                   ts.time_since_epoch().count(),
                   lastTimestamp.time_since_epoch().count());
    }
    double expexted = static_cast<double>( ts.time_since_epoch().count() * (ts.time_since_epoch().count()));
    if (expexted != input) {
      spdlog::warn("input differs from output, component: {0} , time: {1}, expected: {2}, value: {3},",
                   getName(),
                   ts.time_since_epoch().count(),
                   expexted,
                   input);
    } else {
      if (ts.time_since_epoch().count() == 1000)
        spdlog::info("got result for ts: {0}, value: {1}", ts.time_since_epoch().count(), input);
    }

    lastTimestamp = ts;

  }
 protected:
  traact::TimestampType lastTimestamp;
};

class MulComponent : public traact::DefaultComponent {
 public:
  explicit MulComponent(const std::string &name) : traact::DefaultComponent(name,
                                                                            traact::component::ComponentType::Functional) {
  }

  static traact::pattern::Pattern::Ptr getPattern() {
    traact::pattern::spatial::SpatialPattern::Ptr
        pattern = std::make_shared<traact::pattern::spatial::SpatialPattern>("Multiplication", tbb::flow::unlimited);

    pattern->addConsumerPort("input0", traact::type_name::TestDouble)
        .addConsumerPort("input1", traact::type_name::TestDouble)
        .addProducerPort("output", traact::type_name::TestDouble);

    return pattern;
  }

  void processTimePoint(traact::DefaultComponentBuffer &data) override {

    const auto &input0 = data.getInput<double, traact::DoubleHeader>(0);
    const auto &input1 = data.getInput<double, traact::DoubleHeader>(1);
    auto &output = data.getOutput<double, traact::DoubleHeader>(0);

    //std::this_thread::sleep_for(std::chrono::milliseconds(10));

    traact::TimestampType ts = traact::now();
    traact::TimestampType finish = ts + std::chrono::milliseconds(10);
    while (ts < finish)
      ts = traact::now();

    output = input0 * input1;

  }
};

int main() {
  using namespace traact;
  using namespace traact::dataflow;
  /*spdlog::set_pattern("[source %s] [function %!] [line %#] %v");

  auto console = spdlog::stdout_logger_mt("console");
  spdlog::set_default_logger(console);
  SPDLOG_TRACE("global output with arg {}", 3); // [source main.cpp] [function main] [line 16] global output with arg 1
  SPDLOG_LOGGER_TRACE(console, "logger output with arg {}", 3); // [source main.cpp] [function main] [line 17] logger output with arg 2
  console->info("invoke member function"); // [source ] [function ] [line ] invoke member function*/

  util::PerformanceMonitor monitor("test");
  // add datatypes
  std::set<buffer::GenericFactoryObject::Ptr> generic_factory_objects;
  generic_factory_objects.emplace(std::make_shared<DoubleFactoryObject>());
  DefaultTraactNetwork network(generic_factory_objects);

  // create dataflow configuraiton
  DefaultInstanceGraphPtr pattern_graph_ptr = std::make_shared<DefaultInstanceGraph>("test1");

  // create instances of patterns
  DefaultPatternInstancePtr
      source_pattern = pattern_graph_ptr->addPattern("source", SimpleSourceComponent::getPattern());
  DefaultPatternInstancePtr
      source2_pattern = pattern_graph_ptr->addPattern("source2", SimpleSourceComponent::getPattern());
  DefaultPatternInstancePtr sink_pattern = pattern_graph_ptr->addPattern("sink", PrintSinkComponent::getPattern());
  DefaultPatternInstancePtr mul_pattern = pattern_graph_ptr->addPattern("mul1", MulComponent::getPattern());

  // connect
  pattern_graph_ptr->connect("source", "output", "mul1", "input0");
  pattern_graph_ptr->connect("source2", "output", "mul1", "input1");
  pattern_graph_ptr->connect("mul1", "output", "sink", "input");



  // create components
  DefaultComponentGraphPtr component_graph = std::make_shared<DefaultComponentGraph>(pattern_graph_ptr);
  std::shared_ptr<SimpleSourceComponent> source = std::make_shared<SimpleSourceComponent>(source_pattern->instance_id);
  std::shared_ptr<SimpleSourceComponent> source2 = std::make_shared<SimpleSourceComponent>("source2");
  DefaultComponentPtr sink = std::make_shared<PrintSinkComponent>("sink");
  DefaultComponentPtr mul_component = std::make_shared<MulComponent>("mul1");
  component_graph->addPattern(source_pattern->instance_id, source);
  component_graph->addPattern(source2_pattern->instance_id, source2);
  component_graph->addPattern(sink_pattern->instance_id, sink);
  component_graph->addPattern(mul_pattern->instance_id, mul_component);




  // add to network
  network.addComponentGraph(component_graph);

  {
    MEASURE_TIME(monitor, 0, "network start")
    network.start();
  }

  {
    MEASURE_TIME(monitor, 1, "wait for finish source")
    source->waitForFinish();
    source2->waitForFinish();
  }

  {
    MEASURE_TIME(monitor, 2, "finish and wait for network stop")
    network.stop();
  }

  spdlog::info(monitor.toString());

  spdlog::info("exit program");

  return 0;
}

#pragma clang diagnostic pop