/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_INCLUDE_TRAACT_PATTERN_SPATIAL_COORDINATESYSTEM_H_
#define TRAACT_INCLUDE_TRAACT_PATTERN_SPATIAL_COORDINATESYSTEM_H_

#include <string>
#include <memory>
#include <traact/traact_core_export.h>

namespace traact::pattern::spatial {
struct TRAACT_CORE_EXPORT CoordinateSystem {
 public:
    CoordinateSystem();
    explicit CoordinateSystem(std::string name, bool is_multi);

    std::string name;
    bool is_multi;

};
}

#endif //TRAACT_INCLUDE_TRAACT_PATTERN_SPATIAL_COORDINATESYSTEM_H_
