/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_COMPONENT_COMPONENTFACTORY_H_
#define TRAACT_CORE_SRC_TRAACT_COMPONENT_COMPONENTFACTORY_H_

#include "traact/traact_core_export.h"
#include "traact/traact_plugins.h"
#include "Component.h"

namespace traact::component {

class TRAACT_CORE_EXPORT ComponentFactory : public std::enable_shared_from_this<ComponentFactory> {
 public:
    typedef typename std::shared_ptr<ComponentFactory> Ptr;
    ComponentFactory() = default;
    virtual ~ComponentFactory() = default;

    virtual pattern::Pattern::Ptr createPattern() = 0;
    virtual Component::Ptr createComponent(std::string instance_id) = 0;

    template<typename Derived>
    std::shared_ptr<Derived> shared_from_base() {
        return std::static_pointer_cast<Derived>(shared_from_this());
    }
 TRAACT_PLUGIN_ENABLE()
};

template<class T>
class TRAACT_CORE_EXPORT TemplatedDefaultComponentFactory : public ComponentFactory {
 public:
    pattern::Pattern::Ptr createPattern() override {
        return T::GetPattern();
    };
    Component::Ptr createComponent(std::string instance_id) override {
        return std::make_shared<T>(instance_id);
    };
 TRAACT_PLUGIN_ENABLE(ComponentFactory)
};

}

#endif //TRAACT_CORE_SRC_TRAACT_BUFFER_DATAFACTORY_H_

