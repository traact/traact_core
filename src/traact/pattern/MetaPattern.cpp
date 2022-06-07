/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#include "traact/pattern/MetaPattern.h"
traact::pattern::MetaPattern::MetaPattern(const std::string &name, Concurrency concurrency) : Pattern(
    name,
    concurrency) {}
