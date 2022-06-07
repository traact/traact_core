/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACTMULTI_JSONTIMEDOMAINMANAGERCONFIG_H
#define TRAACTMULTI_JSONTIMEDOMAINMANAGERCONFIG_H

#include <nlohmann/json.hpp>
#include <traact/datatypes.h>
#include <traact/traact_core_export.h>
namespace ns {

using nlohmann::json;

void TRAACT_CORE_EXPORT to_json(json &jobj, const traact::buffer::TimeDomainManagerConfig &obj);

void TRAACT_CORE_EXPORT from_json(const json &jobj, traact::buffer::TimeDomainManagerConfig &obj);

} // namespace ns


#endif //TRAACTMULTI_JSONTIMEDOMAINMANAGERCONFIG_H
