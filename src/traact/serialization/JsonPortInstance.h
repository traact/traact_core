/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACT_INCLUDE_TRAACT_SERIALIZATION_JSONPORTINSTANCE_H_
#define TRAACT_INCLUDE_TRAACT_SERIALIZATION_JSONPORTINSTANCE_H_

#include <traact/serialization/JsonPort.h>
#include <traact/pattern/instance/PortInstance.h>
#include <traact/traact_core_export.h>
namespace ns {

using nlohmann::json;

void TRAACT_CORE_EXPORT to_json(json &jobj, const traact::pattern::instance::PortInstance &obj);

void TRAACT_CORE_EXPORT from_json(const json &jobj, traact::pattern::instance::PortInstance &obj);

} // namespace ns

#endif //TRAACT_INCLUDE_TRAACT_SERIALIZATION_JSONPORTINSTANCE_H_
