/*  BSD 3-Clause License
 *
 *  Copyright (c) 2020, FriederPankratz <frieder.pankratz@gmail.com>
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/

#include "FileUtil.h"
#include <spdlog/spdlog.h>

namespace traact::util {

    bool FileExists(const std::string& filename, const std::string& component_name) {
        cppfs::FileHandle fh = cppfs::fs::open(filename);
        if(fh.exists() && fh.isFile()) {
            SPDLOG_INFO("{0}: file {1} ready", component_name, filename);
            return true;
        } else {
            SPDLOG_ERROR("{0}: file {1} does not exist", component_name, filename);
            return false;
        }
    }

    bool hasEnding(std::string const &fullString, std::string const &ending) {
        if (fullString.length() >= ending.length()) {
            return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
        }
        else {
            return false;
        }
    }

    std::vector<std::string> glob_files(const std::string &path) {
        std::vector<std::string> result;
        if (!path.empty()) {
            namespace fs = boost::filesystem;

            fs::path bpath(path);
            fs::recursive_directory_iterator end;

            for (fs::recursive_directory_iterator i(bpath); i != end; ++i) {
                const fs::path cp = (*i);

                result.emplace_back(cp.string());
            }
        }
        return std::move(result);
    }

    std::vector<std::string> glob_dirs(const std::string &path) {
        std::vector<std::string> result;
        if (!path.empty()) {
            namespace fs = boost::filesystem;

            fs::path bpath(path);
            fs::recursive_directory_iterator end;

            for (fs::recursive_directory_iterator i(bpath); i != end; ++i) {
                const fs::path cp = (*i);
                if(is_directory(cp))
                    result.emplace_back(cp.string());
            }
        }
        return std::move(result);
    }

    std::vector<std::string> glob_files(const std::string &path, const std::string &file_ending) {
        std::vector<std::string> result;

        for (std::string path : glob_files(path)) {
            if (hasEnding(path, file_ending))
                result.emplace_back(path);
        }



        return std::move(result);
    }

}
