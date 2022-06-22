/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_SERIALIZATION_JSONTIMEDOMAINMANAGERCONFIG_H_
#define TRAACT_CORE_SRC_TRAACT_SERIALIZATION_JSONTIMEDOMAINMANAGERCONFIG_H_

#include <nlohmann/json.hpp>
#include <traact/datatypes.h>
#include <traact/traact_core_export.h>
namespace ns {

using nlohmann::json;

void TRAACT_CORE_EXPORT to_json(json &jobj, const traact::buffer::TimeDomainManagerConfig &obj);

void TRAACT_CORE_EXPORT from_json(const json &jobj, traact::buffer::TimeDomainManagerConfig &obj);

} // namespace ns


#endif //TRAACT_CORE_SRC_TRAACT_SERIALIZATION_JSONTIMEDOMAINMANAGERCONFIG_H_
