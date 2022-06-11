/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "traact/pattern/MetaPattern.h"
traact::pattern::MetaPattern::MetaPattern(const std::string &name, Concurrency concurrency) : Pattern(
    name,
    concurrency, component::ComponentType::ASYNC_SINK) {}
