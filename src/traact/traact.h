/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_TRAACT_H_
#define TRAACT_CORE_SRC_TRAACT_TRAACT_H_
#include <nlohmann/json.hpp>
#include <traact/buffer/ComponentBuffer.h>
#include <traact/buffer/DataFactory.h>
#include <traact/buffer/SourceComponentBuffer.h>
#include <traact/component/Component.h>
#include <traact/component/ModuleComponent.h>
#include <traact/component/ComponentFactory.h>
#include <traact/dataflow/Network.h>
#include <traact/datatypes.h>
#include <traact/facade/DefaultFacade.h>
#include <traact/pattern/ParameterUtils.h>
#include <traact/pattern/Pattern.h>
#include <traact/pattern/instance/GraphInstance.h>
#include <traact/serialization/JsonGraphInstance.h>
#include <traact/traact_core_export.h>
#include <traact/util/EnumToString.h>
#include <traact/util/Logging.h>
#include <traact/util/FileUtil.h>
#include "traact/component/generic_components.h"
#include "traact_plugins.h"

namespace traact {

using DefaultFacade = traact::facade::DefaultFacade;

typedef typename pattern::instance::GraphInstance DefaultInstanceGraph;
typedef typename pattern::instance::GraphInstance::Ptr DefaultInstanceGraphPtr;
typedef typename pattern::instance::PatternInstance::Ptr DefaultPatternInstancePtr;

};

#define CREATE_TRAACT_HEADER_TYPE(header_name, type, meta, export_tag)\
struct export_tag header_name { \
    using NativeType = type; \
    static constexpr const char *NativeTypeName{#type}; \
    static constexpr const char *MetaType{meta}; \
    static constexpr const size_t size = sizeof(type); \
    header_name() = default;                  \
    header_name(header_name const& value) = default; \
    header_name& operator=(header_name const& value) = default; \
    header_name(header_name && value) = default; \
    header_name& operator=(header_name && value) = default; \
}; \
class header_name##Factory : public traact::buffer::TemplatedDefaultDataFactory<header_name> { \
    TRAACT_PLUGIN_ENABLE(traact::buffer::DataFactory, traact::buffer::TemplatedDefaultDataFactory<header_name>) \
};

#define CREATE_TRAACT_COMPONENT_FACTORY(component_name) \
class component_name##Factory : public traact::component::TemplatedDefaultComponentFactory<component_name> { \
    TRAACT_PLUGIN_ENABLE(traact::component::ComponentFactory, traact::component::TemplatedDefaultComponentFactory<component_name>) \
};

#define CREATE_TEMPLATED_TRAACT_COMPONENT_FACTORY(component_name, type_namespace, type_name) \
class component_name##type_name##Factory : public traact::component::TemplatedDefaultComponentFactory<component_name<type_namespace::type_name> > { \
    TRAACT_PLUGIN_ENABLE(traact::component::ComponentFactory, traact::component::TemplatedDefaultComponentFactory<component_name<type_namespace::type_name> >) \
};

#define REGISTER_DEFAULT_COMPONENT(component_name) \
REGISTER_COMPONENT(component_name##Factory)

#define REGISTER_TEMPLATED_DEFAULT_COMPONENT(component_name, type_name) \
REGISTER_COMPONENT(component_name##type_name##Factory)

#define REGISTER_DEFAULT_TRAACT_TYPE(type) \
REGISTER_TYPE(type##Factory)



#endif//TRAACT_CORE_SRC_TRAACT_TRAACT_H_
