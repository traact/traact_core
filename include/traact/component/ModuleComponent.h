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

#ifndef TRAACTMULTI_TRAACT_CORE_INCLUDE_TRAACT_COMPONENT_MODULECOMPONENT_H_
#define TRAACTMULTI_TRAACT_CORE_INCLUDE_TRAACT_COMPONENT_MODULECOMPONENT_H_

#include <traact/component/Component.h>
namespace traact::component {

class TRAACT_CORE_EXPORT ModuleComponent;

/**
 * Base class for all module implementation to be used by a module component
 */
class TRAACT_CORE_EXPORT Module {
 public:
  typedef typename std::shared_ptr<Module> Ptr;
  typedef ModuleComponent* ComponentPtr;

  Module() = default;
  virtual ~Module() = default;

  virtual bool init(ComponentPtr module_component);

  virtual bool start(ComponentPtr module_component);

  virtual bool stop(ComponentPtr module_component);

  virtual bool teardown(ComponentPtr module_component);

  RTTR_ENABLE();

};

/**
 * Base for all traact module components.
 * A module component is a component that shares a common "module" with a set of other module components.
 *
 */
class TRAACT_CORE_EXPORT ModuleComponent : public Component{
 public:
  typedef typename std::shared_ptr<ModuleComponent> Ptr;
  ModuleComponent(std::string name, const ComponentType traact_component_type, const ModuleType module_type);

  ModuleType GetModuleType() const;

  virtual std::string GetModuleKey() = 0;
  virtual Module::Ptr InstantiateModule() = 0;

  void SetModule(Module::Ptr module);

  bool configure(const nlohmann::json &parameter, buffer::GenericComponentBufferConfig *data) override;
  bool start() override;
  bool stop() override;
  bool teardown() override;

 protected:
  ModuleType module_type_;
  Module::Ptr module_;

 RTTR_ENABLE(Component)
};

}
#endif //TRAACTMULTI_TRAACT_CORE_INCLUDE_TRAACT_COMPONENT_MODULECOMPONENT_H_
