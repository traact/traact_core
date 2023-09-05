/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

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

template<template <typename> typename Container, typename T>
bool containsName(const Container<T> &vecOfElements, const std::string &name) {
    auto it = std::find_if(std::begin(vecOfElements), std::begin(vecOfElements), [&name](const T &val) {
        if (val->getName() == name)
            return true;
        return false;
    });

    return it != vecOfElements.end();
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
 auto* vectorGetForName(const std::vector<T> &vecOfElements, const std::string &name) {
    auto it = std::find_if(vecOfElements.cbegin(), vecOfElements.cend(), [name](const T &val) {
        if (val.getName() == name)
            return true;
        return false;
    });


    if (it == vecOfElements.cend()) {
        //SPDLOG_TRACE("used vectorGetForName with unknown name {0}", name);
        return typename std::vector<T>::const_iterator::pointer(nullptr);
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
        //SPDLOG_TRACE("used vectorGetForName with unknown name {0}", name);
        return typename std::vector<T>::iterator::pointer(nullptr);
    }

    return &(*it);
}

}

#endif //TRAACT_INCLUDE_TRAACT_UTIL_UTILS_H_
