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

#ifndef TRAACTMULTI_FILEREADER_H
#define TRAACTMULTI_FILEREADER_H


#include <rttr/registration>
#include <traact/traact.h>
#include <fmt/format.h>
#include <traact/util/FileUtil.h>

namespace traact::component {

    template<class T>
    class FileReader : public Component {
    public:
        explicit FileReader(const std::string &name, const std::string& serializer_name) : Component(name,
                                                                                                     traact::component::ComponentType::SyncSource),
                                                                                           serializer_name_(serializer_name){
        }



        traact::pattern::Pattern::Ptr GetPattern()  const {


            std::string pattern_name = fmt::format("FileReader_{0}_{1}", serializer_name_, T::MetaType);

            traact::pattern::spatial::SpatialPattern::Ptr
                    pattern =
                    std::make_shared<traact::pattern::spatial::SpatialPattern>(pattern_name, serial);

            pattern->addProducerPort("output", T::MetaType);

            pattern->addStringParameter("file", "file.json");

            return pattern;
        }

        bool configure(const nlohmann::json &parameter, buffer::ComponentBufferConfig *data) override {
            bool result = pattern::setValueFromParameter(parameter, "file", filename_, "");
            if(result) {
                if(util::FileExists(filename_, getName()))
                    ReadValue(data_);
                    return true;
            } else {
                SPDLOG_ERROR("{0}: missing file property", getName());
            }

            return false;
        }



        virtual bool ReadValue(typename T::NativeType &data) = 0;

        bool processTimePoint(buffer::ComponentBuffer &data) override {
            auto &output = data.template getOutput<typename T::NativeType, T>(0);
            output = data_;
            return true;
        }


    protected:
        std::string filename_;
        const std::string serializer_name_;
        typename T::NativeType data_;

    RTTR_ENABLE(Component)

    };

}


#endif //TRAACTMULTI_FILEREADER_H
