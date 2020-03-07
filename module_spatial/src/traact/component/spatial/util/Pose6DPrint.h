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

#ifndef TRAACT_SPATIAL_MODULE_SRC_TRAACT_COMPONENT_SPATIAL_UTIL_POSE6DPRINT_H_
#define TRAACT_SPATIAL_MODULE_SRC_TRAACT_COMPONENT_SPATIAL_UTIL_POSE6DPRINT_H_
#include <sstream>
#include <traact/traact.h>
#include "../../../spatial/spatialPlugin.h"

namespace traact::component::spatial::util {

class Pose6DPrint : public traact::DefaultComponent {
 public:
  explicit Pose6DPrint(const std::string &name) : traact::DefaultComponent(name,
                                                                           traact::component::ComponentType::SyncSink) {
    lastTimestamp = traact::TimestampType::min();
  }

  static traact::pattern::Pattern::Ptr getPattern() {
    traact::pattern::spatial::SpatialPattern::Ptr
        pattern =
        std::make_shared<traact::pattern::spatial::SpatialPattern>("Pose6DPrint", tbb::flow::unlimited);

    pattern->addConsumerPort("input", traact::spatial::type_name::Pose6D);

    return pattern;
  }

  void processTimePoint(traact::DefaultComponentBuffer &data) override {
    using namespace traact::spatial;
    const auto &input = data.getInput<Pose6DHeader::NativeType, Pose6DHeader>(0);

    traact::TimestampType ts = data.getTimestamp();
    if (ts < lastTimestamp) {
      spdlog::warn("current ts: {0} < lastTs: {1}",
                   ts.time_since_epoch().count(),
                   lastTimestamp.time_since_epoch().count());
    }
    /*double expexted = ts.time_since_epoch().count() * (ts.time_since_epoch().count());
    if (expexted != input) {
      spdlog::warn("input differs from output, component: {0} , time: {1}, expected: {2}, value: {3},",
                   getName(),
                   ts.time_since_epoch().count(),
                   expexted,
                   input);
    } else*/
    {

      //if (ts.time_since_epoch().count() == 1000)
      {
        Eigen::IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");
        std::stringstream ss;
        ss << input.matrix().format(CleanFmt);

        spdlog::info("got result for ts: {0}, value: {1}", ts.time_since_epoch().count(), ss.str());
      }

    }

    lastTimestamp = ts;

  }
 protected:
  traact::TimestampType lastTimestamp;
};

}

#endif //TRAACT_SPATIAL_MODULE_SRC_TRAACT_COMPONENT_SPATIAL_UTIL_POSE6DPRINT_H_
