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

#ifndef TRAACT_INCLUDE_TRAACT_DATAFLOW_NETWORK_H_
#define TRAACT_INCLUDE_TRAACT_DATAFLOW_NETWORK_H_

#include <memory>
#include <map>
#include <traact/buffer/GenericFactoryObject.h>
#include <traact/component/ComponentGraph.h>

#include <traact/traact_core_export.h>

namespace traact::dataflow::intern {
class TRAACT_CORE_EXPORT NetworkGraph;
}

namespace traact::dataflow {

class TRAACT_CORE_EXPORT Network {
 public:
  typedef typename std::shared_ptr<Network> Ptr;
  typedef typename component::ComponentGraph::Ptr ComponentGraphPtr;

  explicit Network(std::set<buffer::GenericFactoryObject::Ptr> generic_factory_objects);

  void addComponentGraph(ComponentGraphPtr component_graph);

  void start();

  void stop();
 private:
  typedef typename std::shared_ptr<intern::NetworkGraph> NetworkGraphPtr;

  std::set<ComponentGraphPtr> component_graphs_;
  std::set<NetworkGraphPtr> network_graphs_;
  std::set<buffer::GenericFactoryObject::Ptr> generic_factory_objects_;

};
}

#endif //TRAACT_INCLUDE_TRAACT_DATAFLOW_NETWORK_H_
