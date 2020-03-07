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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#ifndef TRAACTTEST_SRC_TRAACT_NETWORK_TRAACTCOMPONENTBASE_H_
#define TRAACTTEST_SRC_TRAACT_NETWORK_TRAACTCOMPONENTBASE_H_

#include <tbb/flow_graph.h>

#include <traact/datatypes.h>
#include <traact/component/Component.h>
#include <traact/pattern/instance/PatternInstance.h>
#include <traact/buffer/TimeDomainManager.h>
#include <traact/buffer/GenericComponentBuffer.h>

namespace traact::dataflow::intern {
class NetworkGraph;
}

namespace traact::dataflow::intern {

class TraactComponentBase {

 public:
  typedef typename std::shared_ptr<TraactComponentBase> Ptr;
  typedef typename pattern::instance::PatternInstance::Ptr DefaultPatternPtr;
  typedef typename component::Component::Ptr DefaultComponentPtr;
  typedef typename buffer::TimeDomainManager::Ptr DefaultTimeDomainManagerPtr;
  typedef typename buffer::GenericComponentBuffer DefaultComponentBuffer;

  TraactComponentBase(DefaultPatternPtr pattern_base,
                      DefaultComponentPtr component_base,
                      DefaultTimeDomainManagerPtr buffer_manager,
                      NetworkGraph *network_graph)
      : pattern_base_(std::move(pattern_base)),
        component_base_(std::move(component_base)),
        buffer_manager_(std::move(buffer_manager)),
        network_graph_(network_graph) {};
  virtual ~TraactComponentBase() = default;;

  virtual bool init() {
    component_base_->init();
    return true;
  }

  virtual void connect() = 0;

  virtual bool start() {
    component_base_->start();
    return true;
  }

  virtual bool stop() {
    component_base_->stop();
    return true;
  }

  virtual void disconnect() = 0;

  virtual bool teardown() {
    component_base_->teardown();
    return true;
  }

  virtual tbb::flow::sender<TraactMessage> &getSender(int index) {
    throw std::runtime_error("trying to get sender port of component with no senders");
  }
  virtual tbb::flow::receiver<TraactMessage> &getReceiver(int index) {
    throw std::runtime_error("trying to get receiever port of component with no receievers");
  }

 protected:
  DefaultTimeDomainManagerPtr buffer_manager_;
  DefaultComponentPtr component_base_;
  DefaultPatternPtr pattern_base_;
  NetworkGraph *network_graph_;

};
}
#endif //TRAACTTEST_SRC_TRAACT_NETWORK_TRAACTCOMPONENTBASE_H_

#pragma clang diagnostic pop