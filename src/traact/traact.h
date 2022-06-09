/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_TRAACT_H_
#define TRAACT_CORE_SRC_TRAACT_TRAACT_H_
#include <nlohmann/json.hpp>
#include <traact/buffer/ComponentBuffer.h>
#include <traact/buffer/DataFactory.h>
#include <traact/buffer/SourceComponentBuffer.h>
#include <traact/component/Component.h>
#include <traact/component/ModuleComponent.h>
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

template<typename T>
using DefaultDataFactory = buffer::TemplatedDefaultFactoryObject<T>;

using DefaultComponentGraphPtr = component::ComponentGraph::Ptr;

using DefaultFacade = traact::facade::DefaultFacade;
typedef typename buffer::ComponentBuffer DefaultComponentBuffer;

typedef typename component::Component DefaultComponent;
typedef typename DefaultComponent::Ptr DefaultComponentPtr;

typedef typename pattern::instance::GraphInstance DefaultInstanceGraph;
typedef typename pattern::instance::GraphInstance::Ptr DefaultInstanceGraphPtr;
typedef typename pattern::instance::PatternInstance::Ptr DefaultPatternInstancePtr;

}

#define REGISTER_DEFAULT_TRAACT_TYPE(type)\
REGISTER_TYPE(type##FactoryObject)\
REGISTER_GENERIC_BASE_COMPONENTS(type)

#endif//TRAACT_CORE_SRC_TRAACT_TRAACT_H_
