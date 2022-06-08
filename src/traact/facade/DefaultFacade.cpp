/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "DefaultFacade.h"

#include "traact/facade/rttr/RTTRPluginFactory.h"
//#include <traact/dataflow/tbb/TBBNetwork.h>
#include "traact/dataflow/taskflow/TaskFlowNetwork.h"

traact::facade::DefaultFacade::DefaultFacade(std::string plugin_dirs) : Facade(
    std::make_shared<RTTRPluginFactory>(plugin_dirs), std::make_shared<dataflow::TaskFlowNetwork>()) {

}

traact::facade::DefaultFacade::DefaultFacade() : Facade(
    std::make_shared<RTTRPluginFactory>(), std::make_shared<dataflow::TaskFlowNetwork>()) {}
