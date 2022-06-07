/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACT_INCLUDE_TRAACT_TRAACT_H_
#define TRAACT_INCLUDE_TRAACT_TRAACT_H_
#include <traact/traact_core_export.h>
#include <traact/util/EnumToString.h>
#include <traact/util/Logging.h>
#include <traact/datatypes.h>
#include <traact/component/Component.h>
#include <traact/component/ModuleComponent.h>
#include <traact/pattern/Pattern.h>
#include <traact/pattern/ParameterUtils.h>
#include <traact/pattern/instance/GraphInstance.h>
#include <traact/dataflow/Network.h>
//#include <traact/buffer/TimeDomainManager.h>
#include <traact/buffer/ComponentBuffer.h>
#include <traact/buffer/DataFactory.h>
//#include <traact/buffer/TimeDomainBuffer.h>
#include <traact/util/TraactCoreUtils.h>
#include <traact/buffer/SourceComponentBuffer.h>
#include <traact/serialization/JsonGraphInstance.h>
#include <nlohmann/json.hpp>
#include <traact/facade/DefaultFacade.h>
#include <rttr/type>
#include <rttr/registration>

namespace traact {

template<typename T>
using DefaultDataFactory = buffer::TemplatedDefaultFactoryObject<T>;

using DefaultComponentGraphPtr = component::ComponentGraph::Ptr;

using DefaultFacade = traact::facade::DefaultFacade;

//typedef typename buffer::TimeDomainManager DefaultTimeDomainManager;
//typedef typename DefaultTimeDomainManager::ComponentGraph DefaultComponentGraph;
//typedef typename DefaultTimeDomainManager::ComponentGraphPtr DefaultComponentGraphPtr;
typedef typename buffer::ComponentBuffer DefaultComponentBuffer;

typedef typename component::Component DefaultComponent;
typedef typename DefaultComponent::Ptr DefaultComponentPtr;

typedef typename pattern::Pattern DefaultPattern;
typedef typename DefaultPattern::Ptr DefaultPatternPtr;
typedef typename pattern::instance::GraphInstance DefaultInstanceGraph;
typedef typename pattern::instance::GraphInstance::Ptr DefaultInstanceGraphPtr;
typedef typename pattern::instance::PatternInstance DefaultPatternInstance;
typedef typename pattern::instance::PatternInstance::Ptr DefaultPatternInstancePtr;

typedef typename dataflow::Network DefaultTraactNetwork;
}

#endif //TRAACT_INCLUDE_TRAACT_TRAACT_H_
