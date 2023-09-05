/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACTMULTI_TRAACT_FACADE_INCLUDE_TRAACT_UTIL_FILEUTIL_H_
#define TRAACTMULTI_TRAACT_FACADE_INCLUDE_TRAACT_UTIL_FILEUTIL_H_

#include <string>
#include <set>
#include <vector>
#include "traact/traact_core_export.h"

namespace traact::util {

bool TRAACT_CORE_EXPORT fileExists(const std::string &filename, const std::string &component_name);

bool TRAACT_CORE_EXPORT fileExists(const std::string &filename);

bool TRAACT_CORE_EXPORT createFileDirectory(const std::string &filename);

bool TRAACT_CORE_EXPORT hasEnding(std::string const &full_string, std::string const &ending);

std::vector<std::string> TRAACT_CORE_EXPORT globFiles(const std::string &path);

std::vector<std::string>TRAACT_CORE_EXPORT globDirs(const std::string &path);

std::vector<std::string> TRAACT_CORE_EXPORT globFiles(const std::string &path, const std::string &file_ending);

}

#endif //TRAACTMULTI_TRAACT_FACADE_INCLUDE_TRAACT_UTIL_FILEUTIL_H_
