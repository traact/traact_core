/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#include "traact/pattern/CoordinateSystem.h"
traact::pattern::spatial::CoordinateSystem::CoordinateSystem(std::string name, bool is_multi)
    : name(std::move(name)), is_multi(is_multi) {}
traact::pattern::spatial::CoordinateSystem::CoordinateSystem() : name("Invalid"), is_multi(false) {

}
