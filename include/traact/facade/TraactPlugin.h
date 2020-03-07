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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#ifndef TRAACT_INCLUDE_TRAACT_FACADE_TRAACTPLUGIN_H_
#define TRAACT_INCLUDE_TRAACT_FACADE_TRAACTPLUGIN_H_

#include <traact/pattern/Pattern.h>
#include <traact/buffer/GenericFactoryObject.h>
#include <traact/component/Component.h>

namespace traact::facade {
class TraactPlugin {
 public:
  typedef typename std::shared_ptr<TraactPlugin> Ptr;
  TraactPlugin() = default;;
  virtual ~TraactPlugin() = default;;

  virtual bool init() {
    return true;
  }
  virtual bool teardown() {
    return true;
  }
  virtual void fillDatatypeNames(std::vector<std::string> &datatype_names) {

  }
  virtual void fillPatternNames(std::vector<std::string> &pattern_names) {
  }

  virtual buffer::GenericFactoryObject::Ptr instantiateDataType(const std::string &datatype_name) {
    return nullptr;
  }
  virtual pattern::Pattern::Ptr instantiatePattern(const std::string &pattern_name) {
    return nullptr;
  }
  virtual component::Component::Ptr instantiateComponent(const std::string &pattern_name,
                                                         const std::string &new_component_name) {
    return nullptr;
  }
};

}

#endif //TRAACT_INCLUDE_TRAACT_FACADE_TRAACTPLUGIN_H_

#pragma clang diagnostic pop