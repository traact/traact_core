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

#include "NetworkGraph.h"

#include <dataflow/intern/TraactComponentSource.h>
#include <dataflow/intern/TraactComponentSink.h>
#include <dataflow/intern/TraactComponentFunctional.h>

namespace traact::dataflow::intern {
NetworkGraph::NetworkGraph(DefaultComponentGraphPtr component_graph,
                           int time_domain,
                           std::set<buffer::GenericFactoryObject::Ptr> generic_factory_objects)
    : component_graph_(std::move(
    component_graph)), time_domain_(time_domain), generic_factory_objects_(std::move(generic_factory_objects)) {};

void NetworkGraph::init() {
  //component_graph_->checkGraph();

  time_domain_manager_.reset(new buffer::TimeDomainManager(3, generic_factory_objects_));

  time_domain_manager_->init(component_graph_);

  for (const auto &base_component : component_graph_->getPatterns()) {
    DefaultPatternPtr pattern = base_component.first;
    if (!pattern) {
      spdlog::error("Skipping non dataflow pattern: {0}", base_component.first->instance_id);
      continue;
    }

    DefaultComponentPtr component = std::dynamic_pointer_cast<DefaultComponent>(base_component.second);
    if (!component) {
      spdlog::error("Skipping non dataflow component: {0}", base_component.second->getName());
      continue;
    }

    TraactComponentBasePtr newComponent;

    switch (component->getComponentType()) {
      case component::ComponentType::AsyncSource: {
        newComponent = std::make_shared<TraactComponentSource>(pattern,
                                                               component,
                                                               time_domain_manager_,
                                                               this);
        break;
      }
      case component::ComponentType::Functional: {
        newComponent = std::make_shared<TraactComponentFunctional>(pattern,
                                                                   component,
                                                                   time_domain_manager_,
                                                                   this);
        break;
      }
      case component::ComponentType::SyncSink: {
        newComponent =
            std::make_shared<TraactComponentSink>(pattern, component, time_domain_manager_, this);
        break;
      }
      default: {
        throw std::invalid_argument("unsupported traact network component type");
      }

    }

    for (auto port : pattern->getProducerPorts()) {
      port_to_network_component[port] = newComponent;
    }

    for (auto port : pattern->getConsumerPorts()) {
      port_to_network_component[port] = newComponent;
    }

    network_components_.emplace(newComponent);

  }

  for (const auto &component : network_components_) {
    component->init();
  }
}
void NetworkGraph::start() {
  for (const auto &component : network_components_) {
    component->connect();
  }
  for (const auto &component : network_components_) {
    component->start();
  }
}
void NetworkGraph::stop() {
  for (const auto &component : network_components_) {
    component->stop();
  }

  for (const auto &component : network_components_) {
    component->disconnect();
  }

  graph_.wait_for_all();
}
void NetworkGraph::teardown() {
  for (const auto &component : network_components_) {
    component->teardown();
  }
  network_components_.clear();
  //graph_.reset();
}

tbb::flow::graph &NetworkGraph::getTBBGraph() {
  return graph_;
}

tbb::flow::sender<TraactMessage> &NetworkGraph::getSender(PortPtr port) {
  return port_to_network_component[port]->getSender(port->getPortIndex());
}
tbb::flow::receiver<TraactMessage> &NetworkGraph::getReceiver(PortPtr port) {
  return port_to_network_component[port]->getReceiver(port->getPortIndex());
}
}