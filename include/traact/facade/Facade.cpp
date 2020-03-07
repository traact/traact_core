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
#include <traact/util/Utils.h>
#include <traact/serialization/JsonGraphInstance.h>

traact::facade::Facade::Facade(std::string plugin_directory) : plugin_directory_(std::move(plugin_directory)) {
  std::vector<std::string> files = util::glob_files(plugin_directory_);
  for (const auto &file : files) {
    factory_.addLibrary(file);
  }

}
traact::facade::Facade::~Facade() {

  if (network_)
    network_->stop();
  network_.reset();
  component_graph_.reset();
  graph_instance_.reset();

}
void traact::facade::Facade::loadDataflow(traact::pattern::instance::GraphInstance::Ptr graph_instance) {
  graph_instance_ = graph_instance;

  component_graph_ = std::make_shared<component::ComponentGraph>(graph_instance_);

  for (auto &dataflow_component : graph_instance_->getAll()) {

    auto newComponent =
        factory_.instantiateComponent(dataflow_component->getPatternName(), dataflow_component->instance_id);
    component_graph_->addPattern(dataflow_component->instance_id, newComponent);

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
  std::set<buffer::GenericFactoryObject::Ptr> generic_factory_objects;
  for (const auto &datatype_name : factory_.getDatatypeNames()) {
    auto newDatatype = factory_.instantiateDataType(datatype_name);
    generic_factory_objects.emplace(newDatatype);
  }
  network_ = std::make_shared<dataflow::TraactNetwork>(generic_factory_objects);

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
  return factory_.instantiatePattern(pattern_name);
}
