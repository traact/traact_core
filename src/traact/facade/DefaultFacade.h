/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACTMULTI_DEFAULTFACADE_H
#define TRAACTMULTI_DEFAULTFACADE_H

#include <traact/facade/Facade.h>

namespace traact::facade {

class TRAACT_CORE_EXPORT DefaultFacade : public Facade {
 public:
    DefaultFacade(std::string plugin_dirs);

    DefaultFacade();

};

}

#endif //TRAACTMULTI_DEFAULTFACADE_H
