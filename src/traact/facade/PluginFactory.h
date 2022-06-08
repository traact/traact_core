/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/
#ifndef TRAACT_INCLUDE_TRAACT_FACADE_PLUGINFACTORY_H_
#define TRAACT_INCLUDE_TRAACT_FACADE_PLUGINFACTORY_H_

#include <string>
#include <set>
#include <map>

#include <traact/pattern/Pattern.h>
#include <traact/buffer/DataFactory.h>
#include <traact/buffer/ComponentBuffer.h>
#include <traact/component/Component.h>
#include <traact/traact_core_export.h>

#include <traact/buffer/DataBufferFactory.h>

namespace traact::facade {

class TRAACT_CORE_EXPORT PluginFactory : public buffer::DataBufferFactory {
 public:
    typedef typename std::shared_ptr<PluginFactory> Ptr;
    typedef typename buffer::DataFactory::Ptr FactoryObjectPtr;
    typedef typename component::Component::Ptr ComponentPtr;
    typedef typename pattern::Pattern::Ptr PatternPtr;

    PluginFactory() = default;
    virtual ~PluginFactory() = default;

    virtual std::vector<std::string> getDatatypeNames() = 0;
    virtual std::vector<std::string> getPatternNames() = 0;

    virtual PatternPtr instantiatePattern(const std::string &pattern_name) = 0;
    virtual ComponentPtr instantiateComponent(const std::string &pattern_name,
                                              const std::string &new_component_name) = 0;

};

}

#endif //TRAACT_INCLUDE_TRAACT_FACADE_PLUGINFACTORY_H_
