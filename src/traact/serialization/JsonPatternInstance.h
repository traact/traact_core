/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_INCLUDE_TRAACT_SERIALIZATION_JSONPATTERNINSTANCE_H_
#define TRAACT_INCLUDE_TRAACT_SERIALIZATION_JSONPATTERNINSTANCE_H_

#include <traact/serialization/JsonPattern.h>
#include <traact/serialization/JsonPortInstance.h>
#include <traact/pattern/instance/PatternInstance.h>
#include <traact/traact_core_export.h>
namespace ns {

using nlohmann::json;

void TRAACT_CORE_EXPORT to_json(json &jobj, const traact::pattern::instance::PatternInstance &obj);
void TRAACT_CORE_EXPORT from_json(const json &jobj, traact::pattern::instance::PatternInstance &obj);

} // namespace ns


#endif //TRAACT_INCLUDE_TRAACT_SERIALIZATION_JSONPATTERNINSTANCE_H_
