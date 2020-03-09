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

#include <traact/dataflow/Network.h>

#include "dataflow/intern/NetworkGraph.h"

namespace traact::dataflow {

Network::Network(std::set<buffer::GenericFactoryObject::Ptr> generic_factory_objects)
    : generic_factory_objects_(std::move(generic_factory_objects)) {}

void Network::addComponentGraph(ComponentGraphPtr component_graph) {
  component_graphs_.emplace(std::move(component_graph));
}

void Network::start() {
  network_graphs_.clear();

  for (const ComponentGraphPtr &component_graph : component_graphs_) {
    auto newGraph = std::make_shared<intern::NetworkGraph>(component_graph, 0, generic_factory_objects_);
    network_graphs_.emplace(newGraph);
  }

  for (const auto &network_graph : network_graphs_) {
    network_graph->init();
  }

  for (const auto &network_graph : network_graphs_) {
    network_graph->start();
  }
}

void Network::stop() {
  for (const auto &network_graph : network_graphs_) {
    network_graph->stop();
  }

  for (const auto &network_graph : network_graphs_) {
    network_graph->teardown();
  }
}

}