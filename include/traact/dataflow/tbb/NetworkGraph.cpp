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

#include "ComponentAsyncSource.h"
#include "ComponentSyncSource.h"
#include "ComponentSyncSink.h"
#include "ComponentSyncFunctional.h"
#include "ComponentAsyncFunctional.h"
#include "ComponentAsyncSink.h"

#include "TBBTimeDomainManager.h"



namespace traact::dataflow {
NetworkGraph::NetworkGraph(DefaultComponentGraphPtr component_graph,
                           std::set<buffer::BufferFactory::Ptr> generic_factory_objects)
    : component_graph_(std::move(
    component_graph)), generic_factory_objects_(std::move(generic_factory_objects)) {};

void NetworkGraph::init() {
  //component_graph_->checkGraph();

  auto time_domains = component_graph_->GetTimeDomains();
  std::set<std::string> master_sources;
  for(auto time_domain : time_domains) {

     auto td_config = component_graph_->GetTimeDomainConfig(time_domain);

     master_sources.emplace(td_config.master_source);

    time_domain_manager_.emplace(time_domain, std::make_shared<TBBTimeDomainManager>(td_config, generic_factory_objects_, this));
  }




  for (const auto &base_component : component_graph_->getPatterns()) {
    DefaultPatternPtr pattern = base_component.first;
    if (!pattern) {
      spdlog::error("Skipping non dataflow pattern");
      continue;
    }

    DefaultComponentPtr component = std::dynamic_pointer_cast<DefaultComponent>(base_component.second);
    if (!component) {
      spdlog::error("Skipping non dataflow component");
      continue;
    }

    TraactComponentBasePtr newComponent;

    auto& tdm_component = time_domain_manager_[pattern->time_domain];

    switch (component->getComponentType()) {
      case component::ComponentType::AsyncSource: {
        newComponent = std::make_shared<ComponentAsyncSource>(pattern,
                                                                    component,
                                                                    tdm_component.get(),
                                                                    this);
        newComponent->setSourceFinishedCallback(std::bind(&NetworkGraph::MasterSourceFinished, this));
        break;
      }
        case component::ComponentType::SyncSource: {
            newComponent = std::make_shared<ComponentSyncSource>(pattern,
                                                                        component,
                                                                        tdm_component.get(),
                                                                        this);

            break;
        }
      case component::ComponentType::SyncFunctional: {
        newComponent = std::make_shared<ComponentSyncFunctional>(pattern,
                                                                   component,
                                                                   tdm_component.get(),
                                                                   this);
        break;
      }
        case component::ComponentType::AsyncFunctional: {
            newComponent = std::make_shared<ComponentAsyncFunctional>(pattern,
                                                                     component,
                                                                     tdm_component.get(),
                                                                     this);
            break;
        }
      case component::ComponentType::SyncSink: {
        newComponent =
            std::make_shared<ComponentSyncSink>(pattern, component, tdm_component.get(), this);
        break;
      }
        case component::ComponentType::AsyncSink: {
            newComponent =
                    std::make_shared<ComponentAsyncSink>(pattern, component, tdm_component.get(), this);
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


    //if(master_sources.count(component->getName())) {
    //    newComponent->setSourceFinishedCallback(std::bind(&NetworkGraph::MasterSourceFinished, this));
    //}

    network_components_.emplace_back(newComponent);

  }

  for (const auto &component : network_components_) {
    component->init();
  }

   for (const auto &component : network_components_) {
        component->connect();
   }

  for(auto& td_manager : time_domain_manager_){
      td_manager.second->Init(component_graph_);
  }

    for(auto& td_manager : time_domain_manager_){
        td_manager.second->Configure();
    }

    graph_.wait_for_all();




}
void NetworkGraph::start() {
    running_ = true;
    for(auto& tdm : time_domain_manager_){
        tdm.second->Start();
    }
}
void NetworkGraph::stop() {

    running_ = false;

    for(auto& tdm : time_domain_manager_) {
        tdm.second->Stop();
    }

    graph_.wait_for_all();

}
void NetworkGraph::teardown() {

    for(auto& tdm : time_domain_manager_){
        tdm.second->Teardown();
    }

    graph_.wait_for_all();

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

    tbb::flow::receiver<TraactMessage> &NetworkGraph::getSourceReceiver(const std::string &component_name) {
        for (const auto &component : network_components_) {
            if(component->GetName() == component_name)
                return component->getReceiver(0);
        }
        throw std::invalid_argument(fmt::format("unknown source {0}", component_name));
    }

    tbb::flow::sender<TraactMessage> &NetworkGraph::getSender(const std::string &component_name) {
        for (const auto &component : network_components_) {
            if(component->GetName() == component_name)
                return component->getSender(0);
        }
        throw std::invalid_argument(fmt::format("unknown source {0}", component_name));
    }

    bool NetworkGraph::IsRunning() {
        return running_;
    }

    void NetworkGraph::setMasterSourceFinishedCallback(component::Component::SourceFinishedCallback callback) {
        source_finished_callback = callback;
    }

    void NetworkGraph::MasterSourceFinished() {
    // first finished call ends playback
    if(source_finished_.test_and_set()) {
            return;
        }

        //finished_count_++;
        //if(finished_count_ == time_domain_manager_.size())
            source_finished_callback();
    }
}