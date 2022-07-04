/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_COMPONENT_MODULECOMPONENT_H_
#define TRAACT_CORE_SRC_TRAACT_COMPONENT_MODULECOMPONENT_H_

#include "traact/component/Component.h"
#include "traact/traact_plugins.h"
#include <spdlog/spdlog.h>

namespace traact::component {

class TRAACT_CORE_EXPORT ModuleComponent;

/**
 * Base class for all module implementation to be used by a module component
 */
class TRAACT_CORE_EXPORT Module {
 public:
    typedef typename std::shared_ptr<Module> Ptr;
    typedef ModuleComponent *ComponentPtr;

    Module() = default;
    virtual ~Module() = default;

    virtual bool init(ComponentPtr module_component);

    virtual bool start(ComponentPtr module_component);

    virtual bool stop(ComponentPtr module_component);

    virtual bool teardown(ComponentPtr module_component);

    virtual void processTimePoint();

};

/**
 * Base for all traact module components.
 * A module component is a component that shares a common "module" with a set of other module components.
 *
 */
class TRAACT_CORE_EXPORT ModuleComponent : public Component {
 public:
    typedef typename std::shared_ptr<ModuleComponent> Ptr;
    ModuleComponent(std::string name, ModuleType module_type);
    virtual ~ModuleComponent() = default;

    ModuleType getModuleType() const;

    virtual std::string getModuleKey() = 0;
    virtual Module::Ptr instantiateModule() = 0;

    virtual void setModule(Module::Ptr module);

    bool configure(const pattern::instance::PatternInstance &pattern_instance, buffer::ComponentBufferConfig *data) override;
    bool start() override;
    bool stop() override;
    bool teardown() override;

 protected:
    ModuleType module_type_;
    Module::Ptr module_;


};

}
#endif //TRAACT_CORE_SRC_TRAACT_COMPONENT_MODULECOMPONENT_H_
