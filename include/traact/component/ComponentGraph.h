#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nodiscard"
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

#ifndef TRAACT_INCLUDE_TRAACT_DATAFLOW_DATAFLOWGRAPH_H_
#define TRAACT_INCLUDE_TRAACT_DATAFLOW_DATAFLOWGRAPH_H_

#include <memory>
#include <set>

#include <traact/component/Component.h>
#include <traact/pattern/instance/PatternInstance.h>
#include <traact/pattern/instance/GraphInstance.h>

namespace traact::component {

class ComponentGraph {
 public:
  typedef typename std::shared_ptr<ComponentGraph> Ptr;
  typedef typename std::shared_ptr<pattern::instance::PatternInstance> PatternPtr;
  typedef typename std::shared_ptr<Component> ComponentPtr;
  typedef typename std::pair<PatternPtr, ComponentPtr> PatternComponentPair;

  explicit ComponentGraph(pattern::instance::GraphInstance::Ptr graph_instance) : graph_instance_(std::move(
      graph_instance)) {

  }

  std::string getName() const {
    return graph_instance_->name;
  }

  void addPattern(const std::string &pattern_id, ComponentPtr component) {
    patterns_.emplace(std::make_pair(graph_instance_->getPattern(pattern_id), std::move(component)));
  }
  const std::set<PatternComponentPair> &getPatterns() const {
    return patterns_;
  }

  ComponentPtr getComponent(std::string id) {
    for (const auto &item : patterns_) {
      if (item.first->instance_id == id)
        return item.second;
    }
    return nullptr;
  }

 private:
  pattern::instance::GraphInstance::Ptr graph_instance_;
  std::set<PatternComponentPair> patterns_;
};
}

#endif //TRAACT_INCLUDE_TRAACT_DATAFLOW_DATAFLOWGRAPH_H_

#pragma clang diagnostic pop