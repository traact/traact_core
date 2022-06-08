/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "FileUtil.h"
#include <spdlog/spdlog.h>

#include <cppfs/fs.h>
#include <cppfs/FileHandle.h>
#include <cppfs/FileIterator.h>
namespace traact::util {

bool fileExists(const std::string &filename, const std::string &component_name) {
    cppfs::FileHandle fh = cppfs::fs::open(filename);
    if (fh.exists() && fh.isFile()) {
        SPDLOG_INFO("{0}: file {1} ready", component_name, filename);
        return true;
    } else {
        SPDLOG_ERROR("{0}: file {1} does not exist", component_name, filename);
        return false;
    }
}

bool hasEnding(std::string const &full_string, std::string const &ending) {
    if (full_string.length() >= ending.length()) {
        return (0 == full_string.compare(full_string.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

std::vector<std::string> globFiles(const std::string &path) {
    using namespace cppfs;
    std::vector<std::string> result;
    if (!path.empty()) {
        FileHandle dir = fs::open(path);
        if (dir.isDirectory()) {

            for (FileIterator it = dir.begin(); it != dir.end(); ++it) {
                result.push_back(*it);
            }
        }
    }
    return std::move(result);
}

std::vector<std::string> globDirs(const std::string &path) {
    using namespace cppfs;
    std::vector<std::string> result;
    if (!path.empty()) {
        FileHandle dir = fs::open(path);
        if (dir.isDirectory()) {

            for (FileIterator it = dir.begin(); it != dir.end(); ++it) {
                FileHandle sub_dir = fs::open(*it);
                if (sub_dir.isDirectory())
                    result.push_back(*it);
            }
        }
    }
    return result;
}

std::vector<std::string> globFiles(const std::string &path, const std::string &file_ending) {
    std::vector<std::string> result;

    for (std::string t_path : globFiles(path)) {
        if (hasEnding(t_path, file_ending))
            result.emplace_back(t_path);
    }

    return result;
}

}
