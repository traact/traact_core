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
#include <traact/util/Logging.h>
#include <traact/component/ModuleComponent.h>

namespace traact::component {



    bool Module::init(ComponentPtr module_component) {
      return true;
    }

    bool Module::start(ComponentPtr module_component) {
      return true;
    }

    bool Module::stop(ComponentPtr module_component) {
      return true;
    }

    bool Module::teardown(ComponentPtr module_component) {
      return true;
    }

  ModuleComponent::ModuleComponent(std::string name, const ComponentType traact_component_type, const ModuleType module_type)
: Component(std::move(name), traact_component_type), module_type_(module_type) {

}

ModuleType ModuleComponent::GetModuleType() const {
  return module_type_;
}

void ModuleComponent::SetModule(Module::Ptr module) {
  SPDLOG_DEBUG("setting module for module component");
  module_ = module;
}

bool ModuleComponent::configure(const nlohmann::json &parameter, buffer::ComponentBufferConfig *data) {
return module_->init(this);
}
bool ModuleComponent::start() {
return module_->start(this);
}
bool ModuleComponent::stop() {
return module_->stop(this);
}
bool ModuleComponent::teardown() {
return module_->teardown(this);
}


}