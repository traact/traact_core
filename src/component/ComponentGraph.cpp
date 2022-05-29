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

#include <traact/component/ComponentGraph.h>
#include <traact/util/Logging.h>

#include <exception>

namespace traact::component {

  ComponentGraph::ComponentGraph(pattern::instance::GraphInstance::Ptr graph_instance) : graph_instance_(std::move(
      graph_instance)) {



  }

  std::string ComponentGraph::getName() const {
    return graph_instance_->name;
  }

  void ComponentGraph::addPattern(const std::string &pattern_id, ComponentPtr component) {
    SPDLOG_INFO("Graph add: {0} Component: {1}", getName(), component->getName());

    ModuleComponent::Ptr module_comp_tmp = std::dynamic_pointer_cast<ModuleComponent, Component>(component);

    if(module_comp_tmp){
      SPDLOG_INFO("Graph: {0} Component: {1} is a module component", getName(), component->getName());
      std::string module_key = module_comp_tmp->GetModuleKey();
      if(module_map_[module_key]) {
        SPDLOG_INFO("Graph: {0} Component: {1} module exists", getName(), component->getName());
        module_comp_tmp->SetModule(module_map_[module_key]);
      } else {
        SPDLOG_INFO("Graph: {0} Component: {1} create new module", getName(), component->getName());
        Module::Ptr new_module = module_comp_tmp->InstantiateModule();
        module_comp_tmp->SetModule(new_module);
        module_map_[module_key] = new_module;
      };
    }

    patterns_.emplace(std::make_pair(graph_instance_->getPattern(pattern_id), std::move(component)));

  }
  const std::set<ComponentGraph::PatternComponentPair> &ComponentGraph::getPatterns() const {
    return patterns_;
  }

ComponentGraph::ComponentPtr ComponentGraph::getComponent(const std::string& id) {
    for (const auto &item : patterns_) {
      if (item.first->instance_id == id)
        return item.second;
    }
    return nullptr;
  }

std::set<ComponentGraph::PatternComponentPair> ComponentGraph::getPatternsForTimeDomain(int time_domain) const {
  std::set<ComponentGraph::PatternComponentPair> result;
    for(const auto& pattern : patterns_) {
        switch (pattern.second->getComponentType()) {
            case ComponentType::AsyncSource:
            case ComponentType::AsyncSink:
            case ComponentType::SyncSource:
            case ComponentType::SyncFunctional:
            case ComponentType::AsyncFunctional:
            case ComponentType::SyncSink:
            {
                if(pattern.first->time_domain == time_domain)
                    result.emplace(pattern);
                break;
            }
            case ComponentType::Invalid:{
                throw std::runtime_error("getPatternsForTimeDomain for invalid ComponentType");
            }
            default: {
                throw std::runtime_error("getPatternsForTimeDomain not implemented for ComponentType : "+ std::to_string((int)pattern.second->getComponentType()));
            }

        }
    }
  return std::move(result);
}
std::set<int > ComponentGraph::GetTimeDomains() const {
    std::set<int> all_domains;

    for(const auto& pattern : patterns_){
        all_domains.emplace(pattern.first->time_domain);
    }


  return all_domains;
}

    traact::buffer::TimeDomainManagerConfig ComponentGraph::GetTimeDomainConfig(std::size_t time_domain) const {
      auto result = graph_instance_->timedomain_configs.find(time_domain);
      if(result == graph_instance_->timedomain_configs.end()){
          traact::buffer::TimeDomainManagerConfig config;
          spdlog::error("no valid time domain manager config for time domain {0}", time_domain);
          return config;
      } else {
          return result->second;
      }

    }

}
