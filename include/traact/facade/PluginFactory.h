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
#ifndef TRAACT_INCLUDE_TRAACT_FACADE_PLUGINFACTORY_H_
#define TRAACT_INCLUDE_TRAACT_FACADE_PLUGINFACTORY_H_

#include <string>
#include <set>
#include <map>
#include <rttr/type>

#include <traact/pattern/Pattern.h>
#include <traact/buffer/GenericFactoryObject.h>
#include <traact/buffer/GenericComponentBuffer.h>
#include <traact/facade/TraactPlugin.h>
namespace traact::facade {

class PluginFactory {
 public:
  typedef typename buffer::GenericFactoryObject::Ptr FactoryObjectPtr;
  typedef typename component::Component::Ptr ComponentPtr;
  typedef typename pattern::Pattern::Ptr PatternPtr;

  bool addLibrary(const std::string &filename);
  bool removeLibrary(const std::string &filename);

  std::vector<std::string> getDatatypeNames();
  std::vector<std::string> getPatternNames();

  FactoryObjectPtr instantiateDataType(const std::string &datatype_name);
  PatternPtr instantiatePattern(const std::string &pattern_name);
  ComponentPtr instantiateComponent(const std::string &pattern_name, const std::string &new_component_name);
 private:
  struct Plugin {
    explicit Plugin(const std::string &library_file_name);
    ~Plugin();

    rttr::library library_;

    traact::facade::TraactPlugin::Ptr traact_plugin;
    std::vector<std::string> pattern_names;
    std::vector<std::string> datatype_names;

    bool init();
    bool teardown();
    FactoryObjectPtr instantiateDataType(const std::string &patternName);
    PatternPtr instantiatePattern(const std::string &patternName);
    ComponentPtr instantiateComponent(const std::string &patternName, const std::string &new_component_name);
  };

  typedef typename std::shared_ptr<Plugin> PluginPtr;

  std::map<std::string, PluginPtr> registered_patterns_;
  std::map<std::string, PluginPtr> registered_datatypes_;

};

}

#endif //TRAACT_INCLUDE_TRAACT_FACADE_PLUGINFACTORY_H_
