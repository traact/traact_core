/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACT_INCLUDE_TRAACT_UTIL_UTILS_H_
#define TRAACT_INCLUDE_TRAACT_UTIL_UTILS_H_

#include <set>
#include <map>

#include "Logging.h"
#include "traact/traact_core_export.h"
#include "traact/datatypes.h"
namespace traact::util {
template<typename K, typename T>
static std::set<K> getKeys(const std::map<K, T> &values) {
    std::set<K> result;
    for (const auto &item : values) {
        result.emplace(item.first);
    }
    return std::move(result);
}
template<typename K, typename T>
static std::set<T> getValues(const std::map<K, T> &values) {
    std::set<T> result;
    for (const auto &item : values) {
        result.emplace(item.second);
    }
    return std::move(result);
}

template<typename T>
bool vectorContainsName(const std::vector<T> &vecOfElements, const std::string &name) {
    auto it = std::find_if(vecOfElements.begin(), vecOfElements.end(), [&name](const T &val) {
        if (val.getName() == name)
            return true;
        return false;
    });

    return it != vecOfElements.end();
}

template<typename T>
const T *vectorGetForName(const std::vector<T> &vecOfElements, const std::string &name) {
    auto it = std::find_if(vecOfElements.begin(), vecOfElements.end(), [name](const T &val) {
        if (val.getName() == name)
            return true;
        return false;
    });

    if (it == vecOfElements.end()) {
        SPDLOG_ERROR("used vectorGetForName with unkown name");
    }

    return &(*it);
}

template<typename T>
T *vectorGetForName(std::vector<T> &vecOfElements, const std::string &name) {
    auto it = std::find_if(vecOfElements.begin(), vecOfElements.end(), [name](const T &val) {
        if (val.getName() == name)
            return true;
        return false;
    });

    if (it == vecOfElements.end()) {
        SPDLOG_ERROR("used vectorGetForName with unknown name");
        return nullptr;
    }

    return &(*it);
}

}

#endif //TRAACT_INCLUDE_TRAACT_UTIL_UTILS_H_
