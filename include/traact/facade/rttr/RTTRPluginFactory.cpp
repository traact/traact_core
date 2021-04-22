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

#include "RTTRPluginFactory.h"
#include <iostream>
#include <traact/util/Logging.h>
#include <regex>
#include <traact/util/FileUtil.h>

bool traact::facade::RTTRPluginFactory::addLibrary(const std::string &filename) {
    PluginPtr newPlugin;

    spdlog::info("loading library file: {0}", filename);

    try {
        newPlugin = std::make_shared<Plugin>(filename);
        if (!newPlugin->init()) {
            spdlog::error("Init failed for: {0}", filename);
            return false;
        }

    } catch (...) {
        spdlog::error("Init thows exception for: {0}", filename);
        return false;
    }


    return true;

}

bool traact::facade::RTTRPluginFactory::init() {
    using namespace rttr;

#ifdef TRAACT_TARGET_WINDOWS
    auto const re = std::regex{R"(;+)"};
	const std::string file_ending = ".dll";
#else
    auto const re = std::regex{ R"(:+)" };
    const std::string file_ending = ".so";
#endif



    auto const plugin_dirs = std::vector<std::string>(
            std::sregex_token_iterator{begin(plugin_directories_), end(plugin_directories_), re, -1},
            std::sregex_token_iterator{}
    );
    for (const auto &plugin_dir : plugin_dirs) {
        spdlog::info("attempting to load plugin directory: {0}", plugin_dir);
        std::vector<std::string> files = util::glob_files(plugin_dir, file_ending);
        for (const auto &file : files) {
            addLibrary(file);
        }
    }


    for (const auto &t : type::get_types()) // returns all registered types from this library
    {


        if (!t.is_class() || t.is_wrapper())
            continue;

        spdlog::debug("testing type: {0}", std::string(t.get_name().begin(), t.get_name().end()));


        if(t.is_derived_from<component::Component>()){
            addPattern(t);
        } else if(t.is_derived_from<buffer::BufferFactory>()){
            addDatatype(t);
        }

    }

    return true;
}
bool traact::facade::RTTRPluginFactory::removeLibrary(const std::string &filename) {
    return false;
}
std::vector<std::string> traact::facade::RTTRPluginFactory::getDatatypeNames() {
    return datatype_names;
}
std::vector<std::string> traact::facade::RTTRPluginFactory::getPatternNames() {
    return pattern_names;
}

traact::facade::RTTRPluginFactory::Plugin::Plugin(const std::string &library_file_name) : library_(library_file_name) {

}
traact::facade::RTTRPluginFactory::Plugin::~Plugin() {
    teardown();
}
bool traact::facade::RTTRPluginFactory::Plugin::init() {
    using namespace rttr;


    if (library_.is_loaded()) {
        spdlog::info("library alreay loaded");
    } else {

        if (!library_.load()) {
            auto error_string = library_.get_error_string();
            spdlog::error("error loading plugin: {0}", std::string(error_string.begin(), error_string.end()));
            return false;
        }
    }

    return true;

}

void traact::facade::RTTRPluginFactory::addDatatype(const rttr::type& t ){
    using namespace rttr;
    std::string classname = std::string(t.get_name().begin(), t.get_name().end());
    if(classname == "traact::buffer::BufferFactory")
        return;



    //if(classname.find("traact::buffer::TemplatedDefaultFactoryObject") == std::string::npos)
    //    return;

    constructor ctor = t.get_constructor();
    if(ctor.is_valid()){
        variant var = ctor.invoke();
        if(var.is_valid()){
            spdlog::info("found datatype: {0}", classname);
            auto factory = var.get_value<FactoryObjectPtr>();

            datatype_to_traact_plugin.emplace(classname, factory);
            datatype_names.push_back(classname);

        } else {
            spdlog::warn("can not instantiate datatype factory: {0}", classname);
        }
    } else {
        spdlog::warn("constructor invalid for datatype factory: {0}", classname);
    }

}
void traact::facade::RTTRPluginFactory::addPattern(const rttr::type& t ){
    using namespace rttr;
    std::string classname = std::string(t.get_name().begin(), t.get_name().end());
    if(classname == "traact::component::Component")
        return;
    constructor ctor = t.get_constructor({type::get<std::string>()});
    if(ctor.is_valid()){
        variant var = ctor.invoke(classname + std::string("_factory_object"));
        if(var.is_valid()){
            spdlog::info("found component: {0}", classname);
            auto component = var.get_value<component::Component::Ptr>();

            pattern_to_traact_plugin.emplace(classname, component);
            component_to_traact_plugin.emplace(classname, ctor);
            pattern_names.push_back(classname);
        } else {
            spdlog::warn("can not instantiate component: {0}", classname);
        }
    } else {
        spdlog::warn("constructor invalid for component: {0}", classname);
    }
}

bool traact::facade::RTTRPluginFactory::Plugin::teardown() {
   return true;//library_.unload();
}
traact::facade::RTTRPluginFactory::FactoryObjectPtr traact::facade::RTTRPluginFactory::instantiateDataType(const std::string &datatype_name) {
    auto find_result = datatype_to_traact_plugin.find(datatype_name);
    if (find_result == datatype_to_traact_plugin.end()) {
        throw std::runtime_error(std::string("Trying to instantiate unkown datatype: ") + datatype_name);
    }
    return find_result->second;

}
traact::facade::RTTRPluginFactory::PatternPtr traact::facade::RTTRPluginFactory::instantiatePattern(const std::string &patternName) {
    auto find_result = pattern_to_traact_plugin.find(patternName);
    if (find_result == pattern_to_traact_plugin.end()) {
        throw std::runtime_error(std::string("Trying to instantiate unkown pattern: ") + patternName);
    }
    return find_result->second->GetPattern();
}
traact::facade::RTTRPluginFactory::ComponentPtr traact::facade::RTTRPluginFactory::instantiateComponent(const std::string &patternName,
                                                                                                        const std::string &new_component_name) {

    auto find_result = component_to_traact_plugin.find(patternName);
    if (find_result == component_to_traact_plugin.end()) {
        throw std::runtime_error(std::string("Trying to instantiate unkown component: ") + patternName);
    }
    return find_result->second.invoke(new_component_name).get_value<ComponentPtr>();

}

traact::facade::RTTRPluginFactory::RTTRPluginFactory() {
    plugin_directories_ = getenv("TRAACT_PLUGIN_PATHS");
    init();
}

traact::facade::RTTRPluginFactory::RTTRPluginFactory(const std::string &pluginDirectories) : plugin_directories_(
        pluginDirectories) {
    init();

}
