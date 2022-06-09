/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_COMPONENT_GENERIC_COMPONENTS_H_
#define TRAACT_CORE_SRC_TRAACT_COMPONENT_GENERIC_COMPONENTS_H_

#include <traact/component/facade/ApplicationAsyncSource.h>
#include <traact/component/facade/ApplicationSyncSink.h>

#define REGISTER_GENERIC_COMPONENT_APPLICATION(type)\
REGISTER_COMPONENT(traact::component::facade::ApplicationAsyncSource<type>)\
REGISTER_COMPONENT(traact::component::facade::ApplicationSyncSink<type>)

#define REGISTER_GENERIC_BASE_COMPONENTS(type)\
REGISTER_GENERIC_COMPONENT_APPLICATION(type)\

#endif//TRAACT_CORE_SRC_TRAACT_COMPONENT_GENERIC_COMPONENTS_H_
