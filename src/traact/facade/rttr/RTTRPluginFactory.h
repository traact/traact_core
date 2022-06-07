/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACTMULTI_RTTRPLUGINFACTORY_H
#define TRAACTMULTI_RTTRPLUGINFACTORY_H

#include <string>
#include <set>
#include <map>
#include <rttr/type>
#include "traact/facade/PluginFactory.h"
#include "traact/traact_core_export.h"

namespace traact::facade {

class TRAACT_CORE_EXPORT RTTRPluginFactory : public PluginFactory {
 public:
    typedef typename buffer::DataFactory::Ptr FactoryObjectPtr;
    typedef typename component::Component::Ptr ComponentPtr;
    typedef typename pattern::Pattern::Ptr PatternPtr;

    RTTRPluginFactory();

    explicit RTTRPluginFactory(const std::string &pluginDirectories);

    ~RTTRPluginFactory() = default;

    std::vector<std::string> getDatatypeNames() override;
    std::vector<std::string> getPatternNames() override;

    buffer::DataBufferPtr createBuffer() override;

    PatternPtr instantiatePattern(const std::string &pattern_name) override;
    ComponentPtr instantiateComponent(const std::string &pattern_name, const std::string &new_component_name) override;
 private:
    struct Plugin {
        explicit Plugin(const std::string &library_file_name);
        ~Plugin();

        rttr::library library_;

        bool init();
        bool teardown();
    };

    bool addLibrary(const std::string &filename);
    bool init();
    bool removeLibrary(const std::string &filename);

    void addDatatype(const rttr::type &t);
    void addPattern(const rttr::type &t);

    typedef typename std::shared_ptr<Plugin> PluginPtr;

    std::string plugin_directories_;

    std::vector<std::string> pattern_names;
    std::vector<std::string> datatype_names;

    std::set<traact::buffer::DataFactory::Ptr> datatype_traact_plugin;
    std::map<std::string, rttr::constructor> component_to_traact_plugin;
    std::map<std::string, component::Component::Ptr> pattern_to_traact_plugin;

};

}

#endif //TRAACTMULTI_RTTRPLUGINFACTORY_H
