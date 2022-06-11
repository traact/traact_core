/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_UTIL_TRAACT_PLUGINS_RTTR_H_
#define TRAACT_CORE_SRC_TRAACT_UTIL_TRAACT_PLUGINS_RTTR_H_

#include <rttr/type>
#include <rttr/registration>

#define TRAACT_PLUGIN_ENABLE(...) RTTR_ENABLE(__VA_ARGS__)

#define BEGIN_TRAACT_PLUGIN_REGISTRATION RTTR_PLUGIN_REGISTRATION {

#define REGISTER_TYPE(type) rttr::registration::class_< type >(#type).constructor<>();

#define REGISTER_COMPONENT(type) rttr::registration::class_< type >(#type).constructor<>();

#define END_TRAACT_PLUGIN_REGISTRATION }

#endif//TRAACT_CORE_SRC_TRAACT_UTIL_TRAACT_PLUGINS_RTTR_H_
