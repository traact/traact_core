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

#include <iostream>
#include <spdlog/spdlog.h>
#include "PluginFactory.h"

bool traact::facade::PluginFactory::addLibrary(const std::string &filename) {
  PluginPtr newPlugin;

  spdlog::info("loading library file: {0}", filename);

  try {
    newPlugin = std::make_shared<Plugin>(filename);
    if (!newPlugin->init()) {
      spdlog::error("init failed for: {0}", filename);
      return false;
    }

  } catch (...) {
    spdlog::error("init thows exception for: {0}", filename);
    return false;
  }

  //TODO gather all names in a set, check of set is smaller, if so then there where dublicate entries, error

  for (const auto &datatype_name : newPlugin->datatype_names) {
    registered_datatypes_.emplace(std::make_pair(datatype_name, newPlugin));
  }
  for (const auto &pattern_name : newPlugin->pattern_names) {
    registered_patterns_.emplace(std::make_pair(std::string(pattern_name), newPlugin));
  }

  return true;

}
bool traact::facade::PluginFactory::removeLibrary(const std::string &filename) {
  return false;
}
std::vector<std::string> traact::facade::PluginFactory::getDatatypeNames() {
  std::vector<std::string> result;
  std::transform(registered_datatypes_.begin(), registered_datatypes_.end(),
                 std::inserter(result, result.end()),
                 [](auto pair) { return pair.first; });
  return result;
}
std::vector<std::string> traact::facade::PluginFactory::getPatternNames() {
  std::vector<std::string> result;
  /*for (const auto &item : registered_patterns_) {
    result.emplace_back(item.first);

  }*/
  std::transform(registered_patterns_.begin(), registered_patterns_.end(),
                 std::inserter(result, result.end()),
                 [](auto pair) { return pair.first; });
  return result;
}
traact::facade::PluginFactory::FactoryObjectPtr traact::facade::PluginFactory::instantiateDataType(const std::string &datatype_name) {
  return registered_datatypes_.at(datatype_name)->instantiateDataType(datatype_name);
}
traact::facade::PluginFactory::PatternPtr traact::facade::PluginFactory::instantiatePattern(const std::string &pattern_name) {
  return registered_patterns_.at(pattern_name)->instantiatePattern(pattern_name);
}
traact::facade::PluginFactory::ComponentPtr traact::facade::PluginFactory::instantiateComponent(const std::string &pattern_name,
                                                                                                const std::string &new_component_name) {
  return registered_patterns_.at(pattern_name)->instantiateComponent(pattern_name, new_component_name);
}
traact::facade::PluginFactory::Plugin::Plugin(const std::string &library_file_name) : library_(library_file_name) {

}
traact::facade::PluginFactory::Plugin::~Plugin() {
  teardown();
}
bool traact::facade::PluginFactory::Plugin::init() {
  using namespace rttr;
  library_.load();

  for (const auto &t : library_.get_types()) // returns all registered types from this library
  {
    if (!t.is_class() || t.is_wrapper())
      continue;

    spdlog::info("loading plugin {0}", std::string(t.get_name().begin(), t.get_name().end()));

    constructor ctor = t.get_constructor();
    if (!ctor.is_valid()) {
      spdlog::error("could not load default constructor () for {0}",
                    std::string(t.get_name().begin(), t.get_name().end()));
      return false;
    }

    variant var = ctor.invoke();
    if (!var.is_valid()) {
      spdlog::error("invoking constructor() failed for {0}", std::string(t.get_name().begin(), t.get_name().end()));
      return false;
    }

    traact_plugin = var.get_value<traact::facade::TraactPlugin::Ptr>();
    traact_plugin->fillPatternNames(pattern_names);
    traact_plugin->fillDatatypeNames(datatype_names);
  }

  return true;

}
bool traact::facade::PluginFactory::Plugin::teardown() {
  traact_plugin.reset();
  library_.unload();
}
traact::facade::PluginFactory::FactoryObjectPtr traact::facade::PluginFactory::Plugin::instantiateDataType(const std::string &datatype_name) {
  return traact_plugin->instantiateDataType(datatype_name);
}
traact::facade::PluginFactory::PatternPtr traact::facade::PluginFactory::Plugin::instantiatePattern(const std::string &patternName) {
  return traact_plugin->instantiatePattern(patternName);
}
traact::facade::PluginFactory::ComponentPtr traact::facade::PluginFactory::Plugin::instantiateComponent(const std::string &patternName,
                                                                                                        const std::string &new_component_name) {
  return traact_plugin->instantiateComponent(patternName, new_component_name);
}
