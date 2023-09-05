/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "RTTRPluginFactory.h"
#include "traact/util/Logging.h"
#include <regex>
#include <utility>
#include "traact/util/FileUtil.h"
#include <filesystem>

bool traact::facade::RTTRPluginFactory::addLibrary(const std::string &filename) {
    PluginPtr new_plugin;

    SPDLOG_INFO("loading library file: {0}", filename);

    try {
        new_plugin = std::make_shared<Plugin>(filename);
        if (!new_plugin->init()) {
            SPDLOG_ERROR("init failed for: {0}", filename);
            return false;
        }

    } catch (...) {
        SPDLOG_ERROR("init throws exception for: {0}", filename);
        return false;
    }

    return true;

}

bool traact::facade::RTTRPluginFactory::init() {
    using namespace rttr;

#ifdef TRAACT_TARGET_WINDOWS
    auto const kFindLib = std::regex{R"(;+)"};
    const std::string kFileEnding = ".dll";
#else
    auto const kFindLib = std::regex{R"(:+)"};
    const std::string kFileEnding = ".so";
#endif

    auto const kPluginDirs = std::vector<std::string>(
        std::sregex_token_iterator{begin(plugin_directories_), end(plugin_directories_), kFindLib, -1},
        std::sregex_token_iterator{}
    );
    for (const auto &plugin_dir : kPluginDirs) {
        std::filesystem::path pluginDirPath(plugin_dir);
        SPDLOG_INFO("attempting to load plugin directory: {0}", plugin_dir);
        std::vector<std::string> files = util::globFiles(plugin_dir, kFileEnding);
        for (const auto &file : files) {
            addLibrary(pluginDirPath / file);
        }
    }

    for (const auto &loaded_type : type::get_types()) // returns all registered types from all loaded libraries
    {

        if (!loaded_type.is_class() || loaded_type.is_wrapper()) {
            continue;
        }

        SPDLOG_DEBUG("testing type: {0}", std::string(loaded_type.get_name().begin(), loaded_type.get_name().end()));

        if (loaded_type.is_derived_from<component::ComponentFactory>()) {
            addPattern(loaded_type);
        } else if (loaded_type.is_derived_from<buffer::DataFactory>()) {
            addDatatype(loaded_type);
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
        SPDLOG_INFO("library already loaded");
    } else {

        if (!library_.load()) {
            auto error_string = library_.get_error_string();
            SPDLOG_ERROR("error loading plugin: {0}", std::string(error_string.begin(), error_string.end()));
            return false;
        }
    }

    return true;

}

void traact::facade::RTTRPluginFactory::addDatatype(const rttr::type &loaded_type) {
    using namespace rttr;
    std::string classname = std::string(loaded_type.get_name().begin(), loaded_type.get_name().end());
    if (classname == "traact::buffer::DataFactory") {
        return;
    }

    constructor ctor = loaded_type.get_constructor();
    if (ctor.is_valid()) {
        variant var = ctor.invoke();
        if (var.is_valid()) {

            auto factory = var.get_value<FactoryObjectPtr>();

            datatype_traact_plugin.emplace(factory);
            datatype_names.push_back(classname);
            SPDLOG_INFO("found datatype: factory: {0} type name: {1}", classname, factory->getTypeName());
        } else {
            SPDLOG_WARN("can not instantiate datatype factory: {0}", classname);
        }
    } else {
        SPDLOG_TRACE("constructor invalid for datatype factory: {0}", classname);
    }

}
void traact::facade::RTTRPluginFactory::addPattern(const rttr::type &loaded_type) {
    using namespace rttr;
    std::string classname = std::string(loaded_type.get_name().begin(), loaded_type.get_name().end());
    if (classname == "traact::component::ComponentFactory") {
        return;
    }
    constructor ctor = loaded_type.get_constructor();
    if (ctor.is_valid()) {
        variant var = ctor.invoke();
        if (var.is_valid()) {

            auto component_factory = var.get_value<component::ComponentFactory::Ptr>();
            auto pattern_name = component_factory->createPattern()->name;
            pattern_to_traact_plugin.emplace(pattern_name, component_factory);
            pattern_names.push_back(pattern_name);
            SPDLOG_INFO("found component: factory: {0} pattern: {1}", classname, pattern_name);
        } else {
            SPDLOG_WARN("can not instantiate component: {0}", classname);
        }
    } else {
        SPDLOG_TRACE("constructor invalid for component: {0}", classname);
    }
}

bool traact::facade::RTTRPluginFactory::Plugin::teardown() {
    return true;//library_.unload();
}

traact::facade::RTTRPluginFactory::PatternPtr traact::facade::RTTRPluginFactory::instantiatePattern(const std::string &pattern_name) {
    auto find_result = pattern_to_traact_plugin.find(pattern_name);
    if (find_result == pattern_to_traact_plugin.end()) {
        throw std::runtime_error(std::string("Trying to instantiate unknown pattern: ") + pattern_name);
    }
    return find_result->second->createPattern();
}
traact::facade::RTTRPluginFactory::ComponentPtr traact::facade::RTTRPluginFactory::instantiateComponent(const std::string &pattern_name,
                                                                                                        const std::string &instance_id) {

    auto find_result = pattern_to_traact_plugin.find(pattern_name);
    if (find_result == pattern_to_traact_plugin.end()) {
        throw std::runtime_error(std::string("Trying to instantiate unknown component: ") + pattern_name);
    }
    return find_result->second->createComponent(instance_id);

}

traact::facade::RTTRPluginFactory::RTTRPluginFactory() {
    auto *plugin_dirs = getenv("TRAACT_PLUGIN_PATHS");
    if (plugin_dirs != nullptr) {
        plugin_directories_ = plugin_dirs;
    } else {
        // right now this makes sense, in the future when there might be static linking then this check is not needed
        throw std::runtime_error(std::string("Missing environment variable: TRAACT_PLUGIN_PATH"));
    }
    init();
}

traact::facade::RTTRPluginFactory::RTTRPluginFactory(std::string plugin_directories) : plugin_directories_(std::move(
    plugin_directories)) {
    init();

}
traact::buffer::DataBufferPtr traact::facade::RTTRPluginFactory::createBuffer() {
    return std::make_unique<buffer::DataBuffer>(datatype_traact_plugin);
}
