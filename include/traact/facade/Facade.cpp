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

#include "Facade.h"
#include <traact/serialization/JsonGraphInstance.h>


#include <traact/util/Logging.h>
#include <stdlib.h>
#include <fstream>




traact::facade::Facade::Facade(PluginFactory::Ptr plugin_factory, dataflow::Network::Ptr dataflow_network)
        : factory_(plugin_factory), network_(dataflow_network) {

    std::set<PluginFactory::FactoryObjectPtr> factory_objects;

    for(const auto& datatype_name : factory_->getDatatypeNames()){
         factory_objects.emplace(factory_->instantiateDataType(datatype_name));
    }
    network_->setGenericFactoryObjects(factory_objects);

}
traact::facade::Facade::~Facade() {

  if (network_)
    network_->stop();
  network_.reset();
  component_graph_.reset();
  graph_instance_.reset();

}
void traact::facade::Facade::loadDataflow(traact::pattern::instance::GraphInstance::Ptr graph_instance) {
  SPDLOG_DEBUG("loading dataflow from graph instance");
  graph_instance_ = graph_instance;

  component_graph_ = std::make_shared<component::ComponentGraph>(graph_instance_);

  for (auto &dataflow_component : graph_instance_->getAll()) {

    try{
      SPDLOG_DEBUG("Create component: {0}", dataflow_component->getPatternName());
      auto newComponent =
          factory_->instantiateComponent(dataflow_component->getPatternName(), dataflow_component->instance_id);
      component_graph_->addPattern(dataflow_component->instance_id, newComponent);



    } catch(std::out_of_range e){
      throw std::out_of_range("trying to instantiate unknown pattern: " + dataflow_component->getPatternName());
    } catch(...) {
      throw std::out_of_range("exception while trying to instantiate pattern: " + dataflow_component->getPatternName());
    }


  }

}
void traact::facade::Facade::loadDataflow(std::string filename) {
  auto loaded_pattern_graph_ptr = std::make_shared<pattern::instance::GraphInstance>();
  nlohmann::json jsongraph;
  std::ifstream graphfile;
  graphfile.open(filename);
  graphfile >> jsongraph;
  graphfile.close();
  ns::from_json(jsongraph, *loaded_pattern_graph_ptr);
  loadDataflow(loaded_pattern_graph_ptr);
}

bool traact::facade::Facade::start() {
  std::set<buffer::BufferFactory::Ptr> generic_factory_objects;
  for (const auto &datatype_name : factory_->getDatatypeNames()) {
    auto newDatatype = factory_->instantiateDataType(datatype_name);
    generic_factory_objects.emplace(newDatatype);
  }

  network_->addComponentGraph(component_graph_);

  network_->start();

  return true;
}
bool traact::facade::Facade::stop() {
  network_->stop();
  return true;
}
traact::component::Component::Ptr traact::facade::Facade::getComponent(std::string id) {
  return component_graph_->getComponent(id);
}
traact::pattern::Pattern::Ptr traact::facade::Facade::instantiatePattern(const std::string &pattern_name) {
  try{
    return factory_->instantiatePattern(pattern_name);
  } catch(...) {
    throw std::invalid_argument("exception trying to instantiate pattern: "+pattern_name);
  }

}

