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

#ifndef TRAACT_SPATIAL_MODULE_SRC_TRAACT_COMPONENT_SPATIAL_UTIL_POSE6DTESTSOURCE_H_
#define TRAACT_SPATIAL_MODULE_SRC_TRAACT_COMPONENT_SPATIAL_UTIL_POSE6DTESTSOURCE_H_

#include <traact/traact.h>
#include "../../../spatial/spatialPlugin.h"

namespace traact::component::spatial::util {
class Pose6DTestSource : public Component {
 public:
  explicit Pose6DTestSource(const std::string &name) : Component(name, traact::component::ComponentType::AsyncSource) {
    internal_data_.setIdentity();
    running_ = false;
  }

  static traact::pattern::Pattern::Ptr getPattern() {
    traact::pattern::spatial::SpatialPattern::Ptr
        pattern =
        std::make_shared<traact::pattern::spatial::SpatialPattern>("Pose6DTestSource", tbb::flow::unlimited);

    pattern->addProducerPort("output", traact::spatial::type_name::Pose6D);
    pattern->addCoordianteSystem("Origin")
        .addCoordianteSystem("Target")
        .addEdge("Origin", "Target", "output");

    return pattern;
  }

  bool start() override {
    running_ = true;
    spdlog::info("starting simple source");
    thread_.reset(new std::thread(std::bind(&Pose6DTestSource::threadLoop, this)));
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
  Eigen::Affine3d internal_data_;
  std::shared_ptr<std::thread> thread_;
  bool running_;

  void threadLoop() {
    using namespace traact::spatial;
    using namespace traact;
    TimestampType ts = traact::TimestampType::min();
    //TimestampType ts = now();
    TimeDurationType deltaTs = std::chrono::milliseconds(5);

    int output_count = 0;
    internal_data_ = internal_data_ * Eigen::Translation3d(0, 0, 1);

    while (running_ && output_count < 1000) {
      std::this_thread::sleep_for(deltaTs);
      ts += std::chrono::nanoseconds(1);

      spdlog::trace("request buffer");

      if (request_callback_(ts) != 0)
        continue;

      auto &buffer = acquire_callback_(ts);
      auto &newData = buffer.getOutput<Pose6DHeader::NativeType, Pose6DHeader>(0);

      newData = internal_data_;

      spdlog::trace("commit data");
      commit_callback_(ts);

      internal_data_ = internal_data_ * Eigen::Translation3d(0, 0, 1);
      output_count++;

      spdlog::trace("done");
    }
    spdlog::trace("source quit loop");
    running_ = false;
  }
};

}

#endif //TRAACT_SPATIAL_MODULE_SRC_TRAACT_COMPONENT_SPATIAL_UTIL_POSE6DTESTSOURCE_H_
