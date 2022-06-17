/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWUTILS_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWUTILS_H_

#include <vector>

namespace traact::dataflow {
    static constexpr size_t kStartEntries = 10;
    static constexpr std::chrono::milliseconds kFreeTaskFlowTimeout = std::chrono::seconds(60);
    template<typename T, size_t N>
    using SmallVector = std::vector<T>;

    struct ModuleTask {
        std::string module_key;
        std::vector<std::string> begin_components;
        std::vector<std::string> end_components;
    };

}

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWUTILS_H_
