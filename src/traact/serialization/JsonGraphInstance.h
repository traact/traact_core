/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACT_INCLUDE_TRAACT_SERIALIZATION_JSONGRAPHINSTANCE_H_
#define TRAACT_INCLUDE_TRAACT_SERIALIZATION_JSONGRAPHINSTANCE_H_

#include <traact/serialization/JsonPatternInstance.h>
#include <traact/pattern/instance/GraphInstance.h>
#include <traact/traact_core_export.h>
#include "traact/serialization/JsonTimeDomainManagerConfig.h"
namespace ns {

using nlohmann::json;

void TRAACT_CORE_EXPORT to_json(json &jobj, const traact::pattern::instance::GraphInstance &obj);

void TRAACT_CORE_EXPORT from_json(const json &jobj, traact::pattern::instance::GraphInstance &obj);
} // namespace ns

#endif //TRAACT_INCLUDE_TRAACT_SERIALIZATION_JSONGRAPHINSTANCE_H_
