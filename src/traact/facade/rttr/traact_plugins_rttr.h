/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_UTIL_TRAACT_PLUGINS_RTTR_H_
#define TRAACT_CORE_SRC_TRAACT_UTIL_TRAACT_PLUGINS_RTTR_H_

#include <rttr/type>
#include <rttr/registration>

#define TRAACT_PLUGIN_ENABLE(...) RTTR_ENABLE(__VA_ARGS__)

#define CREATE_TRAACT_HEADER_TYPE(header_name, type, meta, export_tag)\
struct export_tag header_name {\
    using NativeType = type; \
    static constexpr const char *NativeTypeName{#type}; \
    static constexpr const char *MetaType{meta}; \
    const size_t size = sizeof(type);\
}; \
class export_tag header_name##FactoryObject : public buffer::TemplatedDefaultFactoryObject<header_name> {\
  RTTR_ENABLE(buffer::TemplatedDefaultFactoryObject<header_name>, buffer::DataFactory)\
};


#define BEGIN_TRAACT_PLUGIN_REGISTRATION RTTR_PLUGIN_REGISTRATION {
#define REGISTER_TYPE(type) rttr::registration::class_<type>(#type).constructor<>()();
#define REGISTER_COMPONENT(type ) rttr::registration::class_< type >(#type).constructor<std::string>()();
#define END_TRAACT_PLUGIN_REGISTRATION }

#endif//TRAACT_CORE_SRC_TRAACT_UTIL_TRAACT_PLUGINS_RTTR_H_
