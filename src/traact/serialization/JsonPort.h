/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/
#ifndef TRAACT_INCLUDE_TRAACT_SERIALIZATION_JSONPORT_H_
#define TRAACT_INCLUDE_TRAACT_SERIALIZATION_JSONPORT_H_

#include <nlohmann/json.hpp>
#include <traact/pattern/Port.h>
#include <traact/traact_core_export.h>
namespace ns {

using nlohmann::json;

void TRAACT_CORE_EXPORT to_json(json &jobj, const traact::pattern::Port &obj);

void TRAACT_CORE_EXPORT from_json(const json &jobj, traact::pattern::Port &obj);

} // namespace ns

#endif //TRAACT_INCLUDE_TRAACT_SERIALIZATION_JSONPORT_H_
