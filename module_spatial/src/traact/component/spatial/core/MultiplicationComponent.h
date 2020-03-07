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

#ifndef TRAACT_CORECOMPONENTS_SRC_TRAACT_MULTIPLICATIONCOMPONENT_H_
#define TRAACT_CORECOMPONENTS_SRC_TRAACT_MULTIPLICATIONCOMPONENT_H_

#include <traact/traact.h>
#include "../../../spatial/spatialPlugin.h"

namespace traact::component::spatial::core {

class MultiplicationComponent : public Component {
 public:
  explicit MultiplicationComponent(const std::string &name) : Component(name, ComponentType::Functional) {}

  static pattern::Pattern::Ptr getPattern() {
    pattern::spatial::SpatialPattern::Ptr
        pattern = std::make_shared<pattern::spatial::SpatialPattern>("MultiplicationComponent", tbb::flow::unlimited);

    pattern->addConsumerPort("input0", traact::spatial::type_name::Pose6D)
        .addConsumerPort("input1", traact::spatial::type_name::Pose6D)
        .addProducerPort("output", traact::spatial::type_name::Pose6D);

    pattern->addCoordianteSystem("A")
        .addCoordianteSystem("B")
        .addCoordianteSystem("C")
        .addEdge("A", "B", "input0")
        .addEdge("B", "C", "input1")
        .addEdge("A", "C", "output");

    return
        pattern;
  };

  void processTimePoint(DefaultComponentBuffer &data)
  override;
};

}

#endif //TRAACT_CORECOMPONENTS_SRC_TRAACT_MULTIPLICATIONCOMPONENT_H_
