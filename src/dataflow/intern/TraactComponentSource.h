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

#ifndef TRAACTTEST_SRC_TRAACT_NETWORK_TRAACTCOMPONENTSOURCE_H_
#define TRAACTTEST_SRC_TRAACT_NETWORK_TRAACTCOMPONENTSOURCE_H_

#include <thread>
#include <tbb/flow_graph.h>

#include "TraactComponentBase.h"

namespace traact::dataflow::intern {

class TraactComponentSource : public TraactComponentBase, buffer::BufferSource {
 public:
  TraactComponentSource(DefaultPatternPtr pattern_base,
                        DefaultComponentPtr component_base,
                        DefaultTimeDomainManagerPtr buffer_manager,
                        NetworkGraph *network_graph);


  bool init() override;
  bool start() override;
  bool stop() override;
  bool teardown() override;
  component::ComponentType getComponentType() override;

  tbb::flow::sender<TraactMessage> &getSender(int index) override;

  void connect() override;
  void disconnect() override;

  int configure_component(TimestampType ts);

    int invalidateBuffer(TimestampType ts, buffer::GenericTimeDomainBuffer* buffer) override;
    std::string getComponentName() override;

    bool isMaster();

 protected:
  typedef tbb::flow::async_node<tbb::flow::continue_msg, TraactMessage> async_source_node;

  tbb::flow::async_node<TraactMessage, TraactMessage> *node_;
  //tbb::flow::broadcast_node<TraactMessage> *broadcast_node_;

  TimestampType requestBuffer(TimestampType ts) {
    return this->buffer_manager_->requestBuffer(ts, component_base_->getName());
  }
  DefaultComponentBuffer &acquireBuffer(TimestampType ts) {
      DefaultComponentBuffer &buffer = buffer_manager_->acquireBuffer(ts, component_base_->getName());

    return buffer;
  }
  int commitData(TimestampType ts);



};

}

#endif //TRAACTTEST_SRC_TRAACT_NETWORK_TRAACTCOMPONENTSOURCE_H_
