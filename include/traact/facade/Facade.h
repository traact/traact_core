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

#ifndef TRAACT_INCLUDE_TRAACT_FACADE_FACADE_H_
#define TRAACT_INCLUDE_TRAACT_FACADE_FACADE_H_

#include <traact/pattern/instance/GraphInstance.h>
#include <traact/dataflow/Network.h>
#include <traact/component/ComponentGraph.h>
#include <traact/facade/PluginFactory.h>
#include <future>

namespace traact::facade {

class TRAACT_CORE_EXPORT Facade {
 public:

  Facade(PluginFactory::Ptr plugin_factory, dataflow::Network::Ptr dataflow_network);
  ~Facade();
  void loadDataflow(pattern::instance::GraphInstance::Ptr graph_instance);
  void loadDataflow(std::string filename);

  bool start();
  bool blockingStart();

  std::shared_future<void> getFinishedFuture();

  bool stop();

  pattern::Pattern::Ptr instantiatePattern(const std::string &pattern_name);

  component::Component::Ptr getComponent(std::string id);

 private:
  std::shared_ptr<PluginFactory> factory_;
  pattern::instance::GraphInstance::Ptr graph_instance_;
  component::ComponentGraph::Ptr component_graph_;
  dataflow::Network::Ptr network_;
  bool should_stop_{false};
  std::promise<void> finished_promise_;
  std::shared_future<void> finished_future_;

  void MasterSourceFinished();
};
}

#endif //TRAACT_INCLUDE_TRAACT_FACADE_FACADE_H_
