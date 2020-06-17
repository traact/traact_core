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

#ifndef TRAACTTEST_SRC_TRAACT_NETWORK_DYNAMICJOINNODE_H_
#define TRAACTTEST_SRC_TRAACT_NETWORK_DYNAMICJOINNODE_H_

#include <tbb/flow_graph.h>
#include <traact/datatypes.h>
#include <tuple>

namespace traact::dataflow::intern {

class DynamicJoinNode {
 public:

  DynamicJoinNode(tbb::flow::graph &graph, int num_inputs) : num_inputs_(num_inputs), join_node_(nullptr) {
    using namespace tbb::flow;
    switch (num_inputs_) {
      case 2: {
        // TODO key matching
        auto join_p = new join_node<std::tuple<TraactMessage, TraactMessage>, key_matching<uint64_t> >(graph);
        join_node_ = join_p;
        auto func_p = new function_node<tuple<TraactMessage, TraactMessage>, TraactMessage>(graph,
                                                                                            unlimited,
                                                                                            std::bind(&DynamicJoinNode::join2,
                                                                                                      this,
                                                                                                      std::placeholders::_1));
        function_node_ = func_p;
        make_edge(*join_p, *func_p);
        break;
      }
      case 3: {
        auto join_p =
            new join_node<std::tuple<TraactMessage, TraactMessage, TraactMessage>, key_matching<uint64_t> >(graph);
        join_node_ = join_p;
        auto func_p = new function_node<tuple<TraactMessage, TraactMessage, TraactMessage>, TraactMessage>(graph,
                                                                                                           unlimited,
                                                                                                           std::bind(&DynamicJoinNode::join3,
                                                                                                                     this,
                                                                                                                     std::placeholders::_1));
        function_node_ = func_p;
        make_edge(*join_p, *func_p);
        break;
      }
    }
  }
  virtual ~DynamicJoinNode() {
    using namespace tbb::flow;

    switch (num_inputs_) {
      case 2: {
        auto *tmp =
            static_cast<join_node<std::tuple<TraactMessage, TraactMessage>, key_matching<uint64_t> > * >(join_node_);
        delete tmp;
        auto *tmp2 =
            static_cast<function_node<tuple<TraactMessage, TraactMessage>, TraactMessage> * >(function_node_);
        delete tmp2;
        break;
      }
      case 3: {
        auto *tmp =
            static_cast<join_node<std::tuple<TraactMessage, TraactMessage, TraactMessage>,
                                  key_matching<uint64_t> > * >(join_node_);
        delete tmp;
        auto *tmp2 =
            static_cast<function_node<tuple<TraactMessage, TraactMessage, TraactMessage>,
                                      TraactMessage> * >(function_node_);
        delete tmp2;
        break;
      }
      default:break;

    }

  }

  tbb::flow::sender<TraactMessage> &getSender() {
    using namespace tbb::flow;
    switch (num_inputs_) {
      case 2: {
        auto *tmp =
            static_cast<function_node<std::tuple<TraactMessage, TraactMessage>, TraactMessage> * >(function_node_);
        return *tmp;
      }
      case 3: {
        auto *tmp =
            static_cast<function_node<std::tuple<TraactMessage, TraactMessage, TraactMessage>,
                                      TraactMessage> * >(function_node_);
        return *tmp;
      }
	  default:throw std::invalid_argument("unsupported number of senders used");
    }

  }

  tbb::flow::receiver<TraactMessage> &getReceiver(int index) {
    using namespace tbb::flow;

    switch (num_inputs_) {
      case 2: {
        switch (index) {
          case 0: {
            auto *tmp =
                static_cast<join_node<std::tuple<TraactMessage, TraactMessage>,
                                      key_matching<uint64_t> > * >(join_node_);
            return input_port<0>(*tmp);
          }
          case 1: {
            auto *tmp =
                static_cast<join_node<std::tuple<TraactMessage, TraactMessage>,
                                      key_matching<uint64_t> > * >(join_node_);
            return input_port<1>(*tmp);
          }
          default:throw std::invalid_argument("unsupported index for receiver of dynamic join node");
        }
      }
      case 3: {
        switch (index) {
          case 0: {
            auto *tmp =
                static_cast<join_node<std::tuple<TraactMessage, TraactMessage, TraactMessage>,
                                      key_matching<uint64_t> > * >(join_node_);
            return input_port<0>(*tmp);
          }
          case 1: {
            auto *tmp =
                static_cast<join_node<std::tuple<TraactMessage, TraactMessage, TraactMessage>,
                                      key_matching<uint64_t> > * >(join_node_);
            return input_port<1>(*tmp);
          }
          case 2: {
            auto *tmp =
                static_cast<join_node<std::tuple<TraactMessage, TraactMessage, TraactMessage>,
                                      key_matching<uint64_t> > * >(join_node_);
            return input_port<2>(*tmp);
          }
          default:throw std::invalid_argument("unsupported index for receiver of dynamic join node");
        }

      }
      default:throw std::invalid_argument("unsupported numver of inputs for dynamic join node");
    }

  }

  TraactMessage join2(const std::tuple<TraactMessage, TraactMessage> &in) {
    __TBB_ASSERT(std::get<0>(in).timestamp == std::get<1>(in).timestamp, "timestamps of sync input differ");
    TraactMessage result= std::get<0>(in);
    result.valid = std::get<0>(in).valid && std::get<1>(in).valid;
    return result;
  }
  TraactMessage join3(const std::tuple<TraactMessage, TraactMessage, TraactMessage> &in) {
    TraactMessage result = std::get<0>(in);
    result.valid = std::get<0>(in).valid && std::get<1>(in).valid && std::get<2>(in).valid;
    return result;
  }
 private:
  void *join_node_;
  void *function_node_;

  const int num_inputs_;

};

}

#endif //TRAACTTEST_SRC_TRAACT_NETWORK_DYNAMICJOINNODE_H_
